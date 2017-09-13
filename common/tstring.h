#ifndef __TSTRING__H__
#define __TSTRING__H__

#include <string>
#include <tchar.h>

#if defined (WIN32)||defined(WIN64)
#define tcslen _tcslen
#define tcscpy _tcscpy
#else
#define tcslen strlen
#define tcscpy strcpy
#endif

//#ifdef _WIN32
//#else
//typedef char TCHAR;
//#endif

typedef std::basic_string<TCHAR> tstring;

#endif // #ifndef __TSTRING__H__
