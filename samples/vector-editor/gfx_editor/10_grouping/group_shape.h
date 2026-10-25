#pragma once
#include "ishape.h"
#include <memory>
#include <stdexcept>
#include <vector>

class GroupShape final : public IShape
{
public:
	GroupShape() = default;
	GroupShape(const GroupShape&) = delete;
	GroupShape& operator=(const GroupShape&) = delete;
	GroupShape(GroupShape&&) = delete;
	GroupShape& operator=(GroupShape&&) = delete;

	size_t GetShapeCount() const noexcept
	{
		return m_shapes.size();
	}

	IShape& GetShape(size_t index)
	{
		return *m_shapes.at(index).shape;
	}

	const IShape& GetShape(size_t index) const
	{
		return *m_shapes.at(index).shape;
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

		m_shapes.insert(m_shapes.begin() + index,
			ShapeEntry{ std::move(shape), std::move(connection) });
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
		m_shapes.erase(it); // Disconnect from the extracted shape.
		NotifyChanged();
		return shape;
	}

	Rect GetBounds() const override
	{
		// Empty groups can exist transiently during Group/Undo/Ungroup.
		if (m_shapes.empty())
		{
			return {};
		}

		const auto first = m_shapes.front().shape->GetBounds();
		float left = first.x;
		float top = first.y;
		float right = first.x + first.width;
		float bottom = first.y + first.height;

		for (size_t i = 1; i < m_shapes.size(); ++i)
		{
			const auto bounds = m_shapes[i].shape->GetBounds();
			left = std::min(left, bounds.x);
			top = std::min(top, bounds.y);
			right = std::max(right, bounds.x + bounds.width);
			bottom = std::max(bottom, bounds.y + bounds.height);
		}

		return { left, top, right - left, bottom - top };
	}

	bool HitTest(Point2 point) const override
	{
		// Hit only painted child geometry, not empty space inside bounds.
		for (auto it = m_shapes.rbegin(); it != m_shapes.rend(); ++it)
		{
			if (it->shape->HitTest(point))
			{
				return true;
			}
		}
		return false;
	}

	void MoveBy(Vector2 offset) override
	{
		if (m_shapes.empty() || (offset.x == 0 && offset.y == 0))
		{
			return;
		}

		// A single group move emits at most one GroupShape change signal,
		// even though every child may emit its own signal.
		WithDeferredNotifications([&] {
			for (auto& entry : m_shapes)
			{
				entry.shape->MoveBy(offset);
			}
		});
	}

	void Accept(IShapeVisitor& visitor) const override
	{
		visitor.Visit(*this);
	}

	[[nodiscard]] ScopedConnection DoOnChange(ChangeSignal::slot_type slot) override
	{
		return m_changeSignal.connect(std::move(slot));
	}

private:
	struct ShapeEntry
	{
		std::unique_ptr<IShape> shape;
		ScopedConnection connection;
	};

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
				// Keep the original error from action.
			}
			std::rethrow_exception(originalError);
		}
		EndNotificationBatch();
	}

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

	ChangeSignal m_changeSignal;
	std::vector<ShapeEntry> m_shapes;
	size_t m_notificationBatchDepth = 0;
	bool m_hasPendingNotification = false;
};
