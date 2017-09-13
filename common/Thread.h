#pragma once

#include "Lockable.h"

struct IRunnable
{
	virtual int Run() = 0;
};

#ifdef _WIN32
#include <windows.h>
typedef HANDLE TThreadHandle;
typedef DWORD TThreadReturnValue;
#else
#include <pthread.h>
#include <sched.h>
typedef pthread_t TThreadHandle;
typedef void* TThreadReturnValue;
#endif

typedef int(*PFnThreadEntry)(void*);

class CThread : public IRunnable
{
public:
	explicit CThread(IRunnable* pThread = 0, bool bCreateSuspended = false);
	explicit CThread(PFnThreadEntry pfnThreadEntry, void* pParam = NULL, bool bCreateSuspended = false);
	~CThread(void);

	void Resume();
	void Suspend();
	bool Join(unsigned nTimeout = 0);
	void Terminate();
	
	static bool WaitForScope(CThread** aThreads, int nCnt, unsigned nTimeout);
	
	static void Sleep(unsigned nMilliseconds);
	static unsigned GetTickCount();

	inline static unsigned GetCurrentThreadID() {
#ifdef linux
		return (unsigned)pthread_self();
#elif defined(_WIN32)
		return (unsigned)::GetCurrentThreadId();
#endif
	}

#ifdef linux
	static void* ThreadEntry(void * p);
#elif defined(_WIN32)
	static DWORD WINAPI ThreadEntry(LPVOID lpParam);
#endif

	virtual int Run() {
		return 0;
	}

private:
	bool			m_bTerminated;
	CSemaphore		m_Semaphore;
	
	TThreadHandle	m_hHandle;
	bool			m_bStaticThreadFn;
	union {
		IRunnable*	m_pThread;
		void*		m_pParam;
	};
	PFnThreadEntry	m_pfnThreadEntry;
};
