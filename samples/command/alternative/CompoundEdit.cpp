#include "CompoundEdit.h"
#include <cassert>
#include <ranges>

namespace undo
{

bool CompoundEdit::CanUndo() const
{
	return AbstractUndoableEdit::CanUndo() && !m_inProgress;
}

bool CompoundEdit::CanRedo() const
{
	return AbstractUndoableEdit::CanRedo() && !m_inProgress;
}

void CompoundEdit::End()
{
	m_inProgress = false;
	EndImpl();
}

bool CompoundEdit::AddEditImpl(const IUndoableEditPtr& edit)
{
	assert(edit);
	if (!m_inProgress)
	{
		return false;
	}

	if (!m_edits.empty())
	{
		if (auto& lastEdit = m_edits.back();
			!lastEdit->AddEdit(edit))
		{
			if (edit->ReplaceEdit(lastEdit))
			{
				m_edits.pop_back();
			}
			m_edits.emplace_back(edit);
		}
	}
	else
	{
		m_edits.emplace_back(edit);
	}

	return true;
}

void CompoundEdit::DestroyImpl() noexcept
{
	for (auto& edit : std::views::reverse(m_edits))
	{
		edit->Destroy();
	}
	m_edits.clear();
}

void CompoundEdit::UndoImpl()
{
	for (auto& edit : std::views::reverse(m_edits))
	{
		edit->Undo();
	}
}

void CompoundEdit::RedoImpl()
{
	for (auto& edit : m_edits)
	{
		edit->Redo();
	}
}

} // namespace undo