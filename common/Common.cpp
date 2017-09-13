#include "Common.h"
#include <algorithm>
#include <vector>

#include <string.h>

#ifdef linux
#include <iconv.h>
#else
#include <windows.h>
#endif

namespace common
{
bool IsEqual(std::string strString1, std::string strString2)
{
	return stricmp(Trim(strString1).c_str(), Trim(strString2).c_str()) ? false : true;
}

std::string Trim(std::string strSrcString)
{
	const char* szString = strSrcString.c_str();
	size_t nStrLen = strSrcString.length();

       const char* p = NULL;
	for (p = szString + nStrLen - 1; p != szString && strchr(" \t\r\n", *p); p--);
	size_t nSpacesInBegin = strspn(szString, " \t\r\n");
	return strSrcString.substr(nSpacesInBegin, (p - szString) - nSpacesInBegin + 1);
}

bool IsStringConsist(std::string strString, std::string strSubString)
{
	std::transform(strString.begin(), strString.end(), strString.begin(), tolower);
	std::transform(strSubString.begin(), strSubString.end(), strSubString.begin(), tolower);
	return strstr(strString.c_str(), strSubString.c_str()) ? true : false;
}

std::string ToHex(const void* pData, size_t nDataSize)
{
	std::string strResult;

	const char acHexSymbols[] = "0123456789ABCDEF";
	for (unsigned i = 0; i < nDataSize; i++)
	{
		char c = ((const char*)pData)[i];
		strResult += acHexSymbols[(c & 0xF0) >> 4];
		strResult += acHexSymbols[c & 0x0F];
	}

	return strResult;
}

bool FromHex(const std::string& strHexData, void* pBuffer, size_t nMaxDataSize)
{
	bool bResult = true;

	const char acHexSymbols[] = "0123456789ABCDEF";

	memset(pBuffer, 0, nMaxDataSize);
	if (!strHexData.empty())
	{
		size_t nStrLen = strHexData.size();
		size_t nDataSize = nStrLen / 2;
		if (nDataSize <= nMaxDataSize && !(nStrLen % 2))
		{
			for (unsigned i = 0; bResult && i < nStrLen; i += 2)
			{
				unsigned char& ucByteVal = ((unsigned char*)pBuffer)[i / 2];
				ucByteVal = 0;
				for (unsigned j = 0; bResult && j < 2; j++)
				{
					ucByteVal <<= 4;
					char c = strHexData[i + j];
					if (c >= '0' && c <= '9') ucByteVal += c - '0';
					else if (c >= 'a' && c <= 'f') ucByteVal += c - 'a' + 10;
					else if (c >= 'A' && c <= 'F') ucByteVal += c - 'A' + 10;
					else bResult = false;
				}
			}
		} 
		else bResult = false;
	}

	return bResult;

}

/*
bool FromHex(const char* szHexDataString, unsigned char* pBuffer, size_t nMaxDataSize)
{
	bool bResult = true;

	const char acHexSymbols[] = "0123456789ABCDEF";

	memset(pBuffer, 0, nMaxDataSize);
	if (szHexDataString && *szHexDataString)
	{
		unsigned nStrLen = (unsigned)strlen(szHexDataString);
		unsigned nDataSize = nStrLen / 2;
		if (nDataSize <= nMaxDataSize && !(nStrLen % 2))
		{
			for (unsigned i = 0; bResult && i < nStrLen; i += 2)
			{
				unsigned char& ucByteVal = pBuffer[i / 2];
				ucByteVal = 0;
				for (unsigned j = 0; bResult && j < 2; j++)
				{
					ucByteVal <<= 4;
					char c = szHexDataString[i + j];
					if (c >= '0' && c <= '9') ucByteVal += c - '0';
					else if (c >= 'a' && c <= 'f') ucByteVal += c - 'a' + 10;
					else if (c >= 'A' && c <= 'F') ucByteVal += c - 'A' + 10;
					else bResult = false;
				}
			}
		} 
		else bResult = false;
	}

	return bResult;
}
*/

std::string ToUtf8(const wchar_t* wszString)
{
	int nStrLen = (int) wcslen(wszString) + 1;
        size_t nBufSize = nStrLen * 4;
	std::vector<char> vecBuffer;
	vecBuffer.resize(nBufSize, 0);
	char* szStr = &*vecBuffer.begin();

#ifdef linux
	iconv_t cd = iconv_open("UTF-8", "WCHAR_T");
	if (cd != (iconv_t)-1)
	{
                size_t nBytesLeft = nStrLen;
                iconv(cd, (char**)&wszString, &nBytesLeft, &szStr, &nBufSize);
		iconv_close(cd);
	}
#else
	WideCharToMultiByte(CP_UTF8, 0, wszString, nStrLen, szStr, nBufSize, NULL, NULL);
#endif

	return szStr;
}

std::wstring FromUtf8(const std::string& str)
{
	size_t nStrLen = str.size() + 1;
        size_t nBufSize = int(nStrLen);
	std::vector<wchar_t> vecBuffer;
	vecBuffer.resize(nBufSize, 0);
	wchar_t* wszStr = &*vecBuffer.begin();

#ifdef linux
	iconv_t cd = iconv_open("WCHAR_T", "UTF-8");
	if (cd != (iconv_t)-1)
	{
                const char* szFooPtr = str.c_str();
                iconv(cd, (char**)&szFooPtr, &nStrLen, (char**)&wszStr, &nBufSize);
		iconv_close(cd);
	}
#else
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wszStr, nBufSize);
#endif

