#include "DXScreenShooter.h"

#include <conio.h>
#include <chrono>
#include <string>
#include <iostream>

CDxScreenShooter::CDxScreenShooter() : m_surf(nullptr), m_pd3dDevice(nullptr)
{
	initSurface();
}

CDxScreenShooter::~CDxScreenShooter()
{
	// release the image surface 
	releaseAll();
}

void CDxScreenShooter::releaseAll()
{
	if (m_surf)
		m_surf->Release();
	if (m_pd3dDevice)
		m_pd3dDevice->Release();
}

void CDxScreenShooter::initSurface()
{
	releaseAll();

	// структкура с параметрами девайса
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS)); // обнулим
	d3dpp.BackBufferWidth = m_region.m_size.m_x; // указываем ширину и
	d3dpp.BackBufferHeight = m_region.m_size.m_y; // высоту области рендера
	d3dpp.BackBufferCount = 1;					// один внеэкранный буфер
	d3dpp.Windowed = TRUE;						// рендер в окне, а не в полный экран
	//d3dpp.Windowed = FALSE;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;	// формат выберет сам DX
	//d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;		// так лучше делать всегда
	d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;	// формат Z-буфера
	d3dpp.EnableAutoDepthStencil = FALSE;		// нам ненужен Z-буфер
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	// сначала проицициализируем сам DirectX 9
	IDirect3D9 * pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (pD3D == NULL)
		return;

	if (D3D_OK != pD3D->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE::D3DDEVTYPE_HAL,
		//D3DDEVTYPE::D3DDEVTYPE_REF,
		//D3DDEVTYPE::D3D_DRIVER_TYPE_HARDWARE,
		GetDesktopWindow(),						// HWND of desktop 
		//D3DCREATE_MULTITHREADED,
		//D3DCREATE_HARDWARE_VERTEXPROCESSING,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&m_pd3dDevice))
	{
		return;
	}


	// get display dimensions 
	// this will be the dimensions of the front buffer
	if (D3D_OK != m_pd3dDevice->GetDisplayMode(NULL, &m_displaymode))
	{
		return;
	}

	// create the image surface to store the front buffer image 
	// note that call to GetFrontBuffer will always convert format to A8R8G8B8 
	if (D3D_OK != m_pd3dDevice->CreateOffscreenPlainSurface(m_displaymode.Width,
		m_displaymode.Height,
		D3DFMT_A8R8G8B8,
		D3DPOOL_SCRATCH,
		//D3DPOOL_DEFAULT,
		//D3DPOOL_MANAGED,
		//D3DPOOL_SYSTEMMEM,
		&m_surf,
		NULL))
	{
		return;
	}

	// Нужно ли?
	pD3D->Release();
}

/*
HBITMAP CopySurfaceToBitmap(IDirect3DSurface9 *_pD3DSurface, BYTE *_pData, BITMAPINFO *_pHeader)
{
	HDC hScrDC, hMemDC;			// surface DC and memory D
	HBITMAP hBitmap, hOldBitmap;	// handles to deice-dependent bitmap
	int nX, nY;					// top-left of rectangle to gra
	int nWidth, nHeight;		// DIB width and heigh

	if (_pD3DSurface == NULL)
		return NULL;

	// create a DC for the surface and creat
	// a memory DC compatible to surface D
	_pD3DSurface->GetDC(&hScrDC);
	hMemDC = CreateCompatibleDC(hScrDC);

	D3DSURFACE_DESC desc;
	_pD3DSurface->GetDesc(&desc);

	// get points of rectangle to gra
	nX = 0;
	nY = 0;

	nWidth = desc.Width;
	nHeight = desc.Height;

	// create a bitmap compatible with the surface D
	hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight);

	// select new bitmap into memory D
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

	// bitblt surface DC to memory D
	BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, nX, nY, SRCCOPY);

	// select old bitmap back into memory DC and get handle t
	// bitmap of the surfac
	hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);

	// Copy the bitmap data into the provided BYTE buffe
	GetDIBits(hScrDC, hBitmap, 0, nHeight, _pData, _pHeader, DIB_RGB_COLORS);

	// clean u
	_pD3DSurface->ReleaseDC(hScrDC);
	DeleteDC(hMemDC);

	// return handle to the bitma
	return hBitmap;
}
/**/

// ScreenShot 
bool CDxScreenShooter::GetScreenShot(const CRectangle& _region, std::vector<char>& _outBuffer)
{
	if (m_region != _region)
	{
		m_region = _region;
		initSurface();			// Утечка?
	}

	// read the front buffer into the image surface 
	if (D3D_OK != m_pd3dDevice->GetFrontBufferData(0, m_surf))
	{
		std::cout << "Failed GetFrontBufferData()" << std::endl;
		m_surf->Release();
		return false;
	}

	//BYTE *pData = NULL; 
	//BITMAPINFO *pHeader = NULL;
	//HBITMAP bitmap = CopySurfaceToBitmap(m_surf, pData, pHeader);

	D3DLOCKED_RECT lr;
	ZeroMemory(&lr, sizeof(D3DLOCKED_RECT));

	HRESULT hr = m_surf->LockRect(&lr, 0, D3DLOCK_READONLY);
	if (FAILED(hr))
	{
		std::cout << "Failed LockRect()" << std::endl;
		return false;
	}
	
	//_outBuffer.resize(m_displaymode.Width * m_displaymode.Height * m_displaymode.Format);
	_outBuffer.resize(m_displaymode.Width * m_displaymode.Height * 4);
	if (lr.pBits)
	{
		//memcpy(_outBuffer.data(), lr.pBits, m_displaymode.Width * m_displaymode.Height * m_displaymode.Format);
		memcpy(_outBuffer.data(), lr.pBits, m_displaymode.Width * m_displaymode.Height * 4);
	}

	hr = m_surf->UnlockRect();
	if (FAILED(hr))
	{
		std::cout << "Cannot unlock rect!" << std::endl;
		return false;
	}

	// return status of save operation to caller 
	return true;
}