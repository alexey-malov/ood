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
	void Draw(const Drawing& drawing, IShape* selectedShape, sf::RenderTarget& target)
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

int main()
{
	sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Hello SFML");

	Drawing drawing;
	drawing.AddShape(std::make_unique<Rectangle>(Rect{ 100.f, 100.f, 200.f, 100.f }));
	drawing.AddShape(std::make_unique<Ellipse>(Rect{ 400.f, 300.f, 150.f, 100.f }));

	DrawingRenderer renderer;

	IShape* selectedShape = nullptr;
	bool isDragging = false;
	Point2 lastMousePosition;

	while (window.isOpen())
	{
		while (auto event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
				window.close();
			if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>())
			{
				const auto point = ToPoint(mouseButtonPressed->position);
				selectedShape = drawing.FindShapeAt(point);
				isDragging = (selectedShape != nullptr);
				lastMousePosition = point;
			}
			if (auto* mouseButtonReleased = event->getIf<sf::Event::MouseButtonReleased>())
			{
				isDragging = false;
			}
			if (auto* mouseMoved = event->getIf<sf::Event::MouseMoved>(); mouseMoved && isDragging)
			{
				const auto point = ToPoint(mouseMoved->position);
				selectedShape->MoveBy(point - lastMousePosition);
				lastMousePosition = point;
			}
		}

		window.clear();
		renderer.Draw(drawing, selectedShape, window);
		window.display();
	}
}
