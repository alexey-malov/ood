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

	void Undo()
	{
		m_commandHistory.Undo();
	}

	void Redo()
	{
		m_commandHistory.Redo();
	}

	void ExecuteCommand(std::unique_ptr<ICommand> command)
	{
		m_commandHistory.Execute(std::move(command));
	}

private:
	Drawing m_drawing;
	// Команды могут ссылаться на DrawingDocument
	// поэтому CommandHistory должен быть объявлен после Drawing.
	CommandHistory m_commandHistory;
};
