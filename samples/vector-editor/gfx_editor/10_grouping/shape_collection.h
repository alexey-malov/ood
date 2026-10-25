#pragma once
#include "ishape.h"
#include <cassert>
#include <cstddef>
#include <exception>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

// Owns child shapes, their subscriptions and batch notifications.
class ShapeCollection
{
public:
    using ChangeSignal = IShape::ChangeSignal;

    ShapeCollection() = default;
    ShapeCollection(const ShapeCollection&) = delete;
    ShapeCollection& operator=(const ShapeCollection&) = delete;
    ShapeCollection(ShapeCollection&&) = delete;
    ShapeCollection& operator=(ShapeCollection&&) = delete;

    size_t GetShapeCount() const noexcept { return m_shapes.size(); }
    IShape& GetShape(size_t index) { return *m_shapes.at(index).shape; }
    const IShape& GetShape(size_t index) const { return *m_shapes.at(index).shape; }

    void AddShape(std::unique_ptr<IShape> shape) { InsertShape(m_shapes.size(), std::move(shape)); }

    void InsertShape(size_t index, std::unique_ptr<IShape> shape)
    {
        if (!shape) throw std::invalid_argument("Shape cannot be null");
        if (index > m_shapes.size()) throw std::out_of_range("Invalid insertion index");
        auto connection = shape->DoOnChange([this] { NotifyChanged(); });
        m_shapes.insert(m_shapes.begin() + index,
                        ShapeEntry{std::move(shape), std::move(connection)});
        NotifyChanged();
    }

    [[nodiscard]] std::unique_ptr<IShape> ExtractShape(size_t index)
    {
        if (index >= m_shapes.size()) throw std::out_of_range("Invalid extraction index");
        auto it = m_shapes.begin() + index;
        auto shape = std::move(it->shape);
        m_shapes.erase(it);
        NotifyChanged();
        return shape;
    }

    IShape* FindShapeAt(Point2 p)
    {
        for (auto it = m_shapes.rbegin(); it != m_shapes.rend(); ++it)
            if (it->shape->HitTest(p)) return it->shape.get();
        return nullptr;
    }
    const IShape* FindShapeAt(Point2 p) const
    {
        for (auto it = m_shapes.rbegin(); it != m_shapes.rend(); ++it)
            if (it->shape->HitTest(p)) return it->shape.get();
        return nullptr;
    }

    [[nodiscard]] ScopedConnection DoOnChange(ChangeSignal::slot_type slot)
    {
        return m_changeSignal.connect(std::move(slot));
    }

    template <typename F>
    void WithDeferredNotifications(F&& action)
    {
        ++m_notificationBatchDepth;
        try { std::invoke(std::forward<F>(action)); }
        catch (...)
        {
            const auto error = std::current_exception();
            try { EndNotificationBatch(); } catch (...) {}
            std::rethrow_exception(error);
        }
        EndNotificationBatch();
    }

private:
    struct ShapeEntry
    {
        std::unique_ptr<IShape> shape;
        ScopedConnection connection;
    };

    void NotifyChanged()
    {
        if (m_notificationBatchDepth != 0) m_hasPendingNotification = true;
        else m_changeSignal();
    }

    void EndNotificationBatch()
    {
        assert(m_notificationBatchDepth > 0);
        --m_notificationBatchDepth;
        if (m_notificationBatchDepth == 0 &&
            std::exchange(m_hasPendingNotification, false))
            m_changeSignal();
    }

    ChangeSignal m_changeSignal;
    std::vector<ShapeEntry> m_shapes;
    size_t m_notificationBatchDepth = 0;
    bool m_hasPendingNotification = false;
};
