#include "GDIScreenShooter.h"



CGDIScreenShooter::CGDIScreenShooter(const CRectangle& _region) : 
																m_region(_region)
{
	initBitmapInfoStruct();
	reinitDC();
}

void CGDIScreenShooter::getBitMapInfoHeader(BITMAPINFOHEADER& _info)
{
	_info = m_bmi.bmiHeader;
}

bool CGDIScreenShooter::getScreenshot(const CRectangle& _region,
	std::vector<char>& _outBuffer)
{
	if (m_region != _region)
	{
		m_region = _region;
		initBitmapInfoStruct();
		reinitDC();
	}

	if (!BitBlt(m_myHDC, 
				0, 0, 
				_region.m_size.m_x, _region.m_size.m_y, 
				m_desktop_hDC, 
				_region.m_leftBottomCorner.m_x, _region.m_leftBottomCorner.m_y, 
				SRCCOPY))
    {
        return false;
    }

	_outBuffer.resize(m_bmi.bmiHeader.biSizeImage);
	auto ret = GetDIBits(m_myHDC, m_hBMP, 0, _region.m_size.m_y, _outBuffer.data(), &m_bmi, DIB_RGB_COLORS);

	if (ret)
	{
		return true;
	}
	return false;
}

void CGDIScreenShooter::reinitDC()
{
	m_desktop_hDC = GetDC(GetDesktopWindow());
	m_myHDC = CreateCompatibleDC(m_desktop_hDC);
	m_hBMP = CreateCompatibleBitmap(m_desktop_hDC, m_region.m_size.m_x, m_region.m_size.m_y);
	SelectObject(m_myHDC, m_hBMP);
}

void CGDIScreenShooter::initBitmapInfoStruct()
{
	// Initialize the fields in the BITMAPINFO structure.

	m_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_bmi.bmiHeader.biWidth = m_region.m_size.m_x;
	m_bmi.bmiHeader.biHeight = -m_region.m_size.m_y;
	m_bmi.bmiHeader.biPlanes = 1;
	m_bmi.bmiHeader.biBitCount = m_region.m_bpp;

	// If the bitmap is not compressed, set the BI_RGB flag.
	m_bmi.bmiHeader.biCompression = BI_RGB;
	m_bmi.bmiHeader.biSizeImage = m_region.m_size.m_x * m_region.m_size.m_y * (m_region.m_bpp / 8) + sizeof(m_bmi);
	m_bmi.bmiHeader.biClrImportant = 0;				// Set biClrImportant to 0, indicating that all of the device colors are important.

	m_bmi.bmiHeader.biXPelsPerMeter = 0;
	m_bmi.bmiHeader.biYPelsPerMeter = 0;
}