#ifndef __GDI_SCREENSHOOTER__H
#define __GDI_SCREENSHOOTER__H

#include <vector>

#include <Windows.h>

#include "types/geometry_types.h"

class CGDIScreenShooter
{
public:
	CGDIScreenShooter();

	std::vector<char> getScreenshot(const CRectangle& _region);

private:
	void initBitmapInfoStruct();
	void reinitDC();

	CRectangle m_regin;

	BITMAPINFO m_bmi;
	HDC m_hDC;
	HDC m_myHDC;
	HBITMAP m_hBMP;
};

#endif