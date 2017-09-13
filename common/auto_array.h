#pragma once

template <class T> class auto_array
{
public:
	auto_array(T* pPtr) : m_pPtr(pPtr) { }
	~auto_array() { delete [] m_pPtr; }

	inline operator T*() const { return m_pPtr; }

	static void swap(auto_array& rLeft, auto_array& rRight)
	{
		std::swap(rLeft.m_pPtr, rRight.m_pPtr);
	}

private:
	const auto_array& operator = (const auto_array&) { return this; }
	T* m_pPtr;
};


namespace std
{
	template<class T> inline void swap(auto_array<T>& rLeft, auto_array<T>& rRight)
	{
		auto_array<T>::swap(rLeft, rRight);
	}
};

