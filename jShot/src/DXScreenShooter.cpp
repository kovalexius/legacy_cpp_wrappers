#include "DXScreenShooter.h"

#include <conio.h>
#include <chrono>
#include <string>
#include <iostream>

CDxScreenShooter::CDxScreenShooter()
{
	initSurface();
}

CDxScreenShooter::~CDxScreenShooter()
{
	// release the image surface 
	m_surf->Release();
}


void CDxScreenShooter::initSurface()
{
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
	std::shared_ptr<D3DDISPLAYMODE> mode(std::make_shared<D3DDISPLAYMODE>());
	if (D3D_OK != m_pd3dDevice->GetDisplayMode(NULL, mode.get()))
	{
		return;
	}

	// create the image surface to store the front buffer image 
	// note that call to GetFrontBuffer will always convert format to A8R8G8B8 
	if (D3D_OK != m_pd3dDevice->CreateOffscreenPlainSurface(mode->Width,
		mode->Height,
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
}

// ScreenShot 
bool CDxScreenShooter::GetScreenShot(const CRectangle& _region)
{
	if (m_region != _region)
	{
		m_region = _region;
		initSurface();			// Утечка?
	}

	uint64_t numIterations = 0;
	auto startTime = std::chrono::system_clock::now();
	for (numIterations = 0; _kbhit() == 0; numIterations++)
	{
		// read the front buffer into the image surface 
		if (D3D_OK != m_pd3dDevice->GetFrontBufferData(0, m_surf))
		{
			m_surf->Release();
			return false;
		}

		// write the entire surface to the requested file 
		std::string fileName(std::string("screenshotDx") + std::to_string(numIterations) + ".bmp");
		D3DXSaveSurfaceToFile(fileName.c_str(), D3DXIFF_BMP, m_surf, NULL, NULL);
	}
	auto endTime = std::chrono::system_clock::now();

	std::chrono::duration<double> diffTime = endTime - startTime;
	std::cout << "Interval: \'" << diffTime.count() << "\' numIterations: \'" << numIterations << "\'" << std::endl;

	// return status of save operation to caller 
	return true;
}