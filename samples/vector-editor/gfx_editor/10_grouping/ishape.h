#pragma once
#include "ishapevisitor.h"
#include "signals.h"
#include "types.h"

class IShape
{
public:
	using ChangeSignal = boost::signals2::signal<void()>;

	virtual ~IShape() = default;

	virtual Rect GetBounds() const = 0;
	virtual bool HitTest(Point2 point) const = 0;
	virtual void MoveBy(Vector2 offset) = 0;
	virtual void Accept(IShapeVisitor& visitor) const = 0;

	[[nodiscard]] virtual ScopedConnection DoOnChange(ChangeSignal::slot_type slot) = 0;
};
