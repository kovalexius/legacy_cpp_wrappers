#pragma once

#if defined(_WIN32) || defined(_WIN64) || defined(WINDOWS)
#define TComPortHandle void*
#define BAUD_RATE_CONST(nBaudRate) CBR_##nBaudRate

#ifndef WINDOWS
#define WINDOWS
#include <Windows.h>
#endif // #ifndef WINDOWS

#else 

#define TComPortHandle int
#define BAUD_RATE_CONST(nBaudRate) B##nBaudRate

#endif

// cross-platform compatible COM port params
// program flow control (xon/xof/etc.)not supported yet
struct SComPortParams
{
	enum EMode {
		eModeRead = 1,
		eModeWrite = 2,
		eModeReadWrite = 3
	};
	// only compatible between OS'es bit rates
	enum EBaudRate {
		eBR110 = 110, eBR300 = 300, eBR600 = 600, eBR1200 = 1200, eBR2400 = 2400, eBR4800 = 4800, eBR9600 = 9600, eBR19200 = 19200, eBR38400 = 38400, eBR57600 = 57600, eBR115200 = 115200
	};
	enum EParityCheckScheme {
		ePC_Off, ePC_Even, ePC_Odd, ePC_Mark
	};
	enum EByteSize {
		eBS4bits = 4, eBS5bits = 5, eBS6bits = 6, eBS7bits = 7, eBS8bits = 8
	};
	
	EMode				m_eMode;
	EBaudRate			m_eBaudRate;
	EByteSize			m_eByteSize;
	EParityCheckScheme	m_eParityCheck;
	bool				m_bTwoStopBits;

	SComPortParams(EMode eMode = eModeReadWrite, EBaudRate eBaudRate = eBR9600, EByteSize eByteSize = eBS8bits, EParityCheckScheme eParityCheck = ePC_Off, bool bTwoStopBits = false) 
	 :	m_eMode(eMode), m_eBaudRate(eBaudRate), m_eByteSize(eByteSize), m_eParityCheck(eParityCheck), m_bTwoStopBits(bTwoStopBits)
	{ }
};

class CComPort
{
public:
	~CComPort(void);

	// check COM ports range for availability, sets boolean array as result
	static void CheckComPorts(bool* abPortsFound, int nCheckPortsFrom = 1, int nCheckPortsTo = 20);

	// Create object and open port
	static CComPort* Open(int nPortNum, const SComPortParams& sParams);
	bool SetParams(const SComPortParams& sParams);

	// Read data from COM-port to buffer, returns number of read byte.
	//! no waiting not implemented yet
	int Read(void* pBuffer, int nBytesToRead, bool bWait = true);
	// Write data from buffer to COM-port
        int Write(const void* pBuffer, int nBytesToWrite);
	// Read one byte from COM-port, returns byte or -1 if error occurs
	int ReadChar(bool bWait = true);
	// Read one byte from COM-port, returns 0 on success or -1 if error occurs
	int WriteChar(char cChar);
	// Get result for last port operation. If need to get result of static functions use CError::GetErrNo() and CError::GetErrStr()
	int GetLastError(char* szBuffer, int nBufferSize);
	
	enum EFlushMethod {
		eFlushInData = 1,
		eFlushOutData = 2,
		eFlushAllData = 3
	};
	// Clear port data. Returns 0 on success or -1 on error
	int Flush(EFlushMethod eflushMethod = eFlushAllData);

	int GetUnreadDataSize() const;

	enum EComPortBitsCmd {
		eSETDTR, eCLRDTR
	};
	void BitCmd(EComPortBitsCmd eCmd);

	inline void SetPortParams(const SComPortParams& sParams) {
		SetPortParams(m_hPort, sParams);
	}

protected:
	CComPort();

	// Convert com port number to OS-specific file name
	static void GetComPortFileName(int nComPortNum, char* szBuffer, int nBufferSize);
	// Convert enum to OS-specific constant value
	static int GetBaudRateConst(SComPortParams::EBaudRate eRate);
	// Open COM-port by number
	static TComPortHandle OpenPort(int nComPortNum);
	// Close COM-port by hanlde/descriptor
	static void ClosePort(TComPortHandle hPort);
	// Set COM-port params by port handle/descriptor
	static void SetPortParams(TComPortHandle hPort, const SComPortParams& sParams);

	void SetError(int nErrorNum = 0, const char* szErrorMsg = 0, ...);

private:
	// Set COM-port handle/descriptor
	int				m_nPortNum;
	TComPortHandle	m_hPort;

#if defined(_WIN32) || defined(_WIN64) || defined(WINDOWS)
	OVERLAPPED m_sOverlappedRd;
	OVERLAPPED m_sOverlappedWr;
#endif

	struct SLastError
	{
		char* m_pszLastError;
		int m_nLastErrorNum;

		SLastError() : m_pszLastError(0), m_nLastErrorNum(0) {}
		~SLastError() {
			if (m_pszLastError) delete [] m_pszLastError, m_pszLastError = 0;
		}
	};
	SLastError m_sLastError;

};
