#pragma once

#include <stdio.h>

enum ELogLevel
{
	eDoNotLog = 0,
	eLogAnyway = 1,
	eLogWarning = 2,
	eLogComment = 3
};

enum ELogMethod
{
	eLogDisabled = 0,
	eLogToFile,
	eLogToStdout,
	eLogToFileAndStdout
};

class CLogger
{
public:
	CLogger();
	~CLogger(void);

	void Init(ELogLevel eLogLevel, ELogMethod eLogMethod, const char* szFileName, size_t nMaxLogStringLen = 0);

	void Log(ELogLevel eMessageImportance, const char* szMessageOrFormat, ...) const;
	void LogLine(ELogLevel eMessageImportance = eLogAnyway);

	inline ELogLevel GetLogLevel() const {
		return m_eLogLevel;
	}

protected:
	void RTrim(char* szString) const;

private:
	ELogLevel	m_eLogLevel;
	ELogMethod	m_eLogMethod;
	FILE*		m_hFile;
	char*		m_szFileName;
	size_t		m_nMaxLogStringLen;
};

