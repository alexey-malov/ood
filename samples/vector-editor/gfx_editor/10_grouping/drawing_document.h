#pragma once
#include "command_history.h"
#include "domain_model.h"

class DrawingDocument
{
public:
	DrawingDocument() = default;

	DrawingDocument(const DrawingDocument&) = delete;
	DrawingDocument& operator=(const DrawingDocument&) = delete;

	Drawing& GetDrawing() noexcept
	{
		return m_drawing;
	}

	const Drawing& GetDrawing() const noexcept
	{
		return m_drawing;
	}

	void ExecuteCommand(std::unique_ptr<ICommand> command)
	{
		m_drawing.WithDeferredNotifications([&] {
			m_commandHistory.Execute(std::move(command));
		});
	}

	void Undo()
	{
		m_drawing.WithDeferredNotifications([&] {
			m_commandHistory.Undo();
		});
	}

	void Redo()
	{
		m_drawing.WithDeferredNotifications([&] {
			m_commandHistory.Redo();
		});
	}

private:
	Drawing m_drawing;
	// Команды могут ссылаться на DrawingDocument
	// поэтому CommandHistory должен быть объявлен после Drawing.
	CommandHistory m_commandHistory;
};
