#pragma once

#include "Thread.h"
#include <vector>
#include "Queue.h"

#define TIMER_PRECISION 1000


struct ITimerCallback
{
	virtual void OnTimer(int nID) = 0;
};

typedef void (*PFnOnTimer)(int nID);

class CTimerCallback
{
public:
	inline CTimerCallback(ITimerCallback* pCallback) : m_bStatic(false), m_pCallbackPtr(pCallback) {}
        inline CTimerCallback(PFnOnTimer pfnStaticCallback) : m_bStatic(true), m_pCallbackPtr((void*)pfnStaticCallback) {}

	inline void Call(int nID) {
                if (m_bStatic) ((PFnOnTimer)m_pCallbackPtr)(nID);
		else static_cast<ITimerCallback*>(m_pCallbackPtr)->OnTimer(nID);
	}

private:
	bool	m_bStatic;
	void*	m_pCallbackPtr;
};

class CTimer : public IRunnable
{
public:
	CTimer(int nThreadsCount = 1);
	~CTimer(void);

	void Set(int nSeconds, int nID, ITimerCallback* pCallback, bool bAutoStart = true, bool bAutoRestart = true);
	void Set(int nSeconds, int nID, PFnOnTimer pfnCallback, bool bAutoStart = true, bool bAutoRestart = true);
	void Drop(int nID);
	void Start(int nID);

protected:
	struct STimer
	{
		int				m_nID;
		bool			m_bValid;
		int				m_nInitialValue;
		int				m_nCurrentValue;
		bool			m_bAutoRestart;
		CTimerCallback	m_Callback;

		STimer(int nID, int nSeconds, const CTimerCallback& rCallback, bool bAutoStart = true, bool bAutoRestart = true)
		 :	m_nID(nID),
			m_bValid(true),
			m_nInitialValue(bAutoStart ? nSeconds : 0),
			m_nCurrentValue(nSeconds),
			m_Callback(rCallback),
			m_bAutoRestart(bAutoRestart)
		{ }
	};

	void Set(int nSeconds, int nID, const CTimerCallback& rCallback, bool bAutoStart = true, bool bAutoRestart = true);

	int Run();
	void Timer();
	STimer* GetTimer(int nID);
	void Sleep();

private:
	bool					m_bTerminating;
	int						m_nThreadsStarted;
	
	CSemaphore				m_SleepSemaphore;

	CMutex					m_Lock;
	std::vector<STimer*> 	m_vecTimers;
	std::vector<CThread*>	m_vecThreads;

	enum ETimerCmd {
		eCallback, eDropTimer, eStopThread
	};
	struct STimerCmd {
		ETimerCmd	m_eCmd;
		STimer*		m_psTimer;
		STimerCmd(ETimerCmd eCmd, STimer* psTimer = NULL) : m_eCmd(eCmd), m_psTimer(psTimer) {}
		STimerCmd() {}
	};
	CQueue<STimerCmd>		m_queTimerCommands;
};
