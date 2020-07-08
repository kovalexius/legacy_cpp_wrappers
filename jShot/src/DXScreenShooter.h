#ifndef __DX_SCREENSHOOTER__H
#define __DX_SCREENSHOOTER__H

#include <memory>
#include <vector>

#include "types/geometry_types.h"

class CDxScreenShooterImpl;

class CDxScreenShooter
{
public:
	CDxScreenShooter();
	bool GetScreenShot(const CRectangle& _region, std::vector<char>& _outBuffer);
private:
	std::shared_ptr<CDxScreenShooterImpl> m_shooter;
};

#endif