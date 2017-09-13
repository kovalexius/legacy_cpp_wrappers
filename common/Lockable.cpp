#define _CRT_SECURE_NO_WARNINGS

#include "tstring.h"
#include "Lockable.h"
#include "Error.h"

#ifdef _WIN32
#include <vector>
#endif

#ifdef linux

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdexcept>
#include <string.h>
#include <limits.h>
#include <memory>
#include "Common.h"

#endif

/************************************************************************/
/* CMutex implementation                                                */
/************************************************************************/
CMutex::~CMutex() 
{ 
#ifdef _WIN32
	CloseHandle(m_hLock); 
#else
	pthread_mutex_destroy(&m_hLock);
#endif
}

bool CMutex::Lock(unsigned nTimeout) 
{
#ifdef _WIN32 
	return WaitForSingleObject(m_hLock, nTimeout) == WAIT_OBJECT_0;
#else
	int rc;
	if (nTimeout == INFINITE) 
	{
		rc = pthread_mutex_lock(&m_hLock);
		if (rc) SYSERROR("pthread_mutex_lock");
	}
	else 
	{
		timespec ts;
		if (clock_gettime(CLOCK_REALTIME, &ts) == -1) SYSERROR("clock_gettime");
		ts.tv_nsec += (nTimeout % 1000) * 1000000;
		ts.tv_sec += nTimeout / 1000 + ts.tv_nsec / 1000000000;
		ts.tv_nsec = ts.tv_nsec % 1000000000;
		rc = pthread_mutex_timedlock(&m_hLock, &ts);
		if (rc && rc != ETIMEDOUT) SYSERROR("pthread_mutex_timedlock");
	}
	return rc == 0;
#endif
}
	
void CMutex::Unlock() { 
#ifdef _WIN32
	ReleaseMutex(m_hLock); 
#else
	pthread_mutex_unlock(&m_hLock);
#endif
}

CMutex& CMutex::operator = (const CMutex& rLock) 
{
	this->~CMutex();
	this->Init(rLock.m_szName);
	return *this;
}

void CMutex::Init(const TCHAR* szName)
{
	if (szName) { m_szName = new TCHAR[tcslen(szName + 1)]; tcscpy(m_szName, szName); }
	else m_szName = NULL;
#ifdef _WIN32
	if (!(m_hLock = CreateMutex(NULL, FALSE, szName))) SYSERROR("CreateMutex");
#else
	pthread_mutexattr_t mt;
	if (pthread_mutexattr_init(&mt) ||
		pthread_mutexattr_settype(&mt, PTHREAD_MUTEX_RECURSIVE) ||
		pthread_mutex_init(&m_hLock, &mt)
	) SYSERROR("pthread_mutex_init");
#endif
}

/************************************************************************/
/* CSemaphore implementation                                            */
/************************************************************************/
CSemaphore::~CSemaphore()
{ 
#ifdef _WIN32
	CloseHandle(m_hLock);
#else
	sem_destroy(&m_hLock);
#endif
}

bool CSemaphore::Wait(unsigned nTimeout) 
{
	bool bResult = false;
#ifdef _WIN32
	bResult = WaitForSingleObject(m_hLock, nTimeout) == WAIT_OBJECT_0;
#else
	if (nTimeout == INFINITE)
	{
		if (sem_wait(&m_hLock) == -1) SYSERROR("sem_wait");
		else bResult = true;
	}
	else
	{
		timespec ts;
		if (clock_gettime(CLOCK_REALTIME, &ts) == -1) SYSERROR("clock_gettime");
		ts.tv_nsec += (nTimeout % 1000) * 1000000;
		ts.tv_sec += nTimeout / 1000 + ts.tv_nsec / 1000000000;
		ts.tv_nsec = ts.tv_nsec % 1000000000;
		if (sem_timedwait(&m_hLock, &ts) == -1) {
			if (errno != EAGAIN) SYSERROR("semop");
		}
		else bResult = true;
	}
#endif
	if (bResult) m_nInitialCnt--;
	return bResult;
}

void CSemaphore::Signal() 
{
#ifdef _WIN32
	if (!ReleaseSemaphore(m_hLock, 1, NULL)) SYSERROR("ReleaseSemaphore");
#else
	if (sem_post(&m_hLock) == -1) SYSERROR("semop");
	else 
	{
		m_nInitialCnt++;
		CLockGuard rLock(m_Mutex);
		if (!m_setCommonWaiters.empty())
			if (!(*m_setCommonWaiters.begin())->m_nInitialCnt) 
				(*m_setCommonWaiters.begin())->Signal();
	}
#endif
}

CSemaphore& CSemaphore::operator = (const CSemaphore& rLock) 
{
	this->~CSemaphore();
	this->Init(rLock.m_nInitialCnt);
#ifdef linux
	m_Mutex = rLock.m_Mutex;
#endif
	return *this;
}

void CSemaphore::Init(long nInitialCnt) 
{
	m_nInitialCnt = nInitialCnt;
#ifdef _WIN32
	if (!(m_hLock = CreateSemaphore(NULL, m_nInitialCnt = nInitialCnt, INT_MAX, NULL)))
		SYSERROR("Error creating semaphore");
#else
	if (sem_init(&m_hLock, 0, 0) == -1) SYSERROR("sem_init");
#endif
}

