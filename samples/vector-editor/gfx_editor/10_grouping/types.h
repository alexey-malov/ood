#pragma once

struct Rect
{
	float x = 0.0f;
	float y = 0.0f;
	float width = 0.0f;
	float height = 0.0f;

	friend bool operator==(const Rect&, const Rect&) = default;
};

struct Point2
{
	float x = 0.0f;
	float y = 0.0f;
};

struct Vector2
{
	float x = 0.0f;
	float y = 0.0f;
};

inline Vector2 operator-(Point2 lhs, Point2 rhs)
{
	return { lhs.x - rhs.x, lhs.y - rhs.y };
}
