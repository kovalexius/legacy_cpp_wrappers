#ifndef JPEGCOMPRESS_H
#define JPEGCOMPRESS_H

#include <iostream>
#include <stdio.h>
//#include "LibJpeg.h"

class CJpegCompress
{
public:
	unsigned char* compressJpegToMem( const void *srcbuf, const unsigned long &size, const unsigned int &width, const unsigned int &height, const int &quality, unsigned long &bufSize );
	//void* compressJpegToFile( void *srcbuf, const unsigned long &size, const unsigned int &width, const unsigned int &height, const int &quality, unsigned long &bufSize );
	CJpegCompress();
	~CJpegCompress();
	void destroyBuffer( unsigned char *&buf );
private:
	//sJpeg_compress_struct jpCompressObj;
	CLibJpeg jpg;
};


#endif