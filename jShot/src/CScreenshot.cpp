#include "CScreenshot.h"

CScreenshot::CScreenshot( unsigned char *& buffer, BITMAPINFOHEADER &bih, const int &sx, const int &sy )
{
	lpBits = 0;
	this->sx = sx;
	this->sy = sy;

	InitBitmapInfoStruct();
	bih = bmi.bmiHeader;
	bih.biHeight = abs(bih.biHeight);

	lpBits = (LPBYTE) GlobalAlloc( GMEM_FIXED, bmi.bmiHeader.biSizeImage);
	buffer = lpBits;
}

CScreenshot::~CScreenshot()
{
	if(lpBits)
		GlobalFree(lpBits);
}

void CScreenshot::CreateJPGFile( LPTSTR pszFile, LPBYTE lpBits, DWORD size )
{
	HANDLE hf;                 // file handle
	BYTE *hp;                   // byte pointer
	DWORD dwTmp;
	hp = lpBits;
	// Create the .BMP file.
  hf = CreateFile( pszFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
  WriteFile(hf, (LPSTR) hp, (int) size, (LPDWORD) &dwTmp, NULL);
}

void CScreenshot::CreateBMPFile(LPTSTR pszFile, PBITMAPINFO pbi /*, HBITMAP hBMP, HDC hDC */)
{
	HANDLE hf;                 // file handle
	BITMAPFILEHEADER hdr;       // bitmap file-header
	PBITMAPINFOHEADER pbih;     // bitmap info-header
	DWORD dwTotal;              // total count of bytes
	DWORD cb;                   // incremental count of bytes
	BYTE *hp;                   // byte pointer
	DWORD dwTmp;
	
	pbih = (PBITMAPINFOHEADER) pbi;
	
	// Create the .BMP file.
	hf = CreateFile( pszFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	
	hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"
	// Compute the size of the entire file.
	hdr.bfSize = (DWORD) ( sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage );
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;
	
	// Compute the offset to the array of color indices.
	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof (RGBQUAD);
	
	// Copy the BITMAPFILEHEADER into the .BMP file.
	WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER), (LPDWORD) &dwTmp,  NULL);
	
	// Copy the BITMAPINFOHEADER and RGBQUAD array into the file.
	WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof (RGBQUAD), (LPDWORD) &dwTmp, NULL);
	
	// Copy the array of color indices into the .BMP file.
	dwTotal = cb = pbih->biSizeImage;
	hp = lpBits;
	WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL);
	
	// Close the .BMP file.
	CloseHandle(hf);
}

void CScreenshot::InitBitmapInfoStruct( )
{
  // Initialize the fields in the BITMAPINFO structure.
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = sx;
  bmi.bmiHeader.biHeight = -sy;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;

  // If the bitmap is not compressed, set the BI_RGB flag.
  bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = sx * sy * 3;
  bmi.bmiHeader.biClrImportant = 0;						// Set biClrImportant to 0, indicating that all of the device colors are important.
}

void CScreenshot::getScreenshot(  )
{
	HDC hDC = GetDC( GetDesktopWindow() );
	HDC MyHDC = CreateCompatibleDC(hDC);
	HBITMAP hBMP = CreateCompatibleBitmap(hDC, sx, sy);
	
	SelectObject(MyHDC, hBMP);
	LOGBRUSH MyBrush;
	MyBrush.lbStyle = BS_SOLID;
	MyBrush.lbColor = 0xFF0000;
	HBRUSH hBrush = CreateBrushIndirect(&MyBrush);
	RECT MyRect = {0, 0, sx, sy};
	FillRect(MyHDC, &MyRect, hBrush);

	BitBlt( MyHDC, 0, 0, sx, sy, hDC, 0, 0, SRCCOPY );
	
	GetDIBits( hDC, hBMP, 0, sy, lpBits, &bmi, DIB_RGB_COLORS );
	BGR2RGB( lpBits, bmi.bmiHeader.biSizeImage );

	DeleteObject( hBMP );
	DeleteObject( hBrush );
	DeleteDC( MyHDC );
}

void CScreenshot::BGR2RGB( unsigned char *buf, unsigned long &num )
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

void CScreenshot::CopyToRGB( unsigned char *in, unsigned char *out, unsigned long &num )
{
	unsigned int numPixels = num/3;
	for( unsigned int i = 0; i < numPixels; i++ )
	{
		out[ i*3 + 2 ] = in[ i*3 ];
	}
}