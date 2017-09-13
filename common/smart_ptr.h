#pragma once

template <class T> class smart_ptr
{
public:
	smart_ptr(T* pValue = NULL) : m_pValue(pValue), m_pnPtrCnt(pValue ? new int : NULL) {
		if (pValue) *m_pnPtrCnt = 1;
	}
	~smart_ptr(void) {
		free();
	}
	smart_ptr(const smart_ptr& rRight) : m_pValue(NULL), m_pnPtrCnt(NULL) {
		*this = rRight;
	}
	smart_ptr& operator = (const smart_ptr& rRight)
	{
		if (m_pValue && m_pValue != rRight.m_pValue) free();
		m_pValue = rRight.m_pValue;
		if (m_pnPtrCnt = rRight.m_pnPtrCnt) *m_pnPtrCnt += m_pValue ? 1 : 0;
		return *this;
	}
	inline operator T*() {
		return m_pValue;
	}
	operator const T*() const {
		return m_pValue;
	}
	T* operator ->() {
		return m_pValue;
	}
	const T* operator ->() const {
		return m_pValue;
	}
	T* get() {
		return m_pValue;
	}
	const T* get() const {
		return m_pValue;
	}

	void free() {
		if (m_pValue && !--*m_pnPtrCnt) 
		{
			delete m_pValue, m_pValue = NULL;
			delete m_pnPtrCnt, m_pnPtrCnt = NULL;
		}
	}

private:
	T*		m_pValue;
	int*	m_pnPtrCnt;
};

template <class T> class const_smart_ptr
{
public:
	const_smart_ptr(T* pValue = NULL) : m_pValue(pValue), m_pnPtrCnt(pValue ? new int : NULL) {
		if (pValue) *m_pnPtrCnt = 1;
	}
	~const_smart_ptr(void) {
		free();
	}
	const_smart_ptr(const const_smart_ptr& rRight) : m_pValue(NULL), m_pnPtrCnt(NULL) {
		*this = rRight;
	}
	const_smart_ptr(const smart_ptr& rRight) : m_pValue(NULL), m_pnPtrCnt(NULL) {
		*this = rRight;
	}

	const_smart_ptr& operator = (const const_smart_ptr& rRight)
	{
		if (m_pValue && m_pValue != rRight.m_pValue) free();
		m_pValue = rRight.m_pValue;
		if (m_pnPtrCnt = rRight.m_pnPtrCnt) *m_pnPtrCnt += m_pValue ? 1 : 0;
		return *this;
	}

	const_smart_ptr& operator = (const smart_ptr& rRight)
	{
		if (m_pValue && m_pValue != rRight.m_pValue) free();
		m_pValue = rRight.m_pValue;
		if (m_pnPtrCnt = rRight.m_pnPtrCnt) *m_pnPtrCnt += m_pValue ? 1 : 0;
		return *this;
	}

	operator const T*() const {
		return m_pValue;
	}
	const T* operator ->() const {
		return m_pValue;
	}
	const T* get() const {
		return m_pValue;
	}

	void free() {
		if (m_pValue && !--*m_pnPtrCnt) 
		{
			delete m_pValue, m_pValue = NULL;
			delete m_pnPtrCnt, m_pnPtrCnt = NULL;
		}
	}

private:
	T*		m_pValue;
	int*	m_pnPtrCnt;
};

