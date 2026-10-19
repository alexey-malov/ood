#pragma once
#include <boost/signals2/signal.hpp>
#include <memory>
#include <ranges>
#include <vector>

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

Vector2 operator-(Point2 lhs, Point2 rhs)
{
	return { lhs.x - rhs.x, lhs.y - rhs.y };
}

class Rectangle;
class Ellipse;

class IShapeVisitor
{
public:
	virtual void Visit(const Rectangle& rectangle) = 0;
	virtual void Visit(const Ellipse& ellipse) = 0;

protected:
	~IShapeVisitor() = default;
};

using ScopedConnection = boost::signals2::scoped_connection;

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
	using ChangeSignal = boost::signals2::signal<void()>;

	Drawing() = default;

	// «апрещаем копирование и перемещение объекта Drawing, чтобы избежать проблем с управлением ресурсами и сигналами.
	Drawing(const Drawing&) = delete;
	Drawing& operator=(const Drawing&) = delete;
	Drawing(Drawing&&) = delete;
	Drawing& operator=(Drawing&&) = delete;

	void AddShape(std::unique_ptr<IShape> shape)
	{
		InsertShape(m_shapes.size(), std::move(shape));
	}

	void InsertShape(
		size_t index,
		std::unique_ptr<IShape> shape)
	{
		if (!shape)
		{
			throw std::invalid_argument("Shape cannot be null");
		}

		if (index > m_shapes.size())
		{
			throw std::out_of_range("Invalid insertion index");
		}

		auto connection = shape->DoOnChange([this] {
			m_changeSignal();
		});

		m_shapes.insert(
			m_shapes.begin() + index,
			ShapeEntry{
				std::move(shape),
				std::move(connection) });

		m_changeSignal();
	}

	[[nodiscard]] std::unique_ptr<IShape> ExtractShape(size_t index)
	{
		if (index >= m_shapes.size())
		{
			throw std::out_of_range("Invalid extraction index");
		}

		auto it = m_shapes.begin() + index;

		auto shape = std::move(it->shape);

		m_shapes.erase(it);

		m_changeSignal();

		return shape;
	}

	size_t GetShapeCount() const { return m_shapes.size(); }

	const IShape& GetShape(size_t index) const { return *m_shapes.at(index).shape; }
	IShape& GetShape(size_t index) { return *m_shapes.at(index).shape; }

	IShape* FindShapeAt(Point2 point)
	{
		for (const auto& shape : m_shapes
				| std::views::reverse
				| std::views::transform([](const auto& entry) { return entry.shape.get(); }))
		{
			if (shape->HitTest(point))
			{
				return shape;
			}
		}
		return nullptr;
	}

	[[nodiscard]] ScopedConnection DoOnChange(ChangeSignal::slot_type slot)
	{
		return m_changeSignal.connect(std::move(slot));
	}

private:
	struct ShapeEntry
	{
		std::unique_ptr<IShape> shape;
		ScopedConnection connection;
	};

	ChangeSignal m_changeSignal;
	std::vector<ShapeEntry> m_shapes;
};
