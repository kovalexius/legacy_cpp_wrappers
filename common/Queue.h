#pragma once

// Реализация потоко-безопасной очереди
// с блокировкой на опустошение и на переполнение
#include <queue>
#include "Lockable.h"

template <class T> class CQueue
{
public:
	CQueue(int nMaxSize = 0)
		:	m_nMaxSize(nMaxSize),
		m_ReadWaiters(0),
		m_WriteWaiters(0)
	{ }
	~CQueue(void)
	{
		Clear();
	}

	void Push(T e);
	T Peek();
	T Pop();
	bool PopEx(int nTimeout, T* pT);

	void Clear();
	bool IsEmpty() const;
	bool IsFull() const;
	unsigned GetSize() const;

private:

	std::queue<T>		m_queData;		// собственно, очередь
	unsigned			m_nMaxSize;		// максимальный размер очереди

//	mutable CMutex		m_DataLock;			// Мьютекс данных очереди
//	mutable CSemaphore	m_Monitor;	// Семафор опустошения очереди
	//	mutable CSemaphore	m_ReadSemaphore;	// Семафор опустошения очереди
	//	mutable CSemaphore	m_WriteSemaphore;	// Семафор переполнения очереди
	mutable int			m_ReadWaiters;
	mutable int			m_WriteWaiters;
	mutable CMultiLock	m_Lock;
};
template <class T> void CQueue<T>::Push(T e)
{
	CLockGuard rLock(m_Lock);
	while (m_nMaxSize > 0 && m_queData.size() >= m_nMaxSize)
	{
		m_WriteWaiters++;
		m_Lock.Wait();
		m_WriteWaiters--;
	}
	m_queData.push(e);
	if (m_ReadWaiters > 0) m_Lock.Pulse();
}

template <class T> T CQueue<T>::Peek()
{
	T pResult;
	
	CLockGuard rLock(m_Lock);

	while (m_queData.empty())
	{
		m_ReadWaiters++;
		m_Lock.Wait();
		m_ReadWaiters--;
	}
	pResult = m_queData.front();
	m_Lock.Pulse();

	return pResult;
}
template <class T> T CQueue<T>::Pop()
{
	T pResult;

	CLockGuard rLock(m_Lock);

	while (m_queData.empty())
	{
		m_ReadWaiters++;
		m_Lock.Wait();
		m_ReadWaiters--;
	}
	pResult = m_queData.front();
	m_queData.pop();
	if (m_WriteWaiters > 0) m_Lock.Pulse();

	return pResult;
}

template <class T> bool CQueue<T>::PopEx(int nTimeout, T* pT)
{
	bool bResult = false;

	CLockGuard rLock(m_Lock);

	if (m_queData.empty())
	{
		m_ReadWaiters++;
		m_Lock.Wait(nTimeout);
		m_ReadWaiters--;
	}

	if (bResult = !m_queData.empty())
	{
		*pT = m_queData.front();
		m_queData.pop();
		if (m_WriteWaiters > 0) m_Lock.Pulse();
	};

	return bResult;
}
/*
template <> inline void CQueue<void*>::Clear()
{
	CLockGuard rLock(m_Lock);

	size_t nQueueSize = m_queData.size();
	for (unsigned i = 0; i < nQueueSize; i++)
	{
		try
		{
			delete m_queData.front();
		} 
		catch(...) 
		{ }
		m_queData.pop();
	}
	if (m_WriteWaiters > 0) m_Lock.PulseAll();
}
*/
template <class T> void CQueue<T>::Clear()
{
	CLockGuard rLock(m_Lock);

	while (!m_queData.empty()) m_queData.pop();
	if (m_WriteWaiters > 0) m_Lock.PulseAll();

}

template <class T> bool CQueue<T>::IsEmpty() const
{
	CLockGuard rLock(m_Lock);
	return m_queData.empty();
}

template <class T> bool CQueue<T>::IsFull() const
{
	CLockGuard rLock(m_Lock);
	return m_nMaxSize > 0 && m_queData.size() >= m_nMaxSize;
}

#pragma warning(push)
#pragma warning(disable:4267)

template <class T> unsigned CQueue<T>::GetSize() const
{
	CLockGuard rLock(m_Lock);
	return m_queData.size();
}

#pragma warning(pop)
