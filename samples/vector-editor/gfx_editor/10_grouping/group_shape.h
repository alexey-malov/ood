#pragma once
#include "shape_collection.h"
#include <algorithm>
#include <utility>

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
		return m_children.GetShapeCount();
	}

    IShape& GetShape(size_t index)
	{
		return m_children.GetShape(index);
	}

    const IShape& GetShape(size_t index) const
	{
		return m_children.GetShape(index);
	}

    void AddShape(std::unique_ptr<IShape> shape)
	{
		m_children.AddShape(std::move(shape));
	}

    void InsertShape(size_t index, std::unique_ptr<IShape> shape)
    {
        m_children.InsertShape(index, std::move(shape));
    }
    [[nodiscard]] std::unique_ptr<IShape> ExtractShape(size_t index)
    {
        return m_children.ExtractShape(index);
    }

    Rect GetBounds() const override
    {
        if (m_children.GetShapeCount() == 0)
		{
			return {};
		}

        auto first = m_children.GetShape(0).GetBounds();
        float left = first.x;
        float top = first.y;
        float right = first.x + first.width;
        float bottom = first.y + first.height;
        for (size_t i = 1; i < m_children.GetShapeCount(); ++i)
        {
            const auto r = m_children.GetShape(i).GetBounds();
            left = std::min(left, r.x);
            top = std::min(top, r.y);
            right = std::max(right, r.x + r.width);
            bottom = std::max(bottom, r.y + r.height);
        }
        return {left, top, right - left, bottom - top};
    }

    bool HitTest(Point2 point) const override
	{
		return m_children.FindShapeAt(point) != nullptr;
	}

    void MoveBy(Vector2 offset) override
    {
        if (m_children.GetShapeCount() == 0 || (offset.x == 0 && offset.y == 0))
		{
			return;
		}

        m_children.WithDeferredNotifications([&] {
            for (size_t i = 0; i < m_children.GetShapeCount(); ++i)
			{
                m_children.GetShape(i).MoveBy(offset);
			}
        });
    }
    void Accept(IShapeVisitor& visitor) const override
	{
		visitor.Visit(*this);
	}

    [[nodiscard]] ScopedConnection DoOnChange(ChangeSignal::slot_type slot) override
    {
        return m_children.DoOnChange(std::move(slot));
    }
private:
    ShapeCollection m_children;
};
