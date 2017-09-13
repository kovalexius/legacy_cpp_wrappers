#define _CRT_SECURE_NO_WARNINGS

#include "PasswordKeeper.h"

#ifdef _WIN32

#include "time.h"
#include "auto_array.h"
#include <stdlib.h>

CPasswordKeeper::CPasswordKeeper(void)
{
}

CPasswordKeeper::CPasswordKeeper(const CPasswordKeeper& rRight)
{
	*this = rRight;
}

CPasswordKeeper::CPasswordKeeper(const TCHAR* szLoginOrPassword, const TCHAR* szPassword/*= NULL*/)
{
	SetData(szLoginOrPassword, szPassword);
}

CPasswordKeeper::~CPasswordKeeper(void)
{
	Free();
}

void CPasswordKeeper::Encrypt(void* pDataPtr, size_t nDataSize) const
{
	for (size_t i = 0; i < nDataSize; i++)
	{
		for (size_t j = 0; j < nDataSize; j++)
		{
			((char*)pDataPtr)[j] ^= ((char*)pDataPtr)[(j + 1) % nDataSize] ^ ((char*)m_sKey.m_pData)[(i + j) % m_sKey.m_nSize];
		}
	}
}

void CPasswordKeeper::Decrypt(void* pDataPtr, size_t nDataSize) const
{
	for (size_t i = nDataSize; i > 0; i--)
	{
		for (size_t j = nDataSize; j > 0; j--)
		{
			((char*)pDataPtr)[j - 1] ^= ((char*)pDataPtr)[j % nDataSize] ^ ((char*)m_sKey.m_pData)[(i + j - 2) % m_sKey.m_nSize];
		}
	}
}

void CPasswordKeeper::GenerateNewKey()
{
	if (m_sKey.m_pData) delete [] m_sKey.m_pData, m_sKey.m_pData = NULL;

	// случайный размер от 16 до 32
	srand((unsigned)time(NULL));
	m_sKey.m_nSize = rand() * 16 / RAND_MAX + 16;
	// случайный ключ
	m_sKey.m_pData = new char[m_sKey.m_nSize];
	for (size_t i = 0; i < m_sKey.m_nSize; i++)
		((char*)m_sKey.m_pData)[i] = rand() * 255 / RAND_MAX;
}

void CPasswordKeeper::SetData(const TCHAR* szLoginOrPassword, const TCHAR* szPassword/*= NULL*/)
{
	GenerateNewKey();
	if (m_sLogin.m_pData) delete [] (TCHAR*)m_sLogin.m_pData, m_sLogin.m_pData = NULL;
	if (m_sPassword.m_pData) delete [] (TCHAR*)m_sPassword.m_pData, m_sPassword.m_pData = NULL;

	if (szPassword)
	{
		m_sLogin.m_nSize = _tcslen(szLoginOrPassword) * sizeof(TCHAR);
		m_sLogin.m_pData = _tcsncpy(new TCHAR[m_sLogin.m_nSize], szLoginOrPassword, m_sLogin.m_nSize);
		m_sPassword.m_nSize = _tcslen(szPassword)  * sizeof(TCHAR);
		m_sPassword.m_pData = _tcsncpy(new TCHAR[m_sPassword.m_nSize], szPassword, m_sPassword.m_nSize);
	}
	else if (szLoginOrPassword)
	{
		m_sPassword.m_nSize = _tcslen(szLoginOrPassword) * sizeof(TCHAR);
		m_sPassword.m_pData = _tcsncpy(new TCHAR[m_sPassword.m_nSize], szLoginOrPassword, m_sPassword.m_nSize);
	}
	if (m_sLogin.m_pData) Encrypt(m_sLogin.m_pData, m_sLogin.m_nSize);
	if (m_sPassword.m_pData) Encrypt(m_sPassword.m_pData, m_sPassword.m_nSize);
}

tstring CPasswordKeeper::GetLogin() const
{
	tstring strResult;
	if (m_sLogin.m_pData)
	{
		auto_array<TCHAR> szBuffer(new TCHAR[m_sLogin.m_nSize + 1]);
		memcpy(szBuffer, m_sLogin.m_pData, m_sLogin.m_nSize);
		Decrypt(szBuffer, m_sLogin.m_nSize);
		szBuffer[m_sLogin.m_nSize] = '\0';
		strResult = szBuffer;
	}
	return strResult;
}

tstring CPasswordKeeper::GetPassword() const
{
	tstring strResult;
	if (m_sPassword.m_pData)
	{
		auto_array<TCHAR> szBuffer(new TCHAR[m_sPassword.m_nSize + 1]);
		memcpy(szBuffer, m_sPassword.m_pData, m_sPassword.m_nSize);
		Decrypt(szBuffer, m_sPassword.m_nSize);
		szBuffer[m_sPassword.m_nSize] = '\0';
		strResult = szBuffer;
	}
	return strResult;
}

