#pragma once

#ifdef _WIN32
#define MLSTRING_MAX_SIZE	1024

#include "tstring.h"
//#include "resource.h"
//#include <string>
//#include <tchar.h>

class CMLString
{
public:
	CMLString(int nStringID, ...);
	~CMLString(void);

	inline const TCHAR* c_str() const { return m_str.c_str(); }
	inline operator const tstring&() { return m_str; }
	inline const tstring& str() { return m_str; }

	static void* GetInstance();

private:
	tstring m_str;
	static void* m_hInstance;
};

#endif
