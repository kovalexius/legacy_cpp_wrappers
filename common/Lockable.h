#pragma once

#include "tstring.h"

#ifdef _WIN32

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400 
#endif
#include <windows.h>
typedef HANDLE TMutexHandle;
typedef HANDLE TSemaphoreHandle;
typedef HANDLE TEventHanlde;

#else

#include <pthread.h>
#include <semaphore.h>

typedef sem_t TSemaphoreHandle;
typedef pthread_mutex_t TMutexHandle;
//typedef pthread_t TThreadHandle;
typedef pthread_cond_t TEventHanlde;

#define INFINITE -1

#endif

#include <stdio.h>
#include <set>

struct ILockable
{
	virtual bool Lock(unsigned nTimeout = INFINITE) = 0;
	virtual void Unlock() = 0;
};

class CMutex : public ILockable
{
public:
        inline CMutex(const TCHAR* szName = NULL) {
		Init(szName);
	}
	inline CMutex(const CMutex& rRight) {
		this->Init(rRight.m_szName);
	}
	~CMutex();

	CMutex& operator = (const CMutex& rLock);
	inline operator TMutexHandle&() { return m_hLock; }

	virtual bool Lock(unsigned nTimeout = INFINITE);
	virtual void Unlock();

protected:
	void Init(const TCHAR* szName);
	
private:
	TMutexHandle m_hLock;
	TCHAR* m_szName;
};

class CSemaphore
{
public:
	CSemaphore(unsigned long nInitialCnt = 0) {
		Init(nInitialCnt);
	}
	CSemaphore(const CSemaphore& rRight) { 
		*this = rRight; 
	}
	~CSemaphore();

	CSemaphore& operator = (const CSemaphore& rLock);
	inline operator TSemaphoreHandle() const { return m_hLock; }

	bool Wait(unsigned nTimeout = INFINITE);
	void Signal();

	static int WaitForScope(CSemaphore** apSemaphores, int nCnt, unsigned nTimeout, bool bWaitAll);

protected:
	void Init(long nInitialCnt);

private:
	TSemaphoreHandle m_hLock;
	unsigned long m_nInitialCnt;
#ifdef linux
	CMutex m_Mutex;
	std::set<CSemaphore*> m_setCommonWaiters;
#endif
};

class CLockGuard
{
public:
	CLockGuard(ILockable& rSyncObj) : m_rSyncObj(rSyncObj) { m_rSyncObj.Lock(); }
	~CLockGuard() { m_rSyncObj.Unlock(); }

private:
	CLockGuard(const CLockGuard&);
	const CLockGuard& operator = (const CLockGuard&);
	ILockable& m_rSyncObj;
};

class CMultiLock : public ILockable
{
	enum EState { eNone, ePulse, ePulseAll } m_eState;
public:
	inline CMultiLock() { Init(eNone, 0); }
	inline CMultiLock(const CMultiLock& rRight) { Init(rRight.m_eState, rRight.m_nWaiters); }
	~CMultiLock();

	CMultiLock& operator = (const CMultiLock& rRight);

	virtual inline bool Lock(unsigned nTimeout = INFINITE) { return m_Mutex.Lock(nTimeout); }
	virtual inline void Unlock() { 	m_Mutex.Unlock(); }
	
	bool Wait(unsigned nTimeout = INFINITE);
	void Pulse();
	void PulseAll();

protected:
	void Init(EState eState, unsigned nWaiters);

private:
	TEventHanlde m_hEvent;
	CMutex m_Mutex;
	bool m_bState;
	unsigned m_nWaiters;
};
