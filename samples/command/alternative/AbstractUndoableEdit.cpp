#include "AbstractUndoableEdit.h"
#include <stdexcept>
#include <cassert>

namespace undo
{

bool AbstractUndoableEdit::CanUndo() const
{
	return m_isAlive && m_executed;
}

bool AbstractUndoableEdit::CanRedo() const
{
	return m_isAlive && !m_executed;
}

void AbstractUndoableEdit::Undo()
{
	if (!CanUndo())
	{
		throw std::logic_error("Can't undo");
	}
	m_executed = false;
	UndoImpl();
}

void AbstractUndoableEdit::Redo()
{
	if (!CanRedo())
	{
		throw std::logic_error("Can't redo");
	}
	m_executed = true;
	RedoImpl();
}

void AbstractUndoableEdit::Destroy() noexcept
{
	if (!m_isAlive)
	{
		m_isAlive = false;
		DestroyImpl();
	}
}

bool AbstractUndoableEdit::AddEdit(const IUndoableEditPtr& edit)
{
	if (!edit)
	{
		throw std::invalid_argument("Invalid edit");
	}
	if (!m_isAlive)
	{
		throw std::logic_error("Edit has been already destroyed");
	}
	return AddEditImpl(edit);
}

bool AbstractUndoableEdit::ReplaceEdit(const IUndoableEditPtr& edit)
{
	if (!edit)
	{
		throw std::invalid_argument("Invalid edit");
	}
	if (!m_isAlive)
	{
		throw std::logic_error("Edit has been already destroyed");
	}
	return ReplaceEditImpl(edit);
}

std::string AbstractUndoableEdit::GetName() const
{
	return m_name;
}

} // namespace undo
