#ifndef __GEOMETRY_TYPES__H
#define __GEOMETRY_TYPES__H


struct Vector2
{
public:
	explicit Vector2(const int _x, const int _y) : m_x(_x),
		m_y(_y)
	{}

	bool operator == (const Vector2& _other);

	int m_x;
	int m_y;
};

struct CRectangle
{
public:
	CRectangle() : m_leftBottomCorner(0,0),
		m_size(0,0),
		m_bpp(32)
	{}

	explicit CRectangle(const Vector2& _leftBottomCorner, 
						const Vector2& _size,
						short _bitPerPixel) :
																			m_leftBottomCorner(_leftBottomCorner),
																			m_size(_size),
																			m_bpp(_bitPerPixel)
	{}

	bool operator == (const CRectangle& _other);
	bool operator != (const CRectangle& _other);

	Vector2 m_leftBottomCorner;
	Vector2 m_size;

	// The number of bytes-per-pixel.
	short	m_bytespp;
};

#endif