int CSemaphore::WaitForScope(CSemaphore** apSemaphores, int nCnt, unsigned nTimeout, bool bWaitAll)
{
	int nResult = 0;
	if (nCnt)
	{
#ifdef _WIN32
		std::vector<TSemaphoreHandle> vecHandles;
		for (int i = 0; i < nCnt; i++) vecHandles.push_back(*apSemaphores[i]);
		DWORD dwResult = ::WaitForMultipleObjects((DWORD)nCnt, &*vecHandles.begin(), (BOOL)bWaitAll, (DWORD)nTimeout);
		if (dwResult >= WAIT_OBJECT_0 && dwResult <= (WAIT_OBJECT_0 + nCnt - 1)) nResult = dwResult - WAIT_OBJECT_0;
		else nResult = -1;
#else
		timespec ts;
		timespec* pts = NULL;
		if (nTimeout != INFINITE)
		{
			if (clock_gettime(CLOCK_REALTIME, &ts) == -1) SYSERROR("clock_gettime");
			ts.tv_nsec += (nTimeout % 1000) * 1000000;
			ts.tv_sec += nTimeout / 1000 + ts.tv_nsec / 1000000000;
			ts.tv_nsec = ts.tv_nsec % 1000000000;
			pts = &ts;
		}
		if (bWaitAll)
		{
			int rc = 0;
			for (unsigned i = 0; i < nCnt && (nResult == 0 || nResult == EAGAIN); i++) 
			if (pts) rc = sem_timedwait(&apSemaphores[i]->m_hLock, pts);
			else sem_wait(&apSemaphores[i]->m_hLock);
			if (rc == 0) nResult = 0; else nResult = -1;
		}
		else 
		{
			std::auto_ptr<CSemaphore> pCommonLock(new CSemaphore);
			for (unsigned i = 0; i < nCnt; i++) 
			{
				CLockGuard rLock(apSemaphores[i]->m_Mutex);
				apSemaphores[i]->m_setCommonWaiters.insert(pCommonLock.get());
			}
			nResult = pCommonLock->Wait(nTimeout) ? 0 : -1;
			for (unsigned i = 0; i < nCnt; i++) 
			{
				CLockGuard rLock(apSemaphores[i]->m_Mutex);
				apSemaphores[i]->m_setCommonWaiters.erase(pCommonLock.get());
			}
			pCommonLock.reset();
		}
#endif
	}
	return nResult;
}

/************************************************************************/
/* CMultiLock implementation                                            */
/************************************************************************/
CMultiLock::~CMultiLock()
{
#ifdef _WIN32
	CloseHandle(m_hEvent);
#elif defined(linux)
	pthread_cond_destroy(&m_hEvent);
#endif
}

void CMultiLock::Init(EState eState, unsigned nWaiters)
{
	m_nWaiters = nWaiters;
	m_eState = eState;
#ifdef _WIN32
	if (!(m_hEvent = CreateEvent(NULL, TRUE, eState != eNone, NULL))) SYSERROR("CreateEvent");
#elif defined(linux)
	if (pthread_cond_init(&m_hEvent, NULL)) SYSERROR("pthread_cond_init");
#endif
}

CMultiLock& CMultiLock::operator = (const CMultiLock& rRight)
{
	this->~CMultiLock();
	this->Init(rRight.m_eState, rRight.m_nWaiters);
	this->m_Mutex = rRight.m_Mutex;
	return *this;
}

bool CMultiLock::Wait(unsigned nTimeout)
{
	m_nWaiters++;
#ifdef _WIN32
	bool bResult = SignalObjectAndWait(m_Mutex, m_hEvent, nTimeout, FALSE) == WAIT_OBJECT_0;
	m_Mutex.Lock();
	if (--m_nWaiters == 0 || m_eState == ePulse) ResetEvent(m_hEvent);
	return bResult;
#elif defined(linux)
	int rc = 0;
	if (nTimeout = INFINITE) while (m_eState == eNone && !rc) rc = pthread_cond_wait(&m_hEvent, &(TMutexHandle&)m_Mutex);
	else 
	{
		timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += nTimeout / 1000;
		ts.tv_nsec += (nTimeout % 1000) * 1000000;
		while (m_eState == eNone && !rc) rc = pthread_cond_timedwait(&m_hEvent, &(TMutexHandle&)m_Mutex, &ts);
		if (rc != ETIMEDOUT) SYSERROR("pthread_cond_timedwait");
	}
	if (--m_nWaiters == 0 || m_eState == ePulse) m_eState = eNone;
	return rc == 0;
#endif
}

void CMultiLock::Pulse()
{
	m_eState = ePulse;
#ifdef _WIN32
	SetEvent(m_hEvent);
#elif defined(linux)
	pthread_cond_signal(&m_hEvent);
#endif
}

void CMultiLock::PulseAll()
{
	m_eState = ePulseAll;
#ifdef _WIN32
	SetEvent(m_hEvent);
#elif defined(linux)
	pthread_cond_broadcast(&m_hEvent);
#endif
}
