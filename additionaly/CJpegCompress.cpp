#include "CJpegCompress.h"
#include "myMgr.h"

#include <conio.h>



CJpegCompress::CJpegCompress()
{
	
}

//void* CJpegCompress::compressJpegToFile( void *srcbuf, const unsigned long &size, const unsigned int &width, const unsigned int &height, const int &quality, unsigned long &bufSize )
//{
//	sJpeg_compress_struct jpCompressObj;
//	// step 1
//	sJpeg_destination_mgr destmgr;
//	jpCompressObj.dest = &destmgr;
//
//	sJpeg_error_mgr jerr;
//	jpCompressObj.err = jpeg_std_error( &jerr );
//
//	CLibJpeg jpg;
//	jpg.jpeg_create_compress( &jpCompressObj );
//
//	// step 2
//	std::string filename = "123.jpg";
//	FILE * outfile;
//	if ((outfile = fopen(filename.c_str(), "wb")) == NULL) 
//	{
//	    fprintf(stderr, "can't open %s\n", filename);
//	    exit(1);
//	}
//	jpg.jpeg_stdio_dest( &jpCompressObj, outfile );
//
//	// step 3
//	jpCompressObj.image_width = width;
//	jpCompressObj.image_height = height;
//	jpCompressObj.input_components = 3;
//	jpCompressObj.in_color_space = JCS_RGB;
//	jpg.jpeg_set_defaults( &jpCompressObj );
//
//	jpg.jpeg_set_quality(&jpCompressObj, quality, TRUE );
//
//	// step 4
//	jpg.jpeg_start_compress(&jpCompressObj, TRUE);
//
//	// step 5
//	JSAMPROW row_pointer[1];	// pointer to a single row 
//	int row_stride;						// physical row width in buffer 
//	row_stride = width * 3;		// JSAMPLEs per row in image_buffer 
//	JSAMPROW image_buffer = (JSAMPROW) srcbuf;
//	
//	while ( jpCompressObj.next_scanline < jpCompressObj.image_height )
//	{
//		row_pointer[0] = &image_buffer[ jpCompressObj.next_scanline * row_stride ];
//		jpg.jpeg_write_scanlines(&jpCompressObj, row_pointer, 1);
//	}
//
//	// step 6
//	jpg.jpeg_finish_compress( &jpCompressObj );
//	
//	return (void *)(0);
//}

unsigned char* CJpegCompress::compressJpegToMem( const void *srcbuf, const unsigned long &size, const unsigned int &width, const unsigned int &height, const int &quality, unsigned long &bufSize )
{
	sJpeg_compress_struct jpCompressObj;
	// step 1
	sJpeg_destination_mgr destmgr;
	jpCompressObj.dest = &destmgr;

	sJpeg_error_mgr jerr;
	jpCompressObj.err = jpeg_std_error( &jerr );
	
	jpg.jpeg_create_compress( &jpCompressObj );

	// step 2
	unsigned char *outbuffer;
	jMyMgr::jpeg_mem_dst( &jpCompressObj );


	// step 3
	jpCompressObj.image_width = width;
	jpCompressObj.image_height = height;
	jpCompressObj.input_components = 3;
	jpCompressObj.in_color_space = JCS_RGB;
	jpg.jpeg_set_defaults( &jpCompressObj );

	jpg.jpeg_set_quality(&jpCompressObj, quality, TRUE);

	// step 4
	jpg.jpeg_start_compress(&jpCompressObj, TRUE);

	// step 5
	JSAMPROW row_pointer[1];	// pointer to a single row 
	int row_stride;						// physical row width in buffer 
	row_stride = width * 3;		// JSAMPLEs per row in image_buffer 
	JSAMPROW image_buffer = (JSAMPROW) srcbuf;
	
	while ( jpCompressObj.next_scanline < jpCompressObj.image_height )
	{
		int i = jpCompressObj.next_scanline * row_stride;
		row_pointer[0] = &image_buffer[ i ];
		jpg.jpeg_write_scanlines(&jpCompressObj, row_pointer, 1);
	}

	// step 6
	jpg.jpeg_finish_compress( &jpCompressObj );

	// step 6.5
	outbuffer = ( (jMyMgr::jmemdst_s *)jpCompressObj.dest )->data;
	bufSize = ( (jMyMgr::jmemdst_s *)jpCompressObj.dest )->isize;

	jpg.jpeg_destroy_compress( &jpCompressObj );
	return outbuffer;
	
}

CJpegCompress::~CJpegCompress()
{
	// step 7
	//jpg.jpeg_destroy_compress( &jpCompressObj );
}

void CJpegCompress::destroyBuffer( unsigned char *&buf )
{
	if(buf)
		free(buf);
	buf = 0;
}