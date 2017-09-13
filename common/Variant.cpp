#define _CRT_SECURE_NO_WARNINGS

#include "Variant.h"
#include <string.h>
#include "Common.h"
#include <stdlib.h>
#include <stdio.h>

CVariant::~CVariant(void)
{
	Clear();
}

CVariant& CVariant::operator =(const CVariant &rRight)
{
	if (rRight.m_szVal)
	{
		size_t nLen = strlen(rRight.m_szVal);
		if (nLen) 
		{
			m_szVal = new char[nLen + 1];
			strcpy(m_szVal, rRight.m_szVal);
		}
	}
	m_nVal = rRight.m_nVal;
	m_dVal = rRight.m_dVal;

	return *this;
}

void CVariant::SetString(const char* szVal)
{
	size_t nLen = 0;
	if (szVal) nLen = strlen(szVal);
	if (nLen) 
	{
		m_szVal = new char[nLen + 1];
		strcpy(m_szVal, szVal);
		m_nVal = _atoi64(szVal);
		m_dVal = atof(szVal);
	}
	else 
	{
		m_szVal = 0;
		m_nVal = 0;
		m_dVal = 0;
	}
}

void CVariant::Clear()
{
	if (m_szVal) delete [] m_szVal, m_szVal = 0;
	m_nVal = 0;
	m_dVal = 0;
}

void CVariant::SetNumber(const long long& nVal)
{
	m_nVal = nVal;
	m_dVal = (double)nVal;
	if (m_szVal) delete [] m_szVal, m_szVal = 0;
	m_szVal = new char[21];
	_i64toa(nVal, m_szVal, 10);
}

void CVariant::SetNumber(const double& dVal)
{
	m_nVal = (int)dVal;
	m_dVal = dVal;
	if (m_szVal) delete [] m_szVal, m_szVal = 0;
	m_szVal = new char[21];
	_snprintf(m_szVal, 21, "%.20g", dVal);
}

void CVariant::SetBool(bool bVal)
{
	m_nVal = bVal;
	m_dVal = bVal;
	if (m_szVal) delete [] m_szVal, m_szVal = 0;
	m_szVal = new char[6];
	strcpy(m_szVal, bVal ? "true" : "false");
}

const char* CVariant::operator =(const char* szVal) 
{
	Clear();
	SetString(szVal);
	return m_szVal;
}

const long long CVariant::operator =(const int nVal) 
{
	Clear();
	SetNumber((long long)nVal);
	return nVal;
}

bool CVariant::operator = (bool bVal) 
{
	Clear();
	SetBool(bVal);
	return bVal;
}
