#ifndef __DX_SCREENSHOOTER__H
#define __DX_SCREENSHOOTER__H

#include <memory>
#include <vector>

#include <Windows.h>
#include <D3D9Types.h>
#include <D3D9.h>
#include <d3dx9tex.h>

#include "types/geometry_types.h"

class CDxScreenShooter
{
public:
	CDxScreenShooter();
	virtual ~CDxScreenShooter();

	bool GetScreenShot(const CRectangle& _region, std::vector<char>& _outBuffer);
	
private:
	void releaseAll();
	void initSurface();

	IDirect3DDevice9 * m_pd3dDevice;
	IDirect3DSurface9 * m_surf;
	D3DDISPLAYMODE m_displaymode;

	CRectangle m_region;
};

#endif