void CPasswordKeeper::SaveToRegistry(const TCHAR* szCryptKey, const CRegistry& rReg, const TCHAR* szSection, const TCHAR* szKeyName)
{
	// Выделяем буфер необходимого размера
	size_t nBufSize = m_sLogin.m_nSize + m_sPassword.m_nSize + sizeof(m_sLogin.m_nSize) + sizeof(m_sPassword.m_nSize);
	auto_array<unsigned char> pBuf(new unsigned char[nBufSize]);
	unsigned char* pCurPtr = pBuf;
	// сериализуем расшифрованный логин и пароль и их длины для записи в реестр
	SData asData[2] = {m_sLogin, m_sPassword};
	for (size_t i = 0; i < 2; i++)
	{
		if (asData[i].m_pData) 
		{
			memcpy(pCurPtr, &asData[i].m_nSize, sizeof(asData[i].m_nSize));
			pCurPtr += sizeof(m_sLogin.m_nSize);
			memcpy(pCurPtr, asData[i].m_pData, asData[i].m_nSize);
			Decrypt(pCurPtr, asData[i].m_nSize);
		}
	}
	// перешифровываем данные другим ключом
	Encrypt(pBuf, nBufSize);
	// записываем
	rReg.SetBinary(szSection, szKeyName, pBuf, nBufSize);
}

bool CPasswordKeeper::LoadFromRegistry(const TCHAR* szCryptKey, const CRegistry& rReg, const TCHAR* szSection, const TCHAR* szKeyName)
{
	bool bResult = false;
	size_t nSize = rReg.GetBinary(szSection, szKeyName, NULL, 0);
	auto_array<unsigned char> pBuf(new unsigned char[nSize]);
	Decrypt(pBuf, nSize);
	if (nSize)
	{
		size_t nLoginSize = *(size_t*)(unsigned char*)pBuf;
		if (nLoginSize < nSize)
		{
			// проверяем, есть ли вообще пароль
			size_t nPasswordSize = (nLoginSize + sizeof(size_t) < nSize) ? 
				*(size_t*)(pBuf + nLoginSize + sizeof(size_t)) : 0;
			if (nPasswordSize < nSize && nPasswordSize + nLoginSize < nSize)
			{	// похоже на корректные данные
				if (m_sLogin.m_pData) delete [] m_sLogin.m_pData, m_sLogin.m_pData = NULL;
				if (m_sPassword.m_pData) delete [] m_sPassword.m_pData, m_sPassword.m_pData = NULL;
				if (nPasswordSize)
				{
					m_sLogin.m_pData = new unsigned char[nLoginSize];
					m_sLogin.m_nSize = nLoginSize;
					memcpy(m_sLogin.m_pData, pBuf + sizeof(size_t), nLoginSize);

					m_sPassword.m_pData = new unsigned char[nPasswordSize];
					m_sPassword.m_nSize = nPasswordSize;
					memcpy(m_sPassword.m_pData, pBuf + sizeof(size_t) * 2 + nLoginSize, m_sPassword.m_nSize);
				}
				GenerateNewKey();
				Encrypt(m_sLogin.m_pData, m_sLogin.m_nSize);
				Encrypt(m_sPassword.m_pData, m_sPassword.m_nSize);
				
				bResult = true;
			}
		}
	}

	return bResult;
}

CPasswordKeeper& CPasswordKeeper::operator = (const CPasswordKeeper& rRight)
{
	SData sNewLogin, sNewPassword, sNewKey;
	try
	{
		if (rRight.m_sLogin.m_pData)
		{
			sNewLogin.m_pData = new unsigned char[rRight.m_sLogin.m_nSize];
			sNewLogin.m_nSize = rRight.m_sLogin.m_nSize;
			memcpy(sNewLogin.m_pData, rRight.m_sLogin.m_pData, sNewLogin.m_nSize);
		}
		if (rRight.m_sPassword.m_pData)
		{
			sNewPassword.m_pData = new unsigned char[rRight.m_sPassword.m_nSize];
			sNewPassword.m_nSize = rRight.m_sPassword.m_nSize;
			memcpy(sNewPassword.m_pData, rRight.m_sPassword.m_pData, sNewPassword.m_nSize);
		}
		if (rRight.m_sKey.m_pData)
		{
			sNewKey.m_pData = new unsigned char[rRight.m_sKey.m_nSize];
			sNewKey.m_nSize = rRight.m_sKey.m_nSize;
			memcpy(sNewKey.m_pData, rRight.m_sKey.m_pData, sNewKey.m_nSize);
		}
		void* pOldLoginData = m_sLogin.m_pData;
		void* pOldPasswordData = m_sPassword.m_pData;
		void* pOldKeyData = m_sKey.m_pData;
		m_sLogin = sNewLogin;
		m_sPassword = sNewPassword;
		m_sKey = sNewKey;
		if (pOldLoginData) delete [] pOldLoginData;
		if (pOldPasswordData) delete [] pOldPasswordData;
		if (pOldKeyData) delete [] pOldKeyData;
	}
	catch(...)
	{
		if (sNewLogin.m_pData) delete [] sNewLogin.m_pData;
		if (sNewPassword.m_pData) delete [] sNewPassword.m_pData;
		if (sNewKey.m_pData) delete [] sNewKey.m_pData;
		throw;
	}

	return *this;
}

void CPasswordKeeper::Free()
{
	if (m_sKey.m_pData) delete [] m_sKey.m_pData, m_sKey.m_pData = NULL;
	if (m_sLogin.m_pData) delete [] m_sLogin.m_pData, m_sLogin.m_pData = NULL;
	if (m_sPassword.m_pData) delete [] m_sPassword.m_pData, m_sPassword.m_pData = NULL;
}

#endif