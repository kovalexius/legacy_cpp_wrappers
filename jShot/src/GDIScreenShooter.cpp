#include "GDIScreenShooter.h"



CGDIScreenShooter::CGDIScreenShooter()
{
	reinitDC()

	initBitmapInfoStruct();
}

std::vector<char> CGDIScreenShooter::getScreenshot(const CRectangle& _region)
{
	std::vector<char> outData(m_bmi.bmiHeader.biSizeImage);
	BitBlt(m_myHDC, 0, 0, _region.m_width, _region.m_height, m_hDC, _region.m_leftBottomCorner.m_x, _region.m_leftBottomCorner.m_y, SRCCOPY);
	auto ret = GetDIBits(m_myHDC, m_hBMP, _region.m_leftBottomCorner.m_x, _region.m_leftBottomCorner.m_y, outData.data(), &m_bmi, DIB_RGB_COLORS);

	if (ret)
	{
		return outData;
	}
	return std::vector<char>();
}

void CGDIScreenShooter::reinitDC()
{
	m_hDC = GetDC(GetDesktopWindow());
	m_myHDC = CreateCompatibleDC(m_hDC);
	HBITMAP hBMP = CreateCompatibleBitmap(m_hDC, m_regin.m_width, m_regin.m_height);
	SelectObject(m_myHDC, hBMP);
}

void CGDIScreenShooter::initBitmapInfoStruct()
{
	// Initialize the fields in the BITMAPINFO structure.
	m_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_bmi.bmiHeader.biWidth = g_width;
	m_bmi.bmiHeader.biHeight = -g_height;
	m_bmi.bmiHeader.biPlanes = 1;
	m_bmi.bmiHeader.biBitCount = 24;

	// If the bitmap is not compressed, set the BI_RGB flag.
	m_bmi.bmiHeader.biCompression = BI_RGB;
	m_bmi.bmiHeader.biSizeImage = g_width * g_height * 3;
	m_bmi.bmiHeader.biClrImportant = 0;				// Set biClrImportant to 0, indicating that all of the device colors are important.
}