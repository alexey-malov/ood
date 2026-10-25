#pragma once

class Rectangle;
class Ellipse;
class GroupShape;

class IShapeVisitor
{
public:
	virtual void Visit(const Rectangle& rectangle) = 0;
	virtual void Visit(const Ellipse& ellipse) = 0;
	virtual void Visit(const GroupShape& group) = 0;

protected:
	~IShapeVisitor() = default;
};
