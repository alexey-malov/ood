#pragma once
#include "ishape.h"
#include "shape_collection.h"
#include <memory>
#include <ranges>
#include <vector>

class Shape : public IShape
{
public:
	explicit Shape(Rect bounds)
		: m_bounds(bounds)
	{
	}

	Rect GetBounds() const override
	{
		return m_bounds;
	}

	void MoveBy(Vector2 offset) override
	{
		const auto oldBounds = m_bounds;
		m_bounds.x += offset.x;
		m_bounds.y += offset.y;
		if (m_bounds != oldBounds)
		{
			m_changeSignal();
		}
	}

	ScopedConnection DoOnChange(ChangeSignal::slot_type slot) override
	{
		return m_changeSignal.connect(std::move(slot));
	}

private:
	Rect m_bounds;
	ChangeSignal m_changeSignal;
};

class Rectangle : public Shape
{
public:
	explicit Rectangle(Rect bounds)
		: Shape(bounds)
	{
	}

	bool HitTest(Point2 point) const override
	{
		const auto bounds = GetBounds();
		return point.x >= bounds.x && point.x <= bounds.x + bounds.width && point.y >= bounds.y && point.y <= bounds.y + bounds.height;
	}

	void Accept(IShapeVisitor& visitor) const override
	{
		visitor.Visit(*this);
	}
};

class Ellipse : public Shape
{
public:
	explicit Ellipse(Rect bounds)
		: Shape(bounds)
	{
	}

	bool HitTest(Point2 point) const override
	{
		const auto bounds = GetBounds();
		const float centerX = bounds.x + bounds.width / 2.0f;
		const float centerY = bounds.y + bounds.height / 2.0f;
		const float radiusX = bounds.width / 2.0f;
		const float radiusY = bounds.height / 2.0f;
		const float normalizedX = (point.x - centerX) / radiusX;
		const float normalizedY = (point.y - centerY) / radiusY;
		return (normalizedX * normalizedX + normalizedY * normalizedY) <= 1.0f;
	}

	void Accept(IShapeVisitor& visitor) const override
	{
		visitor.Visit(*this);
	}
};

class Drawing
{
public:
    using ChangeSignal = IShape::ChangeSignal;
    Drawing() = default;
	// Запрещаем копирование и перемещение объекта Drawing, чтобы избежать проблем с управлением ресурсами и сигналами.
    Drawing(const Drawing&) = delete;
    Drawing& operator=(const Drawing&) = delete;
    Drawing(Drawing&&) = delete;
    Drawing& operator=(Drawing&&) = delete;

	// Откладывает уведомления об изменениях до завершения самой внешней операции
	// Если действие вызывает исключение
    template<typename F>
    void WithDeferredNotifications(F&& action)
    {
        m_shapes.WithDeferredNotifications(std::forward<F>(action));
    }

    void AddShape(std::unique_ptr<IShape> shape)
	{
		m_shapes.AddShape(std::move(shape));
	}
    void InsertShape(size_t index, std::unique_ptr<IShape> shape)
    {
        m_shapes.InsertShape(index, std::move(shape));
    }
    [[nodiscard]] std::unique_ptr<IShape> ExtractShape(size_t index)
    {
        return m_shapes.ExtractShape(index);
    }
    size_t GetShapeCount() const { return m_shapes.GetShapeCount(); }
    IShape& GetShape(size_t index) { return m_shapes.GetShape(index); }
    const IShape& GetShape(size_t index) const { return m_shapes.GetShape(index); }
    IShape* FindShapeAt(Point2 point)
	{
		return m_shapes.FindShapeAt(point);
	}
    [[nodiscard]] ScopedConnection DoOnChange(ChangeSignal::slot_type slot)
    {
        return m_shapes.DoOnChange(std::move(slot));
    }
private:
    ShapeCollection m_shapes;
};
