#pragma once

#ifdef _WIN32

#include "tstring.h"
#include <windows.h>

// максимальный размер данных, которые можно считать с помощью данного класа
#define MAX_REG_DATA_SIZE	8192

class CRegistry
{
public:
        CRegistry(bool bThrowExceptionOnError = false, const TCHAR* szRoot = _T("software"), HKEY nRootKey = HKEY_CURRENT_USER);
	~CRegistry(void);

	void SetRoot(const TCHAR* szCompanyName, const TCHAR* szAppName = NULL, const TCHAR* szRoot = NULL, HKEY nRootKey = NULL);

	bool DeleteKey(const TCHAR* szSection, const TCHAR* szKeyName) const;

	tstring GetString(const TCHAR* szSection, const TCHAR* szKeyName, const tstring& strDefault) const;
	bool SetString(const TCHAR* szSection, const TCHAR* szKeyName, const tstring& strString) const;
	size_t GetBinary(const TCHAR* szSection, const TCHAR* szKeyName, void* pData, size_t nMaxSize) const;
	bool SetBinary(const TCHAR* szSection, const TCHAR* szKeyName, void* pData, size_t nSize) const;

	inline tstring GetString(const TCHAR* szKeyName, const tstring& strDefault) const {
		return GetString(_T(""), szKeyName, strDefault);
	}
	inline int SetString(const TCHAR* szKeyName, const tstring& strString) const {
		return SetString(_T(""), szKeyName, strString);
	}

protected:
	HKEY GetSectionKey(const TCHAR* szSectionName = _T("")) const;
	bool SaveKey(unsigned nType, const TCHAR* szSection, const TCHAR* szKeyName, const unsigned char* pKeyData, DWORD nKeyDataSize) const;
	bool ReadKey(unsigned nType, const TCHAR* szSection, const TCHAR* szKeyName, unsigned char* pKeyData, DWORD& nKeyDataSize) const;

private:
	bool		m_bThrowExceptionOnErrors;
	HKEY		m_nRootKey;
	tstring		m_strRoot;
	tstring		m_strCompanyName;
	tstring		m_strAppName;
};

#endif
