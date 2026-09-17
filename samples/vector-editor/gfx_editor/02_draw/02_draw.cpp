#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

struct Rect
{
	float x = 0.0f;
	float y = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
};

class IShape
{
public:
	virtual ~IShape() = default;

	virtual Rect GetBounds() const = 0;
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
};

class Ellipse : public Shape
{
public:
	explicit Ellipse(Rect bounds)
		: Shape(bounds)
	{
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

private:
	std::vector<std::unique_ptr<IShape>> m_shapes;
};

class DrawingRenderer
{
public:
	void Draw(const Drawing& drawing, sf::RenderTarget& target)
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
	}
};

int main()
{
	sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Hello SFML");

	Drawing drawing;
	drawing.AddShape(std::make_unique<Rectangle>(Rect{ 100.f, 100.f, 200.f, 100.f }));
	drawing.AddShape(std::make_unique<Ellipse>(Rect{ 400.f, 300.f, 150.f, 100.f }));

	DrawingRenderer renderer;

	while (window.isOpen())
	{
		while (auto event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
				window.close();
		}

		window.clear();
		renderer.Draw(drawing, window);
		window.display();
	}
}
