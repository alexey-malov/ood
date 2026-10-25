#pragma once
#include "canvas_view.h"
#include "drawing_document.h"

class EditorController
{
public:
	explicit EditorController(DrawingDocument& document)
		: m_document(document)
	{
	}

	void Undo()
	{
		m_document.Undo();
	}

	void Redo()
	{
		m_document.Redo();
	}

private:
	DrawingDocument& m_document;
};

class EditorWindow
{
public:
	EditorWindow(DrawingDocument& document)
		: m_window(sf::VideoMode({ 800, 600 }), "Hello SFML")
		, m_editorModel(document.GetDrawing(), [&document](std::unique_ptr<ICommand> command) {
			document.ExecuteCommand(std::move(command));
		})
		, m_editorController(document)
		, m_canvasView(m_editorModel)
	{
	}

	void Run()
	{
		while (m_window.isOpen())
		{
			while (auto event = m_window.pollEvent())
			{
				HandleEvent(*event);
			}

			m_window.clear();
			m_canvasView.Draw(m_window);
			m_window.display();
		}
	}

private:
	void HandleEvent(const sf::Event& event)
	{
		if (event.is<sf::Event::Closed>())
		{
			m_window.close();
			return;
		}

		if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
		{
			if (keyPressed->code == sf::Keyboard::Key::Z && keyPressed->control)
			{
				m_editorController.Undo();
				return;
			}

			if (keyPressed->code == sf::Keyboard::Key::Y && keyPressed->control)
			{
				m_editorController.Redo();
				return;
			}
		}

		m_canvasView.HandleEvent(event);
	}

	sf::RenderWindow m_window;

	EditorModel m_editorModel;
	EditorController m_editorController;
	CanvasView m_canvasView;
};
