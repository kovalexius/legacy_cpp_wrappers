#pragma once

//#pragma warning(push)
// #pragma warning(disable : 4800)
#include <string>

class CVariant
{
public:
	inline CVariant(void) : m_szVal(0) {};
	~CVariant();

	template <class T> CVariant(const T& val) : m_szVal(0) { *this = val; }
	template <class T> CVariant(T& val) : m_szVal(0) { *this = val; }

	template <class T> T operator =(const T& val) {
		Clear();
		SetNumber(val);
		return val;
	}

	CVariant& operator =(const CVariant &rRight);
	const char* operator =(const char* szVal);
	const long long operator =(const int nVal);
	bool operator = (bool bVal);

	/*
	template <class T> operator const T&() const {
		return reinterpret_cast<const T&>(m_nVal);
	}*/
	inline operator const bool&() const {
		return reinterpret_cast<const bool&>(m_nVal);;
	}
	inline operator const int&() const {
		return reinterpret_cast<const int&>(m_nVal);;
	}
	inline operator const unsigned int&() const {
		return reinterpret_cast<const unsigned int&>(m_nVal);;
	}
	inline operator const long long&() const {
		return reinterpret_cast<const long long&>(m_nVal);;
	}
	inline operator const unsigned long long&() const {
		return reinterpret_cast<const unsigned long long&>(m_nVal);;
	}
	inline operator const unsigned short&() const {
		return reinterpret_cast<const unsigned short&>(m_nVal);;
	}

	inline operator std::string() const {
		return m_szVal;
	}

	inline std::string str() const {
		return std::string(m_szVal);
	}
	
/*
	template <class T> operator const T&() const {
		return reinterpret_cast<const T&>(m_nVal);
	}

	template <class T> operator const T*() const {
		return reinterpret_cast<const T*>(m_szVal);
	}
*/
	inline operator const double&() const {
		return m_dVal;
	}

	inline operator const char*() const {
		return m_szVal;
	}


protected:
	void Clear();
	void SetString(const char* szVal);
	void SetNumber(const long long& nVal);
	void SetNumber(const double& dVal);
	void SetBool(bool bVal);

private:
//	EType m_eType;

	char* m_szVal;
	long long m_nVal;
	double m_dVal;
};

//#pragma warning(pop)
