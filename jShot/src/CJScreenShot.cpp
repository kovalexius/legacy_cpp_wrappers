
#include "CJScreenShot.h"
#include "../../JThreadPool/src/CJThreadPool.h"

CJScreenShot::CJScreenShot(const int& _sx, 
						   const int& _sy, 
						   const unsigned int &_maxY, 
						   unsigned char **&result, 
						   unsigned int *&lenOutBuf, 
						   unsigned int *&beginY, 
						   unsigned int *&heights,
						   bool *&isNew, 
						   int &count) : m_sx(_sx),
										 m_sy(_sy),
										 m_maxY(_maxY),
										 m_numPeaces(_sy/_maxY + (_sy%_maxY > 0) ? 1 : 0)
{
	InitBitmapInfoStruct();

	
	m_bytesPerPxl = m_bmi.bmiHeader.biBitCount >> 3;
	this->result = new unsigned char*[numPeaces];
	this->inbuffer = new unsigned char*[numPeaces];

	this->lenOutBuf = new unsigned int[numPeaces];
	this->beginY = new unsigned int[numPeaces];
	this->heights = new unsigned int[numPeaces];
	this->isNew = new bool[numPeaces];

	this->hashSum = new long[numPeaces];
	this->insize = new unsigned long[numPeaces];

	unsigned long fullSize = bmi.bmiHeader.biSizeImage;
	unsigned long tmpSz = m_maxY * this->sx * bytesPerPxl;
	unsigned int curY = 0;
	for(  unsigned int i = 0; i < numPeaces; i++ )
	{
		if( fullSize < tmpSz )
			tmpSz = fullSize;
		this->beginY[i] = curY;
		this->heights[i] = tmpSz / ( this->sx * bytesPerPxl );
		this->insize[i] = tmpSz;
		this->inbuffer[i] = new unsigned char[insize[i]];

		curY += this->heights[i];
		fullSize -= tmpSz;
		this->result[i] = NULL;
	}

	isNew			= this->isNew;
	result		= this->result;
	lenOutBuf = this->lenOutBuf;
	beginY		= this->beginY;
	heights		= this->heights;
	count = numPeaces;

	hDC = GetDC( GetDesktopWindow() );
	MyHDC = CreateCompatibleDC( hDC );
	hBMP = CreateCompatibleBitmap( hDC, sx, sy );
	SelectObject( MyHDC, hBMP );
}

CJScreenShot::~CJScreenShot()
{
	destroyBuffers();
	delete [] result;
	delete [] lenOutBuf;
	delete [] beginY;
	delete [] heights;
	delete [] hashSum;
	delete [] insize;
	for( unsigned int i = 0; i < numPeaces; i++ )
		delete [] inbuffer[i];
	delete [] inbuffer;

	DeleteObject( hBMP );
	DeleteDC( MyHDC );
}

void CJScreenShot::destroyBuffers()
{
	for( unsigned int i = 0; i < m_numPeaces; i++ )
		m_jpcmpress.destroyBuffer( result[i] );
}

long CJScreenShot::getHash( const unsigned long &size, unsigned char * const &buf )
{
	long heshSum = 0;
	for ( unsigned long i = 0; i < size; i++ )
		heshSum += buf[i];
	return heshSum;
}

void CJScreenShot::InitBitmapInfoStruct( )
{
	// Initialize the fields in the BITMAPINFO structure.
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = m_sx;
	bmi.bmiHeader.biHeight = -m_sy;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;

	// If the bitmap is not compressed, set the BI_RGB flag.
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = m_sx * m_sy * 3;
	bmi.bmiHeader.biClrImportant = 0;						// Set biClrImportant to 0, indicating that all of the device colors are important.
}

/*
template<class T, void(T::*mem_fun)( PROCESS_PARAMS * const & )>
void* WINAPI thread_to_member_thunk( void* p )
{
	PROCESS_PARAMS *params = static_cast<PROCESS_PARAMS*>( ptr );
	( static_cast<T*>(params->this_ptr)->*mem_fun )( params );
	 return 3;
}
*/

static void* runFunction( void *ptr )
{
	PROCESS_PARAMS *params = static_cast<PROCESS_PARAMS*>( ptr );
	params->this_ptr->getScreenshot( params );
	return 0;
}

void CJScreenShot::getJShot( const int &quality )
{
	this->quality = quality;

	BitBlt( MyHDC, 0, 0, sx, sy, hDC, 0, 0, SRCCOPY );

	SYSTEM_INFO siSysInfo;
  GetSystemInfo(&siSysInfo);

	CJThreadPool pool( siSysInfo.dwNumberOfProcessors );
	PROCESS_PARAMS *params = new PROCESS_PARAMS[numPeaces];
	for( int i = 0; i < numPeaces; i++ )		// Загрузка заданий в пул потоков
	{
		params[i].i = i;
		params[i].this_ptr = this;
		
		void* (*pf) (void*);
		pf = runFunction;
		pool.AddShedule( pf,  0, (void*)&params[i] );
		//pf(&params[i]);
	}
	pool.Run();		// Запуск заданий
	
	delete [] params;
}

void CJScreenShot::getScreenshot( PROCESS_PARAMS * const &param )
{
	m_mutex.lock();
	GetDIBits(MyHDC, hBMP, m_sy - beginY[param->i] - heights[param->i], heights[param->i], inbuffer[param->i], &m_bmi, DIB_RGB_COLORS);
	m_mutex.unlock();

	BGR2RGB(inbuffer[param->i], insize[param->i]);
	
	long hash = getHash(insize[param->i], inbuffer[param->i]);
	if(hash != hashSum[param->i])
	{
		jpcmpress.destroyBuffer(result[param->i]);
		result[param->i] = jpcmpress.compressJpegToMem(inbuffer[param->i], insize[param->i], sx, heights[param->i], quality, (unsigned long&)(lenOutBuf[param->i]));
		hashSum[param->i] = hash;
		isNew[param->i] = true;
	}
	else
		isNew[param->i] = false;
}

void CJScreenShot::BGR2RGB( unsigned char *buf, unsigned long &num )
{
	unsigned int numPixels = num/3;
	for( unsigned int i = 0; i < numPixels; i++ )
	{
		unsigned int i1 = i*3;
		unsigned int i2 = i*3 + 2;
		unsigned char tmp = buf[ i1 ];
		buf[ i1 ] = buf[ i2 ];
		buf[ i2 ] = tmp;
	}
}