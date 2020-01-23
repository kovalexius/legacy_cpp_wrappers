#ifndef __DX_SCREENSHOOTER__H
#define __DX_SCREENSHOOTER__H

#include <memory>

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

	bool GetScreenShot(const CRectangle& _region);
	
private:
	void initSurface();

	IDirect3DDevice9 * m_pd3dDevice;
	IDirect3DSurface9 * m_surf;

	CRectangle m_region;
};

#endif