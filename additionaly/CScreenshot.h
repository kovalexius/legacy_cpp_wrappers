#ifndef CGETSCREENSHOT_H
#define CGETSCREENSHOT_H

#include <windows.h>

class CScreenshot
{
public:
	CScreenshot( unsigned char *&buffer, BITMAPINFOHEADER &bih , const int &x, const int &y );
	~CScreenshot();
	 void getScreenshot( );
private:
	void CreateBMPFile( LPTSTR pszFile, PBITMAPINFO pbi /*, HBITMAP hBMP, HDC hDC*/ );
	void CreateJPGFile( LPTSTR pszFile, LPBYTE lpBits, DWORD size );
	void CopyToRGB( unsigned char *in, unsigned char *out, unsigned long &num );
	void BGR2RGB( unsigned char *buf, unsigned long &num );
	void InitBitmapInfoStruct( );
	LPBYTE lpBits;              // memory pointer
	int sx;
	int sy;
	BITMAPINFO bmi;
};

#endif