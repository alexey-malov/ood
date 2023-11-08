#include <cstdint>
#include "CommonTypes.h"
#include "Canvas.h"
#include "version1.h"

#include <optional>
#include <memory>
#include <limits>
#include <functional>

using std::optional;

class IDrawable
{
public:
	virtual void Draw(ICanvas & canvas) const = 0;

	virtual ~IDrawable() = default;
};

class IStyle
{
public:
	virtual optional<bool> IsEnabled()const = 0;
	virtual void Enable(bool enable) = 0;

	virtual optional<RGBAColor> GetColor()const = 0;
	virtual void SetColor(RGBAColor color) = 0;

	virtual ~IStyle() = default;
};

class IGroupShape;

class IShape : public IDrawable
{
public:
	virtual RectD GetFrame() = 0;
	virtual void SetFrame(const RectD & rect) = 0;

	virtual IStyle & GetOutlineStyle() = 0;
	virtual const IStyle & GetOutlineStyle()const = 0;
	
	virtual IStyle & GetFillStyle() = 0;
	virtual const IStyle & GetFillStyle()const = 0;

	virtual std::shared_ptr<IGroupShape> GetGroup() = 0;
	virtual std::shared_ptr<const IGroupShape> GetGroup() const = 0;

	virtual ~IShape() = default;
};

class IShapes
{
public:
	virtual size_t GetShapesCount()const = 0;
	virtual void InsertShape(const std::shared_ptr<IShape> & shape, size_t position = std::numeric_limits<size_t>::max()) = 0;
	virtual std::shared_ptr<IShape> GetShapeAtIndex(size_t index) = 0;
	virtual void RemoveShapeAtIndex(size_t index) = 0;

	virtual ~IShapes() = default;
};

class IGroupShape : public IShape, public IShapes
{
public:
	virtual ~IGroupShape() = default;
};

typedef std::function<void(ICanvas & canvas, const IShape & shape)> DrawingStrategy;

class SimpleShape : public IShape
{
public:
	SimpleShape(const DrawingStrategy & drawingStrategy)
	{
		(void)&drawingStrategy;
	}
};

class GroupShape : public IGroupShape
{

};

class ISlide : public IDrawable
{
public:
	virtual double GetWidth()const = 0;
	virtual double GetHeight()const = 0;

	virtual IShapes & GetShapes()const = 0;

	virtual ~ISlide() = default;
};

class Slide : public ISlide
{
public:

};


int main()
{

}
