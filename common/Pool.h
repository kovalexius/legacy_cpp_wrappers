#pragma once

#include "Queue.h"

template <class T> class CPool;

template <class T> class CPoolElement
{
public:
	inline CPoolElement(T rValue, CPool<T>& rPool) : m_Value(rValue), m_Pool(rPool) {}
	
	~CPoolElement() {
		m_Pool.Put(m_Value);
	}
	inline operator T() {
		return m_Value;
	}
	inline T operator ->() {
		return m_Value;
	}

private:
	T			m_Value;
	CPool<T>&	m_Pool;
};

template <class T> class CPool
{
public:
	CPool(unsigned nMaxSize = 0) : m_quePool(nMaxSize) {};
	~CPool(void) {};

	inline CPoolElement<T> Get() {
		return CPoolElement<T>(m_quePool.Pop(), *this);
	}
	inline void Put(T t) { 
		m_quePool.Push(t);
	}
	inline bool IsEmpty() {
		return m_quePool.IsEmpty();
	}
	inline T Pop() { 
		return m_quePool.Pop();
	}

private:
	CQueue<T> m_quePool;
};
