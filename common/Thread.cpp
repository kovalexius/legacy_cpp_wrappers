#include "tstring.h"
#include "Thread.h"
#include <vector>
#include "Error.h"

#ifdef linux
#include <signal.h>
#include <unistd.h>
#include <sys/time.h> 
#endif

#include <stdio.h>

CThread::CThread(IRunnable* pThread /*=NULL*/, bool bCreateSuspended /*= false*/)
 :	m_bTerminated(false),
	m_hHandle(0),
	m_bStaticThreadFn(false),
	m_pThread(pThread)
{
	if (!pThread) m_pThread = this;
#ifdef linux
	if (!bCreateSuspended) Resume();
#elif defined(_WIN32)
	m_hHandle = ::CreateThread(NULL, 0, ThreadEntry, this, bCreateSuspended ? CREATE_SUSPENDED : 0, NULL);
#endif
}

CThread::CThread(PFnThreadEntry pfnThreadEntry, void* pParam, bool bCreateSuspended)
 :	m_bTerminated(false),
	m_hHandle(0),
	m_bStaticThreadFn(true),
	m_pfnThreadEntry(pfnThreadEntry),
	m_pParam(pParam)
{
#ifdef linux
	if (!bCreateSuspended) Resume();
#elif defined(_WIN32)
	m_hHandle = ::CreateThread(NULL, 0, ThreadEntry, this, bCreateSuspended ? CREATE_SUSPENDED : 0, NULL);
#endif
}

CThread::~CThread(void)
{
	if (!m_bTerminated) Terminate();
}

#ifdef linux
TThreadReturnValue CThread::ThreadEntry(void* lpParam)
#elif defined(_WIN32)
TThreadReturnValue WINAPI CThread::ThreadEntry(LPVOID lpParam)
#endif
{
	CThread* pThread = static_cast<CThread*>(lpParam);
	#ifdef linux
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	#endif
	TThreadReturnValue result = (TThreadReturnValue)(pThread->m_bStaticThreadFn ? pThread->m_pfnThreadEntry(pThread->m_pParam) : pThread->m_pThread->Run());
	pThread->m_bTerminated = true;
#ifdef _WIN32
	::CloseHandle(pThread->m_hHandle);
	pThread->m_hHandle = NULL;
#endif
	pThread->m_Semaphore.Signal();
	return result;
}

bool CThread::Join(unsigned nTimeout/*= 0*/)
{
	try
	{
		m_Semaphore.Wait(nTimeout);
#ifdef linux
		if (m_hHandle) pthread_detach(m_hHandle);
#endif
		return m_bTerminated;
	}
	RETHROW;
}

void CThread::Resume()
{
	try
	{
#ifdef linux
		if (m_hHandle) pthread_kill(m_hHandle, SIGCONT);
		else pthread_create(&m_hHandle, NULL, ThreadEntry, (void*)this);
#elif defined(_WIN32)	
		if (m_hHandle) ::ResumeThread(m_hHandle);
		else m_hHandle = CreateThread(NULL, 0, ThreadEntry, this, 0, NULL);
#endif
	}
	RETHROW;
}

void CThread::Suspend()
{
#ifdef linux
	pthread_kill(m_hHandle, SIGSTOP);
#elif defined(_WIN32)	
	::SuspendThread(m_hHandle);
#endif
}

void CThread::Terminate()
{
	try
	{
#if defined(linux)
		if (m_hHandle) 
		{
			pthread_cancel(m_hHandle);
			pthread_join(m_hHandle, NULL);
		}
		else m_Semaphore.Signal();
		pthread_detach(m_hHandle);
#elif defined(_WIN32)	
		::TerminateThread(m_hHandle, 0);
		::CloseHandle(m_hHandle);
		m_hHandle = NULL;
#endif
	}
	RETHROW;
}

bool CThread::WaitForScope(CThread** aThreads, int nCnt, unsigned nTimeout)
{
	std::vector<CSemaphore*> vecWaitSemaphores;
	for (int i = 0; i < nCnt; i++) vecWaitSemaphores.push_back(&aThreads[i]->m_Semaphore);
	return CSemaphore::WaitForScope(&vecWaitSemaphores[0], (int)vecWaitSemaphores.size(), nTimeout, true) >= 0;
}

void CThread::Sleep(unsigned nMilliseconds)
{
#ifdef _WIN32
	::Sleep(nMilliseconds);
#elif defined(linux)
	usleep(nMilliseconds * 1000);
#endif
}

unsigned CThread::GetTickCount()
{
#ifdef _WIN32
	return (unsigned)::GetTickCount();
#else
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif
}
