#include <iostream>
#include <vector>

#include "DDrawScreenShooter.h"
#include "ddraw_decode_error.h"

bool CDDrawScreenShooter::GetScreenShot(const CRectangle& _region, std::vector<char>& _outbuffer)
{
	LPDIRECTDRAW lpDDraw;
	HRESULT hResult = DirectDrawCreate(NULL, &lpDDraw, NULL);
	if (hResult != DD_OK)
	{
		return false;
	}

	HWND desktopHwnd = GetDesktopWindow();
	hResult = lpDDraw->SetCooperativeLevel(desktopHwnd, DDSCL_NORMAL);
	if (hResult != DD_OK)
	{
		return false;
	}

	DDSURFACEDESC ddsd;
	DDSCAPS ddsc;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	ddsd.dwFlags = DDSD_CAPS;

	IDirectDrawSurface * lpPrimarySurface;
	hResult = lpDDraw->CreateSurface(&ddsd, &lpPrimarySurface, NULL);
	if (hResult != DD_OK)
	{
		std::cout << "Error: " << decodeCreateSurfaceError(hResult) << std::endl;
		return false;
	}

	/*
	HDC hDC;
	hResult = lpPrimarySurface->GetDC(&hDC);
	if (hResult != DD_OK)
	{
		return false;
	}
	*/
	RECT rectangle;
	rectangle.left = _region.getLeftBottom().m_x;
	rectangle.top = _region.getLeftBottom().m_y + _region.getSize().m_y;
	rectangle.bottom = _region.getLeftBottom().m_y;
	rectangle.right = _region.getLeftBottom().m_x + _region.getSize().m_x;
	//lpPrimarySurface->Blt(rectangle,);
}