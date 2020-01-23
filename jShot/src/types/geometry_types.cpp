#include "geometry_types.h"


bool Vector2::operator == (const Vector2& _other)
{
	return (m_x == _other.m_x) && (m_y == _other.m_y);
}

bool CRectangle::operator == (const CRectangle& _other)
{
	return (m_leftBottomCorner == _other.m_leftBottomCorner) && (m_size == _other.m_size);
}

bool CRectangle::operator != (const CRectangle& _other)
{
	return !(operator==(_other));
}