// #include "tstring.h"
#include "Registry.h"

#ifdef _WIN32

//#include <winerror.h>

CRegistry::CRegistry(bool bThrowExceptionOnError /*= false*/, const TCHAR* szRoot /*= _T("software")*/, HKEY nRootKey /*= HKEY_CURRENT_USER*/)
 :	m_nRootKey(nRootKey),
	m_strRoot(szRoot)
{
}

CRegistry::~CRegistry(void)
{
}

void CRegistry::SetRoot(const TCHAR* szCompanyName, const TCHAR* szAppName /*= NULL*/, const TCHAR* szRoot /*= NULL*/, HKEY nRootKey /*= NULL*/)
{
	m_nRootKey = HKEY_CURRENT_USER;
	if (szRoot && *szRoot) m_strRoot = szRoot;
	if (szCompanyName && *szCompanyName) m_strCompanyName = szCompanyName;
	if (szAppName && *szAppName) m_strAppName = szAppName;
}

HKEY CRegistry::GetSectionKey(const TCHAR* szSectionName/*= _T("")*/) const
{
	HKEY hAppKey = NULL;
	HKEY hSoftKey = NULL;
	HKEY hCompanyKey = NULL;
	HKEY hSectionKey = NULL;

	if (RegOpenKeyEx(m_nRootKey, m_strRoot.c_str(), 0, KEY_WRITE|KEY_READ, &hSoftKey) == ERROR_SUCCESS)
	{
		DWORD dw;
		if (RegCreateKeyEx(hSoftKey, m_strCompanyName.c_str(), 0, REG_NONE,
			REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
			&hCompanyKey, &dw) == ERROR_SUCCESS)
		{
			RegCreateKeyEx(hCompanyKey, m_strAppName.c_str(), 0, REG_NONE,
				REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
				&hAppKey, &dw);
		}
	}
	if (hSoftKey != NULL)
		RegCloseKey(hSoftKey);
	if (hCompanyKey != NULL)
		RegCloseKey(hCompanyKey);

	DWORD dw;
	RegCreateKeyEx(hAppKey, szSectionName, 0, REG_NONE,
		REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
		&hSectionKey, &dw);
	RegCloseKey(hAppKey);

	return hSectionKey;
}

bool CRegistry::SaveKey(unsigned nType, const TCHAR* szSection, const TCHAR* szKeyName, const unsigned char* pKeyData, DWORD nKeyDataSize) const
{
	bool bResult = false;

	HKEY hSecKey = GetSectionKey(szSection);
	if (hSecKey != NULL)
	{
		LONG lResult = RegSetValueEx(hSecKey, szKeyName, NULL, nType, pKeyData, DWORD(nKeyDataSize));
		RegCloseKey(hSecKey);
		bResult = (lResult == ERROR_SUCCESS);
	}
	return bResult;
}

bool CRegistry::ReadKey(unsigned nType, const TCHAR* szSection, const TCHAR* szKeyName, unsigned char* pKeyData, DWORD& nKeyDataSize) const
{
	bool bResult = false;

	HKEY hSecKey = GetSectionKey(szSection);
	if (hSecKey != NULL)
	{
		DWORD dwType;
		if (ERROR_SUCCESS == RegQueryValueEx(hSecKey, szKeyName, NULL, &dwType, NULL, NULL)
			&& dwType == nType)
		{
			bResult = (ERROR_SUCCESS == RegQueryValueEx(hSecKey, szKeyName, NULL, &dwType, pKeyData, &nKeyDataSize));
		}
		RegCloseKey(hSecKey);
	}
	return bResult;
}

bool CRegistry::DeleteKey(const TCHAR* szSection, const TCHAR* szKeyName) const
{
	bool bResult = false;
	if (!m_strCompanyName.empty() || !m_strAppName.empty() || tcslen(szSection) || tcslen(szKeyName))
	{	// пользователь не должен пытаться удалить root
		HKEY hSecKey = GetSectionKey(szSection);
		if (hSecKey != NULL)
		{
			bResult = (RegDeleteKey(hSecKey, szKeyName) == ERROR_SUCCESS);
			RegCloseKey(hSecKey);
		}
	}
	return bResult;
}

tstring CRegistry::GetString(const TCHAR* szSection, const TCHAR* szKeyName, const tstring& strDefault) const
{
	TCHAR szBuf[MAX_REG_DATA_SIZE];
	DWORD nSize = MAX_REG_DATA_SIZE;
	if (ReadKey(REG_SZ, szSection, szKeyName, (unsigned char*)szBuf, nSize))
		return szBuf;
	else return _T("");
}

bool CRegistry::SetString(const TCHAR* szSection, const TCHAR* szKeyName, const tstring& strString) const
{
	return SaveKey(REG_SZ, szSection, szKeyName, (unsigned char*)strString.c_str(), DWORD(strString.size()));
}

size_t CRegistry::GetBinary(const TCHAR* szSection, const TCHAR* szKeyName, void* pData, size_t nMaxSize) const
{
	DWORD nSize = DWORD(nMaxSize);
	return ReadKey(REG_BINARY, szSection, szKeyName, (unsigned char*)pData, nSize) ?
		nSize : 0;
}

bool CRegistry::SetBinary(const TCHAR* szSection, const TCHAR* szKeyName, void* pData, size_t nSize) const
{
	return SaveKey(REG_BINARY, szSection, szKeyName, (unsigned char*)pData, DWORD(nSize));
}

#endif
