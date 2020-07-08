#ifndef __DDRAW_SCREENSHOOTER__H
#define __DDRAW_SCREENSHOOTER__H

#include <ddraw.h>

#include "types/geometry_types.h"

class CDDrawScreenShooter
{
public:
	bool GetScreenShot(const CRectangle& _region, std::vector<char>& _outbuffer);
};

#endif