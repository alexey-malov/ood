#pragma once
#include "editor_model.h"
#include <SFML/Graphics.hpp>

class DrawingRenderer final : public IEditorShapeVisitor
{
public:
	explicit DrawingRenderer(sf::RenderTarget& target)
		: m_target(target)
	{
	}

	void Draw(const EditorModel& editorModel)
	{
		for (size_t i = 0; i < editorModel.GetShapeCount(); ++i)
		{
			editorModel.GetShape(i).Accept(*this);
		}

		if (const auto* selectedShape = editorModel.GetSelectedShape())
		{
			DrawSelection(*selectedShape);
		}
	}

private:
	void VisitRectangle(const EditorShape& shape) override
	{
		const auto bounds = shape.GetBounds();

		sf::RectangleShape sfRectangle({ bounds.width, bounds.height });

		sfRectangle.setPosition({ bounds.x, bounds.y });

		sfRectangle.setFillColor(sf::Color::Green);

		m_target.draw(sfRectangle);
	}

	void VisitEllipse(const EditorShape& shape) override
	{
		const auto bounds = shape.GetBounds();

		sf::CircleShape sfEllipse(bounds.width / 2.0f);

		sfEllipse.setPosition({ bounds.x, bounds.y });

		sfEllipse.setScale({ 1.0f, bounds.height / bounds.width });

		sfEllipse.setFillColor(sf::Color::Blue);

		m_target.draw(sfEllipse);
	}

	void DrawSelection(const EditorShape& shape)
	{
		const auto bounds = shape.GetBounds();

		sf::RectangleShape sfOutline({ bounds.width, bounds.height });

		sfOutline.setPosition({ bounds.x, bounds.y });

		sfOutline.setFillColor(sf::Color::Transparent);

		sfOutline.setOutlineColor(sf::Color::Red);

		sfOutline.setOutlineThickness(3.0f);

		m_target.draw(sfOutline);
	}

	sf::RenderTarget& m_target;
};

Point2 ToPoint(sf::Vector2i position)
{
	return {
		.x = static_cast<float>(position.x),
		.y = static_cast<float>(position.y)
	};
}

class CanvasController
{
public:
	explicit CanvasController(EditorModel& editorModel)
		: m_editorModel(editorModel)
	{
	}

	void OnMousePressed(Point2 point)
	{
		auto* selectedShape = m_editorModel.FindShapeAt(point);

		m_editorModel.Select(selectedShape);

		if (selectedShape)
		{
			m_editorModel.BeginMove();

			m_isDragging = true;
			m_dragStartPosition = point;
		}
		else
		{
			m_isDragging = false;
		}
	}

	void OnMouseMoved(Point2 point)
	{
		if (!m_isDragging)
		{
			return;
		}

		m_editorModel.UpdateMove(point - m_dragStartPosition);
	}

	void OnMouseReleased(Point2 point)
	{
		if (!m_isDragging)
		{
			return;
		}

		// MouseReleased может прийти в позиции,
		// для которой ещё не было MouseMoved.
		m_editorModel.UpdateMove(point - m_dragStartPosition);

		m_editorModel.CommitMove();

		m_isDragging = false;
	}

private:
	EditorModel& m_editorModel;

	bool m_isDragging = false;
	Point2 m_dragStartPosition;
};

class CanvasView
{
public:
	explicit CanvasView(EditorModel& editorModel)
		: m_editorModel(editorModel)
		, m_controller(editorModel)
	{
	}

	void HandleEvent(const sf::Event& event)
	{
		if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>())
		{
			m_controller.OnMousePressed(ToPoint(pressed->position));
		}
		else if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>())
		{
			m_controller.OnMouseReleased(ToPoint(released->position));
		}
		else if (const auto* moved = event.getIf<sf::Event::MouseMoved>())
		{
			m_controller.OnMouseMoved(ToPoint(moved->position));
		}
	}

	void Draw(sf::RenderTarget& target) const
	{
		DrawingRenderer renderer{ target };
		renderer.Draw(m_editorModel);
	}

private:
	EditorModel& m_editorModel;
	CanvasController m_controller;
};
