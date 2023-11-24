#pragma once

struct Point
{
	int x = 0;
	int y = 0;
};

struct Size
{
	int width = 0;
	int height = 0;
};

// Точка передаётся в локальных координатах.
inline bool IsPointInSize(Point p, Size size) noexcept
{
	return p.x >= 0 && p.y >= 0 && p.x < size.width && p.y < size.height;
}
