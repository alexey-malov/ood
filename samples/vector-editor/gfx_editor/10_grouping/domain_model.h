#pragma once
#include "ishape.h"
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
	using ChangeSignal = boost::signals2::signal<void()>;

	Drawing() = default;

	// Запрещаем копирование и перемещение объекта Drawing, чтобы избежать проблем с управлением ресурсами и сигналами.
	Drawing(const Drawing&) = delete;
	Drawing& operator=(const Drawing&) = delete;
	Drawing(Drawing&&) = delete;
	Drawing& operator=(Drawing&&) = delete;

	// Откладывает уведомления об изменениях до завершения самой внешней операции
	// Если действие вызывает исключение
	template <typename F>
	void WithDeferredNotifications(F&& action)
	{
		++m_notificationBatchDepth;
		try
		{
			std::invoke(std::forward<F>(action));
		}
		catch (...)
		{
			const auto originalError = std::current_exception();
			try
			{
				EndNotificationBatch();
			}
			catch (...)
			{
				// Preserve the exception raised by action.
			}
			std::rethrow_exception(originalError);
		}
		EndNotificationBatch();
	}

	void AddShape(std::unique_ptr<IShape> shape)
	{
		InsertShape(m_shapes.size(), std::move(shape));
	}

	void InsertShape(size_t index, std::unique_ptr<IShape> shape)
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
			NotifyChanged();
		});

		m_shapes.insert(
			m_shapes.begin() + index,
			ShapeEntry{
				std::move(shape),
				std::move(connection) });

		NotifyChanged();
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

		NotifyChanged();

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
	void NotifyChanged()
	{
		if (m_notificationBatchDepth != 0)
		{
			m_hasPendingNotification = true;
		}
		else
		{
			m_changeSignal();
		}
	}

	void EndNotificationBatch()
	{
		assert(m_notificationBatchDepth > 0);
		--m_notificationBatchDepth;
		if (m_notificationBatchDepth == 0
			&& std::exchange(m_hasPendingNotification, false))
		{
			m_changeSignal();
		}
	}

	struct ShapeEntry
	{
		std::unique_ptr<IShape> shape;
		ScopedConnection connection;
	};

	ChangeSignal m_changeSignal;
	std::vector<ShapeEntry> m_shapes;
	size_t m_notificationBatchDepth = 0;
	bool m_hasPendingNotification = false;
};
