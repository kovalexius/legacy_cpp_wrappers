#ifndef __DDRAW_SCREENSHOOTER__H
#define __DDRAW_SCREENSHOOTER__H

#include <ddraw.h>

#include "types/geometry_types.h"

class CDDrawScreenShooter
{
public:
	void GetScreenShot(const CRectangle& _region);
};

#endif