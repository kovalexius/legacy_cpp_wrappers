#ifndef CJSCREENSHOT_H
#define CJSCREENSHOT_H

#include <mutex>
#include <vector>

#include <Windows.h>
#include <d3d9.h>

#include "CJpegCompress.h"

class CJScreenShot;
struct PROCESS_PARAMS
{
	CJScreenShot *this_ptr;
	int i;
};

class CJScreenShot
{
public:
	CJScreenShot(const int &sx, 
				 const int &sy, 
				 const unsigned int &maxY, 
				 unsigned char **&outbuffer,
				 unsigned int *&lensBuf,
				 unsigned int *&y,
				 unsigned int *&h,
				 bool *&isNew,
				 int &count );
	~CJScreenShot();
	void getJShot( const int &quality );
	void destroyBuffers();
	void getScreenshot(PROCESS_PARAMS * const &param);

private:
	void InitBitmapInfoStruct( );
	void BGR2RGB( unsigned char *buf, unsigned long &num );
	long getHash( const unsigned long &size, unsigned char * const &buf );

	CJpegCompress m_jpcmpress;
	int m_sx;
	int m_sy;
	unsigned int m_maxY;
	unsigned int m_numPeaces;

	BITMAPINFO m_bmi;
	HDC hDC;
	HDC MyHDC;
	HBITMAP hBMP;
	
	std::mutex m_mutex;

	short m_bytesPerPxl;
	int m_quality;

	unsigned long *insize;
	unsigned char **inbuffer;
	unsigned char **result;
	unsigned int *lenOutBuf;
	unsigned int *beginY;
	unsigned int *heights;
	bool *isNew;
	int count;
	long *hashSum;
};

#endif