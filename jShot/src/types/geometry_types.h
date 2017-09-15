#ifndef __GEOMETRY_TYPES__H
#define __GEOMETRY_TYPES__H

extern const int g_width;
extern const int g_height;

struct Vector2
{
	int m_x;
	int m_y;
};

class CRectangle
{
public:
	bool operator == (const CRectangle& _other);

	Vector2 m_leftBottomCorner;
	int m_width;
	int m_height;
};

#endif