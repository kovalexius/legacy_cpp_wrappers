#include "Common.h"
#include "ComPort.h"
#include "Error.h"
#include <limits.h>
#include <stdio.h>
#include <fcntl.h>
#include <memory>
#include <string.h>



#ifndef WINDOWS
#include <termios.h>
#include <unistd.h> 
#include <stdarg.h>
#include <sys/ioctl.h>
#ifndef MAX_PATH
#define MAX_PATH 253
#endif
#endif // #ifndef WINDOWS

#pragma warning(push)
#pragma warning(disable : 4996)

CComPort::CComPort(void)
{
	m_sOverlappedRd.hEvent = INVALID_HANDLE_VALUE;
	m_sOverlappedWr.hEvent = INVALID_HANDLE_VALUE;
}

CComPort::~CComPort(void)
{
	if (m_hPort) ClosePort(m_hPort);
}

void CComPort::GetComPortFileName(int nComPortNum, char* szBuffer, int nBufferSize)
{
	const char* szPortFileMask = NULL;
	char cPortID = '0' + nComPortNum;
#ifdef WINDOWS // Windows
	szPortFileMask = "COM%c";
#elif defined(sgi) || defined(__sgi) // IRIX
	szPortFileMask = "/dev/ttyf%c";
#elif defined(sun) || defined(__sun) //SOLARIS
	szPortFileMask = "/dev/tty%c";
	cPortID = cPortID - '0' + 'a';
#elif defined(hpux) || defined(_hpux) || defined(__hpux) //HP-UX
	szPortFileMask = "/dev/tty%cp0";
#elif defined(__unix) || defined(__unix__) // UNIX
	szPortFileMask = "/dev/tty0%c";
#elif defined(linux) || defined(__linux__) // Linux
	szPortFileMask = "/dev/ttyS%c";
#endif
	_snprintf(szBuffer, nBufferSize, szPortFileMask, cPortID);
}

int CComPort::GetBaudRateConst(SComPortParams::EBaudRate eRate)
{
	int nResult = 0;
	switch(eRate)
	{
		case SComPortParams::eBR110: nResult = BAUD_RATE_CONST(110); break;
		case SComPortParams::eBR300: nResult = BAUD_RATE_CONST(300); break;
		case SComPortParams::eBR600: nResult = BAUD_RATE_CONST(600); break;
		case SComPortParams::eBR1200: nResult = BAUD_RATE_CONST(1200); break;
		case SComPortParams::eBR2400: nResult = BAUD_RATE_CONST(2400); break;
		case SComPortParams::eBR4800: nResult = BAUD_RATE_CONST(4800); break;
		case SComPortParams::eBR9600: nResult = BAUD_RATE_CONST(9600); break;
		case SComPortParams::eBR19200: nResult = BAUD_RATE_CONST(19200); break;
		case SComPortParams::eBR38400: nResult = BAUD_RATE_CONST(38400); break;
		case SComPortParams::eBR57600: nResult = BAUD_RATE_CONST(57600); break;
		case SComPortParams::eBR115200: nResult = BAUD_RATE_CONST(115200); break;
	};
	return nResult;
}

void CComPort::SetError(int nErrorNum /*= 0*/, const char* szErrorMsg /*= NULL*/, ...)
{
	if (m_sLastError.m_pszLastError) delete [] m_sLastError.m_pszLastError, m_sLastError.m_pszLastError = NULL;
	if ((m_sLastError.m_nLastErrorNum = nErrorNum) && szErrorMsg && *szErrorMsg)
	{
		va_list vl;
		va_start(vl, szErrorMsg);
		char szBuffer[4096];
		size_t nErrMsgLen = _vsnprintf(szBuffer, sizeof(szBuffer) - 1, szErrorMsg, vl);
		szBuffer[nErrMsgLen] = '\0';
		m_sLastError.m_pszLastError = new char[nErrMsgLen + 1];
		strncpy(m_sLastError.m_pszLastError, szBuffer, nErrMsgLen);
	}
}

int CComPort::GetLastError(char* szBuffer, int nBufferSize)
{
	*szBuffer = '\0';
	if (m_sLastError.m_nLastErrorNum && m_sLastError.m_pszLastError) 
	{
		strncpy(szBuffer, m_sLastError.m_pszLastError, nBufferSize - 1);
		szBuffer[nBufferSize] = '\0';
	}
	return m_sLastError.m_nLastErrorNum;
}

