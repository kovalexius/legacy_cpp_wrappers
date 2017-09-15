#include "jDll.h"

#include "CScreenshot.h"
#include "CJpegCompress.h"


static void getScreenParam( int &x, int &y )
{
	x = GetSystemMetrics(SM_CXSCREEN);
	y = GetSystemMetrics(SM_CYSCREEN);
}

CJShot *jshot = NULL;
CScreenshot *scrn = NULL;
int sx = 0;
int sy = 0;
BITMAPINFOHEADER bih;
unsigned char *scrBuffer;
unsigned char **result;
unsigned int maxH = 0;
unsigned int *lenOutbuf;
unsigned int *beginY;
unsigned int *heights;
bool *isNew;
int numPeaces;


void** getJShot(int quality, 
				int maxHeight,
				unsigned int *&lensBuf, 
				unsigned int *&y, 
				unsigned int *&h, 
				bool *&isNew, 
				int &count, 
				int &width, 
				int &height )
{
	getScreenParam( (int&)width, (int&)height );
	
	if( width != ::sx || height != ::sy || ::maxH != (unsigned int)maxHeight )
	{
		::sx = width;
		::sy = height;
		::maxH = (unsigned int)maxHeight;

		if( scrn )
			delete scrn;
		scrn = new CScreenshot( scrBuffer, bih, sx, sy );
		
		if( jshot )
			delete jshot;
		jshot = new CJShot( ::maxH, quality, ::bih, ::scrBuffer, ::result, ::lenOutbuf, ::beginY, ::heights, ::isNew, ::numPeaces );
	}
	
	if( scrn )
		scrn->getScreenshot( );
	
	if( jshot )
		jshot->getJShot( quality );

	lensBuf = ::lenOutbuf;
	y = ::beginY;
	h = ::heights;
	isNew = ::isNew;
	count = ::numPeaces;
	
	return (void **)::result;
}

bool destroyJShot( void )
{
	delete scrn;
	delete jshot;

	return true;
}

void destroyBuffers( void )
{
	jshot->destroyBuffers();
}

int printHelloWorld( int time )
{
	time = 661234;
	printf("Hello World blzzzzd!!!!");
	return time;
}