#include "Timer.h"
#include "Error.h"

CTimer::CTimer(int nThreadsCount)
 :	m_bTerminating(false),
	m_nThreadsStarted(0)
{
	for (int i = 0; i < nThreadsCount + 1; i++)
	{
		CThread* pThread = new CThread(this);
		if (!pThread) SYSERROR("Error creating timer thread");
		else m_vecThreads.push_back(pThread);
	}
}

CTimer::~CTimer(void)
{
	m_bTerminating = true;

	m_SleepSemaphore.Signal();
	{
		CLockGuard rLock(m_Lock);
		for (unsigned i = 0; i < m_vecTimers.size(); i++)
		{
			m_vecTimers[i]->m_bValid = false;
			m_queTimerCommands.Push(STimerCmd(eDropTimer, m_vecTimers[i]));
		}
		m_vecTimers.clear();
	}
	for (unsigned i = 0; i < m_vecThreads.size() - 1; i++) 
		m_queTimerCommands.Push(STimerCmd(eStopThread));

	if (!m_vecThreads.empty())
	{
		CThread::WaitForScope(&m_vecThreads[0], (int)m_vecThreads.size(), 1000);
		for (unsigned i = 0; i < m_vecThreads.size(); i++) delete m_vecThreads[i], m_vecThreads[i] = NULL;
		m_vecThreads.clear();
	}
}

int CTimer::Run()
{
	try
	{
		if (!m_nThreadsStarted++) Timer();
		else
		{
			for(;;)
			{
				STimerCmd sTimer = m_queTimerCommands.Pop();
				if (sTimer.m_eCmd == eStopThread) break;
				else if (sTimer.m_eCmd == eDropTimer) delete sTimer.m_psTimer;
				else if (m_bTerminating) continue;
				else if (sTimer.m_psTimer->m_bValid) sTimer.m_psTimer->m_Callback.Call(sTimer.m_psTimer->m_nID);
			}
		}
	}
	RETHROW;
	
	return 0;
}

void CTimer::Timer()
{
	int nTimeToSleep = TIMER_PRECISION;
	while (!m_bTerminating)
	{
		unsigned nTicks = CThread::GetTickCount();
		m_SleepSemaphore.Wait(nTimeToSleep);
		if (m_bTerminating) continue;

		{
			CLockGuard rLock(m_Lock);
			for (unsigned i = 0; i < m_vecTimers.size(); i++)
			{
				STimer* psTimer = m_vecTimers[i];
				if (psTimer->m_nCurrentValue > 0 && --psTimer->m_nCurrentValue == 0) 
				{
					if (psTimer->m_bAutoRestart) psTimer->m_nCurrentValue = psTimer->m_nInitialValue;
					m_queTimerCommands.Push(STimerCmd(eCallback, psTimer));
				}
			}
		}
		nTimeToSleep = TIMER_PRECISION - (CThread::GetTickCount() - nTicks);
		if (nTimeToSleep < 0) nTimeToSleep = 0;
	}
}

void CTimer::Set(int nSeconds, int nID, const CTimerCallback& rCallback, bool bAutoStart, bool bAutoRestart)
{
	CLockGuard rLock(m_Lock);
	STimer* pTimer = GetTimer(nID);
	if (pTimer) 
	{
		pTimer->m_nInitialValue = nSeconds;
		pTimer->m_Callback = rCallback;
		pTimer->m_bAutoRestart = bAutoRestart;
		if (bAutoStart) pTimer->m_nCurrentValue = nSeconds;
		else pTimer->m_nCurrentValue = 0;
	}
	else m_vecTimers.push_back(new STimer(nID, nSeconds, rCallback, bAutoRestart));
}

void CTimer::Set(int nSeconds, int nID, ITimerCallback* pCallback, bool bAutoStart, bool bAutoRestart)
{
	Set(nSeconds, nID, CTimerCallback(pCallback), bAutoStart, bAutoRestart);
}

void CTimer::Set(int nSeconds, int nID, PFnOnTimer pfnCallback, bool bAutoStart/*= true*/, bool bAutoRestart/*= true*/)
{
	Set(nSeconds, nID, CTimerCallback(pfnCallback), bAutoStart, bAutoRestart);
}

void CTimer::Drop(int nID)
{
	CLockGuard rLock(m_Lock);
	for (unsigned i = 0; i < m_vecTimers.size(); i++)
	{
		STimer* psTimer = m_vecTimers[i];
		if (psTimer->m_nID == nID) 
		{
			psTimer->m_bValid = false;
			m_queTimerCommands.Push(STimerCmd(eDropTimer, psTimer));
			m_vecTimers.erase(m_vecTimers.begin() + i);
			break;
		}
	}
}

void CTimer::Start(int nID)
{
	try
	{
		CLockGuard rLock(m_Lock);
		STimer* pTimer = GetTimer(nID);
		if (pTimer) pTimer->m_nCurrentValue = pTimer->m_nInitialValue;
		else STDERROR("Internal error: timer not found");
	}
	RETHROW
}

CTimer::STimer* CTimer::GetTimer(int nID)
{
	STimer* pResult = NULL;
	for (unsigned i = 0; i < m_vecTimers.size() && !pResult; i++)
	{
		STimer* psTimer = m_vecTimers[i];
		if (psTimer->m_nID == nID) pResult = psTimer;
	}
	return pResult;
}
