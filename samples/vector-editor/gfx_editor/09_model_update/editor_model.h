#pragma once
#include "command_history.h"
#include "domain_model.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

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

class MoveShapesCommand : public ICommand
{
public:
	MoveShapesCommand(std::vector<IShape*> shapes, Vector2 offset)
		: m_shapes(std::move(shapes))
		, m_offset(offset)
	{
	}

	void Execute() override
	{
		for (auto* shape : m_shapes)
		{
			shape->MoveBy(m_offset);
		}
	}

	void Unexecute() override
	{
		for (auto* shape : m_shapes)
		{
			shape->MoveBy({ -m_offset.x, -m_offset.y });
		}
	}

private:
	std::vector<IShape*> m_shapes;
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

	std::vector<const EditorShape*> GetSelectedShapes() const
	{
		std::vector<const EditorShape*> selected;
		for (const auto& shape : m_shapes)
		{
			if (shape->IsSelected())
			{
				selected.push_back(shape.get());
			}
		}
		return selected;
	}

	void SelectOnly(const EditorShape* selectedShape)
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

	void ToggleSelection(const EditorShape& selectedShape)
	{
		if (m_movePreview)
		{
			throw std::logic_error("Cannot change selection during move");
		}
		auto it = std::ranges::find_if(m_shapes, [&](const auto& shape) {
			return shape.get() == &selectedShape;
		});
		if (it == m_shapes.end())
		{
			throw std::invalid_argument("EditorShape does not belong to this EditorModel");
		}

		auto& shape = **it;
		if (shape.SetSelected(!shape.IsSelected()))
		{
			NotifyChanged();
		}
	}

	void BeginMove()
	{
		if (m_movePreview)
		{
			throw std::logic_error("Move operation is already in progress");
		}

		std::vector<EditorShape*> selected;
		for (const auto& shape : m_shapes)
		{
			if (shape->IsSelected())
			{
				selected.push_back(shape.get());
			}
		}
		if (!selected.empty())
		{
			m_movePreview = MovePreview{ .shapes = std::move(selected), .offset = {} };
		}
	}

	void UpdateMove(Vector2 requestedOffset)
	{
		if (!m_movePreview)
		{
			return;
		}

		// Позже здесь может появиться snapping:
		const auto effectiveOffset = requestedOffset;
		m_movePreview->offset = effectiveOffset;

		bool changed = false;
		for (auto* shape : m_movePreview->shapes)
		{
			changed |= shape->SetPreviewOffset(effectiveOffset);
		}
		if (changed)
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

		bool changed = false;
		for (auto* shape : m_movePreview->shapes)
		{
			changed |= shape->ResetPreview();
		}
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

		const auto offset = m_movePreview->offset;
		std::vector<IShape*> domainShapes;
		domainShapes.reserve(m_movePreview->shapes.size());
		for (auto* shape : m_movePreview->shapes)
		{
			domainShapes.push_back(&shape->GetDomainShape());
		}

		// Создаём команду только если есть ненулевой offset.
		std::unique_ptr<ICommand> command;
		if (offset.x != 0 || offset.y != 0)
		{
			command = std::make_unique<MoveShapesCommand>(std::move(domainShapes), offset);
		}

		// Перед выполнением команды убираем preview
		bool changed = false;
		for (auto* shape : m_movePreview->shapes)
		{
			changed |= shape->ResetPreview();
		}
		m_movePreview.reset();

		if (command)
		{
			m_commandSink(std::move(command));
		}
		else if (changed)
		{
			NotifyChanged();
		}
	}

	[[nodiscard]] ScopedConnection DoOnChange(ChangeSignal::slot_type slot)
	{
		return m_changeSignal.connect(std::move(slot));
	}

private:
	struct MovePreview
	{
		std::vector<EditorShape*> shapes;
		Vector2 offset;
	};

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