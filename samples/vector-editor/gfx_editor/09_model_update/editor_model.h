#pragma once
#include "command_history.h"
#include "domain_model.h"
#include <functional>
#include <memory>
#include <optional>

class EditorShape;

class IEditorShapeVisitor
{
public:
	virtual void VisitRectangle(const EditorShape& shape) = 0;
	virtual void VisitEllipse(const EditorShape& shape) = 0;

protected:
	~IEditorShapeVisitor() = default;
};

class EditorModel;

class EditorShape
{
public:
	Rect GetBounds() const
	{
		auto bounds = m_shape.GetBounds();

		bounds.x += m_previewOffset.x;
		bounds.y += m_previewOffset.y;

		return bounds;
	}

	bool HitTest(Point2 point) const
	{
		// Пока единственный preview transform — перемещение.
		// Переводим точку обратно в committed coordinates фигуры.
		return m_shape.HitTest({ point.x - m_previewOffset.x,
			point.y - m_previewOffset.y });
	}

	bool IsSelected() const
	{
		return m_selected;
	}

	void Accept(IEditorShapeVisitor& visitor) const
	{
		class VisitorAdapter final : public IShapeVisitor
		{
		public:
			VisitorAdapter(const EditorShape& shape, IEditorShapeVisitor& visitor)
				: m_shape(shape)
				, m_visitor(visitor)
			{
			}

			void Visit(const Rectangle&) override
			{
				m_visitor.VisitRectangle(m_shape);
			}

			void Visit(const Ellipse&) override
			{
				m_visitor.VisitEllipse(m_shape);
			}

		private:
			const EditorShape& m_shape;
			IEditorShapeVisitor& m_visitor;
		};

		VisitorAdapter adapter(*this, visitor);
		m_shape.Accept(adapter);
	}

private:
	friend class EditorModel;

	explicit EditorShape(IShape& shape)
		: m_shape(shape)
	{
	}

	IShape& GetDomainShape()
	{
		return m_shape;
	}

	const IShape& GetDomainShape() const
	{
		return m_shape;
	}

	Vector2 GetPreviewOffset() const
	{
		return m_previewOffset;
	}

	bool SetPreviewOffset(Vector2 offset)
	{
		if (m_previewOffset.x == offset.x
			&& m_previewOffset.y == offset.y)
		{
			return false;
		}

		m_previewOffset = offset;
		return true;
	}

	bool ResetPreview()
	{
		return SetPreviewOffset({});
	}

	bool SetSelected(bool selected)
	{
		if (m_selected == selected)
		{
			return false;
		}

		m_selected = selected;
		return true;
	}

	IShape& m_shape;

	Vector2 m_previewOffset;
	bool m_selected = false;
};

class MoveShapeCommand : public ICommand
{
public:
	MoveShapeCommand(IShape& shape, Vector2 offset)
		: m_shape(shape)
		, m_offset(offset)
	{
	}
	void Execute() override
	{
		m_shape.MoveBy(m_offset);
	}
	void Unexecute() override
	{
		m_shape.MoveBy({ -m_offset.x, -m_offset.y });
	}

private:
	IShape& m_shape;
	Vector2 m_offset;
};

class EditorModel
{
public:
	using ChangeSignal = boost::signals2::signal<void()>;
	using CommandSink = std::function<void(std::unique_ptr<ICommand>)>;

	EditorModel(Drawing& drawing, CommandSink commandSink)
		: m_drawing(drawing)
		, m_commandSink(std::move(commandSink))
	{
		SyncShapes();

		m_drawingConnection = m_drawing.DoOnChange([this] {
			OnDrawingChanged();
		});
	}

	size_t GetShapeCount() const
	{
		return m_shapes.size();
	}

	const EditorShape& GetShape(size_t index) const
	{
		return *m_shapes.at(index);
	}

	const EditorShape* FindShapeAt(Point2 point) const
	{
		for (const auto& shape : m_shapes | std::views::reverse)
		{
			if (shape->HitTest(point))
			{
				return shape.get();
			}
		}

		return nullptr;
	}