TComPortHandle CComPort::OpenPort(int nComPortNum)
{
	TComPortHandle hResult;
	char szComPortFileName[MAX_PATH];
	GetComPortFileName(nComPortNum, szComPortFileName, sizeof(szComPortFileName));
#ifdef WINDOWS
	hResult = CreateFileA(szComPortFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	::SetCommMask(hResult, EV_RXCHAR); 

	if (hResult == INVALID_HANDLE_VALUE) hResult = NULL;
#else
	hResult = open(szComPortFileName, O_RDWR | O_NOCTTY | O_NDELAY);
        if (hResult) fcntl(hResult, F_SETFL, 0);
#endif // #ifdef WINDOWS

	return hResult;
}

void CComPort::ClosePort(TComPortHandle hPort)
{
	if (hPort)
	{
		// ignoring errors
#ifdef WINDOWS
		CloseHandle(hPort);
#else
		close(hPort);
#endif // #ifdef WINDOWS
	}
}

void CComPort::CheckComPorts(bool* abPortsFound, int nCheckPortsFrom /*= 1*/, int nCheckPortsTo /*= 20*/)
{
	if (nCheckPortsFrom < 1) nCheckPortsFrom = 1;
	memset(abPortsFound, 0, sizeof(abPortsFound));
	if (nCheckPortsTo > 20) nCheckPortsTo = 20;

	for (int i = nCheckPortsFrom; i <= nCheckPortsTo; i++)
	{
		TComPortHandle hPort = OpenPort(i);
		if (abPortsFound[i - 1] = hPort ? true : false) ClosePort(hPort);
	}
}

bool CComPort::SetParams(const SComPortParams& sParams)
{
	bool bResult = false;
#ifdef WINDOWS
	DCB dcb;
	bResult = GetCommState(m_hPort, &dcb) ? true : false;
	if (bResult) 
	{
		dcb.BaudRate = GetBaudRateConst(sParams.m_eBaudRate);      // set the baud rate
		dcb.ByteSize = 4 + sParams.m_eByteSize - SComPortParams::eBS4bits; // data size, xmit, and rcv
		dcb.fParity = TRUE;
		switch(sParams.m_eParityCheck)
		{
			case SComPortParams::ePC_Even: dcb.Parity = EVENPARITY; break;
			case SComPortParams::ePC_Odd: dcb.Parity = ODDPARITY; break;
			case SComPortParams::ePC_Mark: dcb.Parity = MARKPARITY; break;
			default: dcb.Parity = NOPARITY; dcb.fParity = FALSE; 
		}
		dcb.StopBits = sParams.m_bTwoStopBits ? TWOSTOPBITS : ONESTOPBIT;
		bResult = SetCommState(m_hPort, &dcb) ? true : false;
		if (bResult) SetError();
	}
	if (!bResult) SetError(CError::GetErrNo(), "Error getting or setting COM-port %d parameters\n%s", m_nPortNum, CError::GetErrStr());
#else
	//! temporary no error check for API calls

	// getting current settings
	termios sOptions;
	tcgetattr(m_hPort, &sOptions);
	// baud rate
	cfsetispeed(&sOptions, GetBaudRateConst(sParams.m_eBaudRate));
	cfsetospeed(&sOptions, GetBaudRateConst(sParams.m_eBaudRate));
	sOptions.c_cflag |= CLOCAL | CREAD; // do not change owner & ready to read
	// stop bits
	if (sParams.m_bTwoStopBits) sOptions.c_cflag |= CSTOPB; else sOptions.c_cflag &= ~CSTOPB;
	// byte size
	sOptions.c_cflag &= ~CSIZE;
        switch(sParams.m_eByteSize)
	{
                case SComPortParams::eBS4bits: break;
                case SComPortParams::eBS5bits: sOptions.c_cflag |= CS5; break;
                case SComPortParams::eBS6bits: sOptions.c_cflag |= CS6; break;
                case SComPortParams::eBS7bits: sOptions.c_cflag |= CS7; break;
		default: sOptions.c_cflag |= CS8;
	}
	// parity check mode
        if (sParams.m_eParityCheck != SComPortParams::ePC_Off)
	{
		sOptions.c_cflag |= PARENB;
                if (sParams.m_eParityCheck == SComPortParams::ePC_Odd) sOptions.c_cflag |= PARODD;
                else if (sParams.m_eParityCheck == SComPortParams::ePC_Mark) sOptions.c_iflag |= PARMRK;
		sOptions.c_iflag |= (INPCK | ISTRIP);
	}
	else 
	{
		sOptions.c_cflag &= ~PARENB; 
		sOptions.c_iflag &= ~(INPCK | ISTRIP);
	}
	// raw mode without echo
	sOptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	sOptions.c_oflag &= ~OPOST;
	// applying settings immediately
        tcsetattr(m_hPort, TCSANOW, &sOptions);
	SetError();
#endif
	return bResult;
}

CComPort* CComPort::Open(int nPortNum, const SComPortParams& sParams)
{
	std::auto_ptr<CComPort> pComPort(new CComPort);
	pComPort->m_hPort = OpenPort(nPortNum);
	if (pComPort->m_hPort) {
		if (!pComPort->SetParams(sParams)) pComPort.reset();
	}
//	else SetError(CError::GetErrNo(), "Error open COM-port #%d\n%s", nPortNum, CError::GetErrStr()); 

	return pComPort.release();
}

int CComPort::Read(void* pBuffer, int nBytesToRead, bool bWait /*= true*/)
{
	int nResult = -1;

#ifdef WINDOWS
	DWORD dwMask, dwFoo, dwSignal;
	if (m_sOverlappedRd.hEvent == INVALID_HANDLE_VALUE) m_sOverlappedRd.hEvent = ::CreateEvent(NULL, true, true, NULL);
	::WaitCommEvent(m_hPort, &dwMask, &m_sOverlappedRd);
	dwSignal = ::WaitForSingleObject(m_sOverlappedRd.hEvent, INFINITE);
	BOOL bResult = FALSE;
	if (dwSignal == WAIT_OBJECT_0 && ::GetOverlappedResult(m_hPort, &m_sOverlappedRd, &dwFoo, true) && (dwMask & EV_RXCHAR) != 0)
	{
		COMSTAT comstat;
		::ClearCommError(m_hPort, &dwFoo, &comstat);
		if (comstat.cbInQue)
			bResult = ::ReadFile(m_hPort, pBuffer, comstat.cbInQue, (LPDWORD)&nResult, &m_sOverlappedRd);
	}
	if (!bResult)
#else
	if ((nResult = read(m_hPort, pBuffer, nBytesToRead)) == -1)
#endif // #ifdef WINDOWS
            SetError(CError::GetErrNo(), "Error reading from COM%d port: %s", m_nPortNum, CError::GetErrStr().c_str());
	else SetError();
	
	return nResult;
}

int CComPort::Write(const void* pBuffer, int nBytesToWrite)
{
	int nResult = -1;

#ifdef WINDOWS
	DWORD dwTemp, dwSignal;
	if (m_sOverlappedWr.hEvent == INVALID_HANDLE_VALUE) m_sOverlappedWr.hEvent = ::CreateEvent(NULL, true, true, NULL);

	BOOL bResult = ::WriteFile(m_hPort, pBuffer, nBytesToWrite, (LPDWORD)&nResult, &m_sOverlappedWr);
	if (bResult)
	{
		dwSignal = ::WaitForSingleObject(m_sOverlappedWr.hEvent, INFINITE);
		if((dwSignal == WAIT_OBJECT_0) && (::GetOverlappedResult(m_hPort, &m_sOverlappedWr, &dwTemp, true))) bResult = TRUE;
		else bResult = FALSE;
	}
	if (!bResult)
#else
	if ((nResult = write(m_hPort, pBuffer, nBytesToWrite)) == -1)
#endif // #ifdef WINDOWS
            SetError(CError::GetErrNo(), "Error reading from COM%d port: %s", m_nPortNum, CError::GetErrStr().c_str());
	else SetError();

	return nResult;
}

int CComPort::ReadChar(bool bWait /*= true*/)
{
	int nResult;
	unsigned char ucChar;
	nResult = Read(&ucChar, 1);
	return (nResult > 0) ? ucChar : nResult;
}

int CComPort::WriteChar(char cChar)
{
	return Write(&cChar, 1);
}

int CComPort::Flush(EFlushMethod eFlushMethod /*= eFlushAllData*/)
{
	int nResult = 0;
	int nMask = 0;
#ifdef WINDOWS
	if (eFlushMethod | eFlushInData) nMask |= PURGE_RXCLEAR;
	if (eFlushMethod | eFlushOutData) nMask |= PURGE_TXCLEAR;
	nResult = PurgeComm(m_hPort, nMask) ? 0 : -1;
#else
	if (eFlushMethod == eFlushInData) nMask = TCIFLUSH;
	else if (eFlushMethod == eFlushOutData) nMask = TCOFLUSH;
	else if (eFlushMethod == eFlushAllData) nMask = TCIOFLUSH;
	tcflush(m_hPort, nMask);
#endif
        if (!nResult) SetError(CError::GetErrNo(), "Error flushing COM%d port: %s", m_nPortNum, CError::GetErrStr().c_str());
	else SetError();

	return nResult;
}


int CComPort::GetUnreadDataSize() const
{
	int nResult = 0;
#if defined (WINDOWS)
	COMSTAT cs;
	DWORD dwErrorsFoo;
	ClearCommError(m_hPort, &dwErrorsFoo, &cs);
	nResult = cs.cbInQue;
#else
	ioctl(m_hPort, FIONREAD, &nResult);
#endif
	return nResult;
}

void CComPort::BitCmd(EComPortBitsCmd eCmd)
{
#if defined(WINDOWS)
	int nCmd;
	switch(eCmd)
	{
		case eSETDTR: nCmd = SETDTR; break;
		case eCLRDTR: nCmd = CLRDTR; break;
	}
	EscapeCommFunction(m_hPort, eCmd);
#else
	int nBits;

	ioctl(m_hPort, TIOCMGET, &nBits);

	switch(eCmd)
	{
		case eSETDTR: nBits |= TIOCM_DTR; break;
		case eCLRDTR: nBits &= ~TIOCM_DTR; break;
	}
        ioctl(m_hPort, TIOCMSET, &nBits);
#endif
}
#pragma warning(pop)
