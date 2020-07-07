#ifndef __GDI_SCREENSHOOTER__H
#define __GDI_SCREENSHOOTER__H

#include <vector>

#include <Windows.h>

#include "types/geometry_types.h"

class CGDIScreenShooter
{
public:
	explicit CGDIScreenShooter(const CRectangle& _region);

	bool getScreenshot(const CRectangle& _region, 
									std::vector<char>& _outBuffer);

	void getBitMapInfoHeader(BITMAPINFOHEADER& _info);

private:
	void initBitmapInfoStruct();
	void reinitDC();

	CRectangle m_region;

	BITMAPINFO m_bmi;
	HDC m_desktop_hDC;
	HDC m_myHDC;
	HBITMAP m_hBMP;
};

#endif