	const EditorShape* GetSelectedShape() const
	{
		for (const auto& shape : m_shapes)
		{
			if (shape->IsSelected())
			{
				return shape.get();
			}
		}

		return nullptr;
	}

	void Select(const EditorShape* selectedShape)
	{
		if (m_movePreview)
		{
			throw std::logic_error("Cannot change selection during move");
		}

		if (selectedShape && !Contains(*selectedShape))
		{
			throw std::invalid_argument("EditorShape does not belong to this EditorModel");
		}

		bool changed = false;

		for (const auto& shape : m_shapes)
		{
			changed |= shape->SetSelected(shape.get() == selectedShape);
		}

		if (changed)
		{
			NotifyChanged();
		}
	}

	void BeginMove()
	{
		if (m_movePreview)
		{
			throw std::logic_error(
				"Move operation is already in progress");
		}

		auto* shape = GetSelectedShape();

		if (!shape)
		{
			return;
		}

		m_movePreview = MovePreview{ .shape = shape };
	}

	void UpdateMove(Vector2 requestedOffset)
	{
		if (!m_movePreview)
		{
			return;
		}

		// Позже здесь может появиться snapping:
		//
		// auto snapResult = m_snapper.Snap(...);
		// auto effectiveOffset = snapResult.effectiveOffset;

		const auto effectiveOffset = requestedOffset;

		if (m_movePreview->shape->SetPreviewOffset(effectiveOffset))
		{
			NotifyChanged();
		}
	}

	void CancelMove()
	{
		if (!m_movePreview)
		{
			return;
		}

		const bool changed = m_movePreview->shape->ResetPreview();

		m_movePreview.reset();

		if (changed)
		{
			NotifyChanged();
		}
	}

	void CommitMove()
	{
		if (!m_movePreview)
		{
			return;
		}

		auto* editorShape = m_movePreview->shape;
		const auto offset = editorShape->GetPreviewOffset();

		// Важно убрать preview ДО Execute.
		// Иначе notification от Drawing увидит:
		//
		// new committed state + old preview offset.
		editorShape->ResetPreview();
		m_movePreview.reset();

		if (offset.x == 0 && offset.y == 0)
		{
			NotifyChanged();
			return;
		}

		auto command = std::make_unique<MoveShapeCommand>(
			editorShape->GetDomainShape(),
			offset);

		m_commandSink(std::move(command));
	}

	[[nodiscard]] ScopedConnection DoOnChange(ChangeSignal::slot_type slot)
	{
		return m_changeSignal.connect(std::move(slot));
	}

private:
	struct MovePreview
	{
		EditorShape* shape = nullptr;
	};

	EditorShape* GetSelectedShape()
	{
		return const_cast<EditorShape*>(std::as_const(*this).GetSelectedShape());
	}

	bool Contains(const EditorShape& shape) const
	{
		return std::ranges::any_of(m_shapes,
			[&](const auto& candidate) {
				return candidate.get() == &shape;
			});
	}

	void OnDrawingChanged()
	{
		SyncShapes();
		NotifyChanged();
	}

	void SyncShapes()
	{
		// В текущем Drawing есть только AddShape,
		// поэтому достаточно добавлять новые EditorShape.
		//
		// Когда появятся RemoveShape / изменение Z-order,
		// эту синхронизацию придётся развить.

		while (m_shapes.size() < m_drawing.GetShapeCount())
		{
			auto& shape = m_drawing.GetShape(m_shapes.size());

			m_shapes.emplace_back(new EditorShape(shape));
		}
	}

	void NotifyChanged()
	{
		m_changeSignal();
	}

	Drawing& m_drawing;
	CommandSink m_commandSink;

	std::vector<std::unique_ptr<EditorShape>> m_shapes;

	std::optional<MovePreview> m_movePreview;

	ChangeSignal m_changeSignal;
	ScopedConnection m_drawingConnection;
};