	return wszStr;
}


};

#ifdef linux

#ifndef itoa

void strreverse(char* begin, char* end) {
	
	char aux;
	
	while(end>begin)
	
		aux=*end, *end--=*begin, *begin++=aux;
	
}
	
char* itoa(int value, char* str, int base) 
{
	static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char* wstr = str;
	int sign;
	div_t res;
	
	// Validate base
	if (base < 2 || base > 35) { *wstr = '\0'; return str; }
	
	// Take care of sign
	if ((sign = value) < 0) value = -value;
	
	// Conversion. Number is reversed.
	do {
		res = div(value, base);
		*wstr++ = num[res.rem];
        } while ((value = res.quot));
	
	if (sign < 0) *wstr ++= '-';
	*wstr = '\0';
		
	// Reverse string
	strreverse(str, wstr - 1);
	
	return str;
	
}

char* lltoa(long long value, char* str, int base)
{
        static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
        char* wstr = str;
        long long sign;
        div_t res;

        // Validate base
        if (base < 2 || base > 35) { *wstr = '\0'; return str; }

        // Take care of sign
        if ((sign = value) < 0) value = -value;

        // Conversion. Number is reversed.
        do {
                res = div(value, base);
                *wstr++ = num[res.rem];
        } while ((value = res.quot));

        if (sign < 0) *wstr ++= '-';
        *wstr = '\0';

        // Reverse string
        strreverse(str, wstr - 1);

        return str;

}

#endif

#ifndef strupr
#include <ctype.h>

void strupr(char *p)
{
	while (*p)
	*(p++) = toupper(*p);
}
#endif
#endif // #ifdef linux

void common::ToBase64(const void* pData, size_t nDataSize, std::string& strBase64)
{
	const char* acEncTbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	char cPad = '=';
	size_t nBase64Len = (nDataSize + 2) / 3 * 4;
	strBase64.resize(nBase64Len, cPad);

	/*
	struct {
		unsigned char nPrevByteMask;
		unsigned char nCurByteMask;
		unsigned char nPrevByteShift;
		unsigned char nCurByteShift;
	} as64[4] = { {0x00, 0xFC, 0, 2}, {0x03, 0xF0, 4, 4}, {0x0F, 0xC0, 2, 6}, {0x3F, 0x00, 0, 0} };

	for (size_t i = 0; i < nDataSize; i += 3)
	{
		unsigned char nPrevByte = 0;
		for (size_t j = 0; j < 4 && i + j <= nDataSize; j++)
		{
			unsigned char nCurByte = j < 3 ? ((const char*)pData)[i + j] : 0;
			strBase64.at(i / 3 * 4 + j) = 
				acEncTbl[
					((nPrevByte & as64[j].nPrevByteMask) << as64[j].nPrevByteShift) |
					((nCurByte & as64[j].nCurByteMask) >> as64[j].nCurByteShift)
				];
			nPrevByte = nCurByte;
		}
	}
*/
	size_t j = 0;
	for (size_t i = 0; i < nDataSize; i += 3, j += 4)
	{
		unsigned& n = (unsigned&)((unsigned char*)pData)[i];
		(unsigned&)strBase64.at(j) = 
			acEncTbl[((n & 0xFC) >> 2)] |
			(acEncTbl[((n & 0x03) << 4) | ((n & 0xF000) >> 12)] << 8) |
			((i + 1 < nDataSize ? acEncTbl[((n & 0xF00) >> 6) | ((n & 0xC00000) >> 22)] : '=') << 16) |
			((i + 2 < nDataSize ? acEncTbl[(n & 0x3F0000) >> 16] : '=') << 24);

	}
}

bool common::FromBase64(const std::string& strBase64, void* pData, size_t* pnDataSize)
{
	const char* acEncTbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        char acDecTbl[257];
        acDecTbl[256] = '\0';
	memset(acDecTbl, 0, 256);
	for (unsigned i = 0; acEncTbl[i]; i++) acDecTbl[acEncTbl[i]] = i;
	size_t nLen = strBase64.size();
	size_t nFinalLen = (nLen + 3) / 4 * 3;

	bool bResult = true;
	if (*pnDataSize >= nFinalLen)
	{
		memset(pData, 0, nFinalLen);

		for (unsigned i = 0; i < nFinalLen / 3; i++)
		{
			const unsigned& n = (unsigned&) strBase64.at(i * 4);
			const unsigned char ac[4] = {acDecTbl[n & 0xFF], acDecTbl[(n & 0xFF00) >> 8], acDecTbl[(n & 0xFF0000) >> 16], acDecTbl[(n & 0xFF000000) >> 24]};
			(unsigned&)((char*)pData)[i * 3] |= 
				(ac[0] << 2) | ((ac[1] & 0x30) >> 4) |
				((((ac[1] & 0x0F) << 4) | ((ac[2] & 0x3C) >> 2)) << 8) |
				((((ac[2] & 0x03) << 6) | ac[3]) << 16);
		}
		size_t nTrailsCnt = 0;
		for (std::string::const_reverse_iterator ix = strBase64.rbegin(); ix != strBase64.rend() && *ix == '='; ix++, nTrailsCnt++);
		*pnDataSize = nFinalLen - (nTrailsCnt + 1) * 3 / 4;
	}
	else 
	{
		bResult = false;
		*pnDataSize = nFinalLen;
	}

	return bResult;
}
