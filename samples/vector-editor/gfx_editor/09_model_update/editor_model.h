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
		auto bounds = m_shape->GetBounds();

		bounds.x += m_previewOffset.x;
		bounds.y += m_previewOffset.y;

		return bounds;
	}

	bool HitTest(Point2 point) const
	{
		// Пока единственный preview transform — перемещение.
		// Переводим точку обратно в committed coordinates фигуры.
		return m_shape->HitTest({ point.x - m_previewOffset.x,
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
		m_shape->Accept(adapter);
	}

private:
	friend class EditorModel;

	explicit EditorShape(IShape& shape)
		: m_shape(&shape)
	{
	}

	IShape* GetDomainShapePtr()
	{
		return m_shape;
	}

	const IShape* GetDomainShapePtr() const
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

	IShape* m_shape;

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

class DeleteShapesCommand final : public ICommand
{
public:
	DeleteShapesCommand(Drawing& drawing, std::vector<size_t> indices)
		: m_drawing(drawing)
	{
		if (indices.empty())
			throw std::invalid_argument("No shapes to delete");

		for (size_t i = 0; i < indices.size(); ++i)
		{
			if (indices[i] >= drawing.GetShapeCount() // Индексы должны быть в пределах диапазона
				|| (i > 0 && indices[i] < indices[i - 1]) // Индексы должны быть строго возрастающими (без дубликатов)
			)
			{
				throw std::invalid_argument("Invalid shape indices");
			}
		}

		m_removed.reserve(indices.size());

		for (size_t index : indices)
		{
			m_removed.push_back({ index, nullptr });
		}
	}

	void Execute() override
	{
		// Удаляем с конца, чтобы индексы не смещались.
		for (auto it = m_removed.rbegin();
			it != m_removed.rend(); ++it)
		{
			it->shape = m_drawing.ExtractShape(it->index);
		}
	}

	void Unexecute() override
	{
		// Вставляем с начала, восстанавливая Z-order.
		for (auto& removed : m_removed)
		{
			m_drawing.InsertShape(removed.index, std::move(removed.shape));
		}
	}

private:
	struct RemovedShape
	{
		size_t index;
		std::unique_ptr<IShape> shape;
	};

	Drawing& m_drawing;
	std::vector<RemovedShape> m_removed;
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
			domainShapes.push_back(shape->GetDomainShapePtr());
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

	void DeleteSelectedShapes()
	{
		if (m_movePreview)
		{
			CancelMove();
		}

		std::vector<size_t> indices;

		for (size_t i = 0; i < m_shapes.size(); ++i)
		{
			if (m_shapes[i]->IsSelected())
			{
				indices.push_back(i);
			}
		}

		if (!indices.empty())
		{
			m_commandSink(
				std::make_unique<DeleteShapesCommand>(m_drawing, std::move(indices)));
		}
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
		const size_t count = m_drawing.GetShapeCount();

		bool sameStructure = m_shapes.size() == count;

		if (sameStructure)
		{
			for (size_t i = 0; i < count; ++i)
			{
				if (m_shapes[i]->GetDomainShapePtr() != &m_drawing.GetShape(i))
				{
					sameStructure = false;
					break;
				}
			}
		}

		if (sameStructure)
		{
			return;
		}

		const size_t notFound = m_shapes.size();

		std::vector<size_t> oldIndices(count, notFound);
		std::vector<std::unique_ptr<EditorShape>> updated(count);

		// Подготавливаем новую коллекцию.
		for (size_t i = 0; i < count; ++i)
		{
			auto& domainShape = m_drawing.GetShape(i);

			auto it = std::ranges::find_if(
				m_shapes,
				[&](const auto& editorShape) {
					return editorShape->GetDomainShapePtr()
						== &domainShape;
				});

			// Если фигура уже есть в старой коллекции, запоминаем её индекс.
			if (it != m_shapes.end())
			{
				oldIndices[i] = static_cast<size_t>(it - m_shapes.begin());
			}
			else // Если фигура новая, создаём EditorShape.
			{
				updated[i].reset(new EditorShape(domainShape));
			}
		}

		// Структурное изменение прерывает активный Preview.
		if (m_movePreview)
		{
			for (auto* shape : m_movePreview->shapes)
			{
				shape->ResetPreview();
			}

			m_movePreview.reset();
		}

		// Переносим существующие EditorShape без их пересоздания.
		for (size_t i = 0; i < count; ++i)
		{
			if (oldIndices[i] != notFound)
			{
				updated[i] = std::move(m_shapes[oldIndices[i]]);
			}
		}

		m_shapes.swap(updated);
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