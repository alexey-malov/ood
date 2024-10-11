#include "AbstractUndoableEdit.h"
#include <stdexcept>
#include <cassert>

namespace undo
{

using namespace std;

bool AbstractUndoableEdit::CanUndo() const
{
	return IsExecuted() && m_isAlive;
}

bool AbstractUndoableEdit::CanRedo() const
{
	return !IsExecuted() && m_isAlive;
}

void AbstractUndoableEdit::Undo()
{
	if (!CanUndo())
	{
		throw logic_error("Can't undo");
	}
	SetExecuted(false);
	UndoImpl();
}

void AbstractUndoableEdit::Redo()
{
	if (!CanRedo())
	{
		throw logic_error("Can't redo");
	}
	SetExecuted(true);
	RedoImpl();
}

void AbstractUndoableEdit::Destroy() noexcept
{
	if (m_isAlive)
	{
		m_isAlive = false;
		DestroyImpl();
	}
}

bool AbstractUndoableEdit::AddEdit(const IUndoableEditPtr& edit)
{
	assert(edit);
	if (!m_isAlive)
	{
		throw logic_error("UndoableEdit has been destroyed");
	}
	return AddEditImpl(edit);
}

bool AbstractUndoableEdit::ReplaceEdit(const IUndoableEditPtr& edit)
{
	assert(edit);
	if (!m_isAlive)
	{
		throw logic_error("UndoableEdit has been destroyed");
	}
	if (m_canBeReplaced)
	{
		return ReplaceEditImpl(edit);
	}
	return false;
}

std::string AbstractUndoableEdit::GetName() const
{
	assert(!m_name.empty());
	return m_name;
}

void AbstractUndoableEdit::SetReplaceable(bool canBeReplaced)
{
	m_canBeReplaced = canBeReplaced;
}

void AbstractUndoableEdit::UndoImpl()
{
}

void AbstractUndoableEdit::RedoImpl()
{
}

void AbstractUndoableEdit::DestroyImpl() noexcept
{
}

bool AbstractUndoableEdit::AddEditImpl(const IUndoableEditPtr& /*edit*/)
{
	return false;
}

bool AbstractUndoableEdit::ReplaceEditImpl(const IUndoableEditPtr& /*edit*/)
{
	return false;
}

bool AbstractUndoableEdit::IsExecuted() const
{
	return m_executed;
}

void AbstractUndoableEdit::SetExecuted(bool executed)
{
	m_executed = executed;
}

void AbstractUndoableEdit::SetName(const std::string& name)
{
	m_name = name;
}

} // namespace undo
