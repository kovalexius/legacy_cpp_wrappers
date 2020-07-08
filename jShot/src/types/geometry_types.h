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
		m_bytesPerPixel(4),
		m_bitsPerPixel(m_bytesPerPixel*8)
	{}

	explicit CRectangle(const Vector2& _leftBottomCorner, 
						const Vector2& _size,
						short _bytesPerPixel) :
																			m_leftBottomCorner(_leftBottomCorner),
																			m_size(_size),
																			m_bytesPerPixel(_bytesPerPixel),
																			m_bitsPerPixel(_bytesPerPixel*8)
	{}

	bool operator == (const CRectangle& _other);
	bool operator != (const CRectangle& _other);

	Vector2& getSize()
	{
		return m_size;
	}
	const Vector2& getSize() const
	{
		return m_size;
	}

	short getBytesPerPixel() const
	{
		return m_bytesPerPixel;
	}

	short getBitsPerPixel() const
	{
		return m_bitsPerPixel;
	}

	Vector2& getLeftBottom()
	{
		return m_leftBottomCorner;
	}

	const Vector2& getLeftBottom() const
	{
		return m_leftBottomCorner;
	}

private:
	Vector2 m_leftBottomCorner;
	Vector2 m_size;

	// The number of bytes-per-pixel.
	short	m_bytesPerPixel;
	// The number of bits-per-pixel.
	short	m_bitsPerPixel;
};

#endif