#include <SFML/Graphics.hpp>

int main()
{
	sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Hello SFML");

	sf::RectangleShape rectangle({ 200.f, 100.f });
	rectangle.setPosition({ 300.f, 250.f });
	rectangle.setFillColor(sf::Color::Green);

	while (window.isOpen())
	{
		while (auto event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
				window.close();
		}

		window.clear();
		window.draw(rectangle);
		window.display();
	}
}
