#pragma once

#include "tstring.h"
#include <stdexcept>
#include <map>

#ifndef _UNICODE
#define truntime_error std::runtime_error
#else
class truntime_error : public std::runtime_error
{
public:
	truntime_error(const tstring& strError) : std::runtime_error(""), m_strError(strError) {}
	truntime_error(const std::runtime_error& rError);
	const TCHAR* what() { return m_strError.c_str(); }

private:
	tstring m_strError;
};
#endif

class CError
{
public:
	static void ThrowStdError(const TCHAR* szFile, int nLine, const tstring& strErrorMessage);
	static void ThrowSysError(const TCHAR* szFile, int nLine, const tstring& strErrorMessage);
	
	static tstring& GetStackTrace();
	static tstring PopStackTrace();

	static int GetErrNo();
	static tstring GetErrStr();
};

class OnlyOne
{
public:
    static const OnlyOne& Instance()
    {
        static OnlyOne theSingleInstance;
        return theSingleInstance;
    }
private:        
    OnlyOne(){};
    OnlyOne(const OnlyOne& root);
    const OnlyOne& operator=(OnlyOne&);
};

#ifndef _TRY_DONOTCATCH
#pragma warning(disable : 4005)

#ifdef linux 
#define __FUNCTION__ __func__
#endif

#define STACK_TRACE (CError::PopStackTrace() + _T("\n") + _T(__FUNCTION__))

#define RETHROW \
	catch(truntime_error &e) {\
		if (CError::GetStackTrace().empty()) CError::GetStackTrace() = e.what();\
		CError::GetStackTrace() += tstring(_T("\n")) +  _T(__FUNCTION__);\
		throw; }\
	catch(...) {\
		if (CError::GetStackTrace().empty()) CError::GetStackTrace() = _T("Unhandled exception");\
		CError::GetStackTrace() += tstring(_T("\n")) + _T(__FUNCTION__);\
		throw; }

#define LAST_RETHROW \
    catch(truntime_error &e) {\
        if (CError::GetStackTrace().empty()) CError::GetStackTrace() = e.what();\
        CError::GetStackTrace() += tstring(_T("\n")) +  _T(__FUNCTION__);\
        throw; }\
    catch(...) {\
        if (CError::GetStackTrace().empty()) CError::GetStackTrace() = _T("Unhandled exception");\
        CError::GetStackTrace() += tstring(_T("\n")) + _T(__FUNCTION__);\
        throw ((const char*)CError::GetStackTrace().c_str()); }
/*
#define LOG_AND_RETHROW(logger) \
	catch(std::runtime_error &e) {\
		if (CError::GetStackTrace().empty()) CError::GetStackTrace() = e.what();\
		logger->Log(eLogAnyway, (CError::GetStackTrace() += "\n" + strCurFuncName__).c_str());\
		throw; }\
	catch(...) {\
		if (CError::GetStackTrace().empty()) CError::GetStackTrace() = "Unhandled exception";\
		logger->Log(eLogAnyway, (CError::GetStackTrace() += "\n" + strCurFuncName__).c_str());\
		throw; }
*/

#define LOG_AND_RETHROW(logger) \
	catch(truntime_error &e) {\
		if (CError::GetStackTrace().empty()) CError::GetStackTrace() = e.what();\
		logger->Log(eLogAnyway, (CError::GetStackTrace() += tstring("\n") + __FUNCTION__).c_str());\
		throw; }\
	catch(...) {\
		if (CError::GetStackTrace().empty()) CError::GetStackTrace() = "Unhandled exception";\
		logger->Log(eLogAnyway, (CError::GetStackTrace() += tstring("\n") + __FUNCTION__).c_str());\
		throw; }

#else
#define TRY(x) NULL;
#define RETHROW NULL;
#endif

#pragma warning(default : 4005)

#define STDERROR(message) CError::ThrowStdError(_T(__FILE__), __LINE__, _T(message))
#define SYSERROR(message) CError::ThrowSysError(_T(__FILE__), __LINE__, _T(message))

#ifdef _WIN32
#define STDERRORX(message_id) CError::ThrowStdError(__FILE__, __LINE__, CMLString(_T(message_id)))
#define SYSERRORX(message_id) CError::ThrowSysError(__FILE__, __LINE__, CMLString(_T(message_id)))
#endif

#define INTRYDLOG printf("%s,%s,%d\n",__FUNCTION__,__FILE__,__LINE__)
#define DLOG printf("%s,%d\n",__FILE__,__LINE__)

