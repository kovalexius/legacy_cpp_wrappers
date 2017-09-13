#pragma once

#ifdef _WIN32

#include "PasswordKeeper.h"
#include <vector>

class CRegistry;

class CPasswordArray
{
public:
	CPasswordArray(void);
	~CPasswordArray(void);

	void AddPassword(const TCHAR* szLogin, const TCHAR* szPassword = NULL);
	void AddPassword(const CPasswordKeeper& rPwd);

	const CPasswordKeeper* GetByIndex(size_t nIndex) const;

	bool SaveToRegistry(const TCHAR* szCryptKey, const CRegistry& rReg, const TCHAR* szSection, const TCHAR* szKeyName);
	bool LoadFromRegistry(const TCHAR* szCryptKey, const CRegistry& rReg, const TCHAR* szSection, const TCHAR* szKeyName);

private:
	std::vector<CPasswordKeeper*> m_vecPasswords;
};

#endif