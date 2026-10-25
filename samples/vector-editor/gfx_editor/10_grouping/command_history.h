#pragma once

#include <deque>
#include <memory>
#include <stdexcept>

class ICommand
{
public:
	virtual ~ICommand() = default;
	virtual void Execute() = 0;
	virtual void Unexecute() = 0;
};

class CommandHistory
{
public:
	void Execute(std::unique_ptr<ICommand> command)
	{
		if (!command)
		{
			throw std::invalid_argument("Command cannot be null");
		}
		if (m_nextCommandIndex < m_history.size())
		{
			m_history.erase(m_history.begin() + m_nextCommandIndex, m_history.end());
		}
		command->Execute();
		m_history.push_back(std::move(command));
		m_nextCommandIndex = m_history.size();
	}

	void Undo()
	{
		if (m_nextCommandIndex == 0)
		{
			return;
		}
		m_history[m_nextCommandIndex - 1]->Unexecute();
		--m_nextCommandIndex;
	}

	void Redo()
	{
		if (m_nextCommandIndex >= m_history.size())
		{
			return;
		}
		m_history[m_nextCommandIndex]->Execute();
		++m_nextCommandIndex;
	}

private:
	std::deque<std::unique_ptr<ICommand>> m_history;
	size_t m_nextCommandIndex = 0;
};
