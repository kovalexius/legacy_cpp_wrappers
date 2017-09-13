// testJShot.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <tchar.h>

#include <iostream>
#include <string>
#include <conio.h>

#include <time.h> 
#include "jDll.h"
#include "CJThreadPool.h"


#define Y_MAX_HEIGHT 1080
#define QUALITY 30

static void* runFunction( void *ptr )
{
	unsigned int *lensBuf;
	unsigned int *y;
	unsigned int *h;
	bool *isNew;
	int width;
	int height;
	int count;
	void **buf;

	buf = getJShot( QUALITY, Y_MAX_HEIGHT, lensBuf, y, h, isNew, count, width, height );

	std::string name("screenshotik");
	std::string filename;
	char chbuf[7];
	FILE * outfile;

	for (int i = 0; i < count; i++)
	{
		if( isNew[i] )
		{
			_itoa( i, chbuf, 10 );
			filename = name + "_" + std::string( chbuf ) + std::string( ".jpg" );
			if ((outfile = fopen(filename.c_str(), "wb")) == NULL) 
			{
			    fprintf(stderr, "can't open %s\n", filename);
					_getch();  
			    exit(1);
			}
			fwrite( buf[i], 1, lensBuf[i], outfile );
			fclose( outfile );
		}
		printf( "%d ", isNew[i] );
	}
	printf( "\n" );

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	_getch();

	int numOfThrd = 8;
	CJThreadPool pool( numOfThrd );
	time_t beg;
	time( &beg );
	for ( int j=0; j < 1; j++ )
	{
		pool.AddShedule( &runFunction, 0, 0 );
	}
	pool.Run();
	time_t end;
	time( &end );
	double seconds = difftime( end, beg );
	printf ("interval = %.f\n", seconds );
	_getch();
	
	return 0;
}

