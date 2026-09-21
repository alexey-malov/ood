#include <SFML/Graphics.hpp>
#include <memory>
#include <ranges>
#include <vector>

struct Rect
{
	float x = 0.0f;
	float y = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
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

class IShape
{
public:
	virtual ~IShape() = default;

	virtual Rect GetBounds() const = 0;
	virtual bool HitTest(Point2 point) const = 0;
	virtual void MoveBy(Vector2 offset) = 0;
};

class Shape : public IShape
{
protected:
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
		m_bounds.x += offset.x;
		m_bounds.y += offset.y;
	}

private:
	Rect m_bounds;
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
};

class Drawing
{
public:
	void AddShape(std::unique_ptr<IShape> shape)
	{
		m_shapes.push_back(std::move(shape));
	}

	size_t GetShapeCount() const { return m_shapes.size(); }

	const IShape& GetShape(size_t index) const { return *m_shapes.at(index); }

	IShape* FindShapeAt(Point2 point)
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

private:
	std::vector<std::unique_ptr<IShape>> m_shapes;
};

class DrawingRenderer
{
public:
	void Draw(const Drawing& drawing, const IShape* selectedShape, sf::RenderTarget& target)
	{
		for (size_t i = 0; i < drawing.GetShapeCount(); ++i)
		{
			const IShape& shape = drawing.GetShape(i);
			const auto bounds = shape.GetBounds();
			if (const auto* rectangle = dynamic_cast<const Rectangle*>(&shape))
			{
				sf::RectangleShape sfRectangle({ bounds.width, bounds.height });
				sfRectangle.setPosition({ bounds.x, bounds.y });
				sfRectangle.setFillColor(sf::Color::Green);
				target.draw(sfRectangle);
			}
			else if (const auto* ellipse = dynamic_cast<const Ellipse*>(&shape))
			{
				sf::CircleShape sfEllipse(bounds.width / 2.0f);
				sfEllipse.setPosition({ bounds.x, bounds.y });
				sfEllipse.setScale({ 1.0f, bounds.height / bounds.width });
				sfEllipse.setFillColor(sf::Color::Blue);
				target.draw(sfEllipse);
			}
		}
		if (selectedShape)
		{
			const auto bounds = selectedShape->GetBounds();
			sf::RectangleShape sfOutline({ bounds.width, bounds.height });
			sfOutline.setPosition({ bounds.x, bounds.y });
			sfOutline.setFillColor(sf::Color::Transparent);
			sfOutline.setOutlineColor(sf::Color::Red);
			sfOutline.setOutlineThickness(3.0f);
			target.draw(sfOutline);
		}
	}
};

Point2 ToPoint(sf::Vector2i position)
{
	return {
		.x = static_cast<float>(position.x),
		.y = static_cast<float>(position.y)
	};
}

class EditorSession
{
public:
	IShape* GetSelectedShape()
	{
		return m_selectedShape;
	}

	const IShape* GetSelectedShape() const
	{
		return m_selectedShape;
	}

	void SetSelectedShape(IShape* shape)
	{
		m_selectedShape = shape;
	}

private:
	IShape* m_selectedShape = nullptr;
};

class CanvasController
{
public:
	CanvasController(Drawing& drawing, EditorSession& session)
		: m_drawing(drawing)
		, m_session(session)
	{
	}
	void OnMousePressed(Point2 point)
	{
		auto selectedShape = m_drawing.FindShapeAt(point);
		m_session.SetSelectedShape(selectedShape);
		m_isDragging = (selectedShape != nullptr);
		m_lastMousePosition = point;
	}
	void OnMouseMoved(Point2 point)
	{
		if (auto selectedShape = m_session.GetSelectedShape();
			m_isDragging && selectedShape)
		{
			selectedShape->MoveBy(point - m_lastMousePosition);
			m_lastMousePosition = point;
		}
	}

	void OnMouseReleased(Point2 point)
	{
		m_isDragging = false;
	}

private:
	Drawing& m_drawing;
	EditorSession& m_session;

	bool m_isDragging = false;
	Point2 m_lastMousePosition;
};

class CanvasView
{
public:
	CanvasView(Drawing& drawing, EditorSession& session)
		: m_drawing(drawing)
		, m_session(session)
		, m_controller(drawing, session)
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

	void Draw(sf::RenderTarget& target)
	{
		m_renderer.Draw(m_drawing, m_session.GetSelectedShape(), target);
	}

private:
	const Drawing& m_drawing;
	const EditorSession& m_session;
	CanvasController m_controller;

	DrawingRenderer m_renderer;
};

class EditorWindow
{
public:
	explicit EditorWindow(Drawing& drawing)
		: m_window(sf::VideoMode({ 800, 600 }), "Hello SFML")
		, m_canvasView(drawing, m_session)
	{
	}

	void Run()
	{
		while (m_window.isOpen())
		{
			while (auto event = m_window.pollEvent())
			{
				if (event->is<sf::Event::Closed>())
				{
					m_window.close();
				}
				else
				{
					m_canvasView.HandleEvent(*event);
				}
			}

			m_window.clear();

			m_canvasView.Draw(m_window);

			m_window.display();
		}
	}

private:
	sf::RenderWindow m_window;
	EditorSession m_session;
	CanvasView m_canvasView;
};

int main()
{
	Drawing drawing;
	drawing.AddShape(std::make_unique<Rectangle>(
		Rect{ 100.f, 100.f, 200.f, 100.f }));

	drawing.AddShape(std::make_unique<Ellipse>(
		Rect{ 400.f, 300.f, 150.f, 100.f }));

	EditorWindow window{ drawing };

	window.Run();
}
