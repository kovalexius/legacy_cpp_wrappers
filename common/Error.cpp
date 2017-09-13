#define _CRT_SECURE_NO_WARNINGS

#include "Error.h"
#include <sstream>
#include "MLString.h"
#include <string.h>
#include "Thread.h"

#ifdef linux
#include <errno.h>
#endif

typedef std::map<unsigned, tstring> TStackTraceMap;

#ifndef _WIN32
#define _tcsrchr strrchr
#else
#include <windows.h>
#endif

int g_nTest = 0;

static TStackTraceMap g_mapStackTrace;

typedef std::basic_stringstream<TCHAR> tstringstream;

#ifndef truntime_error

truntime_error::truntime_error(const std::runtime_error& rError)
: std::runtime_error("")
{
#ifdef _UNICODE
	size_t nSize = strlen(rError.what());
	wchar_t* awcBuffer = new wchar_t[nSize + 1];
	mbstowcs(awcBuffer, rError.what(), nSize + 1);
	m_strError = awcBuffer;
	delete [] awcBuffer;
#else
	m_strError = rError.what();
#endif
}

#endif

void CError::ThrowStdError(const TCHAR* szFile, int nLine, const tstring& strErrorMessage)
{
	tstringstream strstr;
	const TCHAR* pszFileNamePtr = _tcsrchr(szFile, '\\');
	if (!pszFileNamePtr) pszFileNamePtr = _tcsrchr(szFile, '/');
	if (!pszFileNamePtr) pszFileNamePtr = szFile;
	else pszFileNamePtr++;
	strstr 
		<< strErrorMessage
		<< "\nFile: " << pszFileNamePtr  << ", line: " << nLine 
		<< std::ends;
	throw truntime_error(strstr.str());
}

int CError::GetErrNo()
{
#if defined(_WIN32) || defined (_WIN64) || defined(WINDOWS)
	return ::GetLastError();
#else
	return errno;
#endif // defined(_WIN32) || defined (_WIN64) || defined(WINDOWS)
}

#pragma warning(push)
#pragma warning(disable : 4996)

tstring CError::GetErrStr()
{
#if defined(_WIN32) || defined (_WIN64) || defined(WINDOWS)
	DWORD dwLastError = ::GetLastError();
	TCHAR szBuffer[1024];
	if (dwLastError)
                ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, ::GetLastError(), 0, (LPSTR)szBuffer, sizeof(szBuffer), NULL);
	else _tcscpy(szBuffer, _T("Unknown error\n"));
	return szBuffer;
#else
	return strerror(errno);
#endif // #if defined(_WIN32) || defined (_WIN64) || defined(WINDOWS)
}

#pragma warning(pop)

void CError::ThrowSysError(const TCHAR* szFile, int nLine, const tstring& strErrorMessage)
{
	ThrowStdError(szFile, nLine, strErrorMessage + _T(":") + GetErrStr());
}

#if defined(_WIN32) || defined (_WIN64) || defined(WINDOWS)

#pragma data_seg(".error_shared")
static TStackTraceMap* g_pmapStackTrace = NULL;
#pragma data_seg()
#pragma comment(linker, "/SECTION:.error_shared,RWS")

TStackTraceMap& GetStackTraceMap()
{
    if (!g_pmapStackTrace) g_pmapStackTrace = &g_mapStackTrace;
    return *g_pmapStackTrace;
}

#else

TStackTraceMap& GetStackTraceMap()
{
}

#endif

tstring& CError::GetStackTrace()
{
    const void* q = &OnlyOne::Instance();
	return GetStackTraceMap()[CThread::GetCurrentThreadID()];
}

tstring CError::PopStackTrace()
{
    TStackTraceMap& mapStackTrace = GetStackTraceMap();
	tstring strTrace = mapStackTrace[CThread::GetCurrentThreadID()];
	mapStackTrace.erase(CThread::GetCurrentThreadID());
	return strTrace;
}
