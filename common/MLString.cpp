#define _CRT_SECURE_NO_WARNINGS

#include "MLString.h"

#ifdef _WIN32

#include <stdarg.h>
#include <windows.h>

void* CMLString::m_hInstance = CMLString::GetInstance();

CMLString::CMLString(int nStringID, ...)
{
	va_list vl;
	va_start(vl, nStringID);
	TCHAR szStr[MLSTRING_MAX_SIZE];
	TCHAR szBuf[MLSTRING_MAX_SIZE];

	LoadString((HINSTANCE)m_hInstance, nStringID, szStr, MLSTRING_MAX_SIZE);
	_vsntprintf(szBuf, MLSTRING_MAX_SIZE, szStr, vl);
	
	m_str = szBuf;
}

CMLString::~CMLString(void)
{
}

void* CMLString::GetInstance()
{
	MEMORY_BASIC_INFORMATION mem;
	if (VirtualQuery(CMLString::GetInstance, &mem, sizeof(mem)))
	{
		if (mem.Type == MEM_IMAGE && mem.AllocationBase != NULL)
			m_hInstance = mem.AllocationBase;
	} else m_hInstance = NULL;
	if (!m_hInstance) throw std::runtime_error("Unable to initialize MLString subsystem");
	return m_hInstance;
}

#endif