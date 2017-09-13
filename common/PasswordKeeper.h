#pragma once

#ifdef _WIN32

#include "tstring.h"
#include "Registry.h"

class CPasswordKeeper
{
	friend class CPasswordArray;
public:
	CPasswordKeeper(void);
	CPasswordKeeper(const CPasswordKeeper& rRight);
	CPasswordKeeper(const TCHAR* szLoginOrPassword, const TCHAR* szPassword = NULL);
	~CPasswordKeeper(void);

	void SetData(const TCHAR* szLoginOrPassword, const TCHAR* szPassword = NULL);

	void SaveToRegistry(const TCHAR* szCryptKey, const CRegistry& rReg, const TCHAR* szSection, const TCHAR* szKeyName);
	bool LoadFromRegistry(const TCHAR* szCryptKey, const CRegistry& rReg, const TCHAR* szSection, const TCHAR* szKeyName);

	tstring GetLogin() const;
	tstring GetPassword() const;
	inline tstring GetData() const {
		return GetPassword();
	}

	inline size_t GetLoginSize() const { return m_sLogin.m_nSize; }
	inline size_t GetPasswordSize() const { return m_sPassword.m_nSize; }
	inline size_t GetDataSize() const { return m_sPassword.m_nSize; }

	CPasswordKeeper& operator = (const CPasswordKeeper& rRight);

	void Free();

protected:
	void Encrypt(void* pDataPtr, size_t nDataSize) const;
	void Decrypt(void* pDataPtr, size_t nDataSize) const;
	void GenerateNewKey();

private:
	struct SData
	{
		void* m_pData;
		size_t m_nSize;
		SData() : m_pData(NULL), m_nSize(0) { }
	};

	SData m_sLogin;
	SData m_sPassword;
	SData m_sKey;
};

#endif