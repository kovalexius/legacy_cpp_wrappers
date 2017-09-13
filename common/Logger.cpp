#define _CRT_SECURE_NO_WARNINGS

#include "Logger.h"
#include <stdarg.h>
#include <stdexcept>
#include <time.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#elif defined(linux)
#include <syslog.h>
#include <sys/time.h>
#endif

//#ifdef ANSI             /* ANSI compatible version          */
//#include <stdarg.h>
//int average( int first, ... );
//#else                   /* UNIX compatible version          */
//int average( va_list );
//#endif

CLogger::CLogger(void)
 :	m_eLogLevel(eDoNotLog),
	m_eLogMethod(eLogDisabled),
	m_hFile(0),
	m_szFileName(NULL),
	m_nMaxLogStringLen(0)
{
}

CLogger::~CLogger(void)
{
#ifdef _WIN32
	if (m_hFile) 
	{
		fclose(m_hFile);
		m_hFile = NULL;
		delete [] m_szFileName, m_szFileName = NULL;
	}
#elif defined(linux)
	closelog();
#endif
}

void CLogger::Init(ELogLevel eLogLevel, ELogMethod eLogMethod, const char* szFileName, size_t nMaxLogStringLen /*= 0*/)
{
	if ((m_eLogLevel = eLogLevel) && (eLogMethod == eLogToFile || eLogMethod == eLogToFileAndStdout))
	{
#ifdef _WIN32
		m_hFile = fopen(szFileName, "at");
		if (!m_hFile) throw std::runtime_error("Error opening log file");
#elif defined(linux)
		openlog(szFileName, 0, LOG_USER);
#endif
		m_szFileName = new char[strlen(szFileName) + 1];
		strcpy(m_szFileName, szFileName);
	}
	m_eLogMethod = eLogMethod;
	m_nMaxLogStringLen = nMaxLogStringLen;
	LogLine();
	LogLine();
}

void CLogger::RTrim(char* szString) const
{
	for (int i = (int)strlen(szString); i > 0; i--) 
		if (strchr("\r\n\t ", szString[i])) szString[i] = '\0'; else break;
}

void CLogger::Log(ELogLevel eMessageImportance, const char* szMessageOrFormat, ...) const
{

	if (m_eLogLevel >= eMessageImportance && (m_hFile || m_eLogMethod > 1))
	{
		va_list vl;
		va_start(vl, szMessageOrFormat);
		{	
			char szBuffer[4096] = "";
			// writing date
			if (m_eLogMethod == eLogToFile || m_eLogMethod == eLogToFileAndStdout)
			{
#ifdef _WIN32
				SYSTEMTIME st;
				GetSystemTime(&st);
				snprintf(
					szBuffer, 
					sizeof(szBuffer), 
					"%04d-%02d-%02d %02d:%02d:%02d.%03d ", 
					st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds
				);
#elif defined(linux)
				timeval tv;
				if (!gettimeofday(&tv, NULL))
				{
					char szBuffer[4096];
					tm* pstm = gmtime(&tv.tv_sec);
					snprintf(
						szBuffer, 
						sizeof(szBuffer), 
						"%s: %04d-%02d-%02d %02d:%02d:%02d.%03d ", 
						m_szFileName, 
						pstm->tm_year + 1900, pstm->tm_mon + 1, pstm->tm_mday, 
						pstm->tm_hour, pstm->tm_min, pstm->tm_sec, tv.tv_usec / 1000
					);
				}
#endif
			}
			size_t nDateStrLen = strlen(szBuffer);
			// Writing other
			unsigned nLen = vsnprintf(szBuffer + nDateStrLen, sizeof(szBuffer) - nDateStrLen - 1, szMessageOrFormat, vl);
			if (m_nMaxLogStringLen && nLen >= m_nMaxLogStringLen - 3) 
				*(unsigned*)&szBuffer[m_nMaxLogStringLen - 3] = *(unsigned*)"...";
			if (nLen == sizeof(szBuffer) - 1) szBuffer[sizeof(szBuffer) - 1] = '\0';
			
			RTrim(szBuffer);

			if (m_eLogMethod == eLogToFile || m_eLogMethod == eLogToFileAndStdout)
			{
#ifdef _WIN32
				fputs(szBuffer, m_hFile);
				fputs("\n", m_hFile);
				fflush(m_hFile);
#elif defined(linux)
				syslog(LOG_WARNING, "%s", szBuffer);
#endif
			}
			if (m_eLogMethod == eLogToStdout || m_eLogMethod == eLogToFileAndStdout)
			{
				vprintf(szBuffer + nDateStrLen, vl);
				puts("");
			}
		}
	}
}

void CLogger::LogLine(ELogLevel eMessageImportance)
{
	if (m_eLogLevel >= eMessageImportance)
	{
		if (m_hFile && (m_eLogMethod == 1 || m_eLogMethod == 3))
		{
#ifdef _WIN32
			fprintf(m_hFile, "==================================================================\n");
			fflush(m_hFile);
#elif defined(linux)
			char szBuffer[4096];
			snprintf(szBuffer, sizeof(szBuffer) - 1,  
				"%s: =====================================================================",
				m_szFileName
			);
			syslog(LOG_WARNING, szBuffer);
#endif
		}
//		if (m_eLogMethod == eLogToStdout || m_eLogMethod == eLogToFileAndStdout)
//		{
//			puts("==================================================================");
//		}
	}
}
