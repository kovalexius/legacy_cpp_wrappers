#pragma once

#include <string>

namespace common
{
	bool IsEqual(std::string strString1, std::string strString2);
	std::string Trim(std::string strSrcString);
	bool IsStringConsist(std::string strString, std::string strSubString);

	std::string ToHex(const void* pData, size_t nDataSize);
	bool FromHex(const std::string& strHexData, void* pBuffer, size_t nMaxDataSize);
//	bool FromHex(const char* szHexDataString, unsigned char* pBuffer, size_t nMaxDataSize);
//	inline bool FromHex(const std::string& strHexData, void* pBuffer, size_t nMaxDataSize) {
//		return FromHex(strHexData.c_str(), pBuffer, nMaxDataSize);	
//	}

	std::string ToUtf8(const wchar_t* wszString);
	std::wstring FromUtf8(const std::string& str);

	void ToBase64(const void* pData, size_t nDataSize, std::string& strBase64);
	bool FromBase64(const std::string& strBase64, void* pData, size_t* pnDataSize);
};

#if !(defined(_WIN32) || defined(_WIN64) || defined(WINDOWS))

#ifndef itoa
void strreverse(char* begin, char* end);
char* itoa(int value, char* str, int base);
char* lltoa(long long value, char* str, int base);
#endif

#ifndef strupr
void strupr(char* pstr);
#endif

#include <strings.h>
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define _snprintf snprintf
#define _vsnprintf vsnprintf

#define _atoi64 atoll
#define _i64toa lltoa

#endif // #if !(defined(_WIN32) || defined(_WIN64) || defined(WINDOWS))
