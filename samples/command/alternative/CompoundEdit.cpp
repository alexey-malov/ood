#include "CompoundEdit.h"
#include <cassert>
#include <ranges>
#include <stdexcept>

namespace undo
{

CompoundEdit::CompoundEdit(const std::optional<std::string>& customName)
{
	if (customName)
	{
		SetName(*customName);
	}
}

CompoundEdit::~CompoundEdit()
{
	Destroy();
}

bool CompoundEdit::CanUndo() const
{
	return AbstractUndoableEdit::CanUndo() && !m_inProgress;
}

bool CompoundEdit::CanRedo() const
{
	return AbstractUndoableEdit::CanRedo() && !m_inProgress;
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
		auto lastEdit = m_edits.back();

		if (!lastEdit->AddEdit(edit))
		{
			if (edit->ReplaceEdit(lastEdit))
			{
				m_edits.pop_back();
			}
			m_edits.push_back(edit);
		}
	}
	else
	{
		m_edits.push_back(edit);
	}

	return true;
}

void CompoundEdit::EndImpl()
{
}

std::string CompoundEdit::GetName() const
{
	if (m_edits.empty() || m_hasCustomName)
	{
		return AbstractUndoableEdit::GetName();
	}
	return m_edits.front()->GetName();
}

void CompoundEdit::End()
{
	m_inProgress = false;
	EndImpl();
}

void CompoundEdit::DestroyImpl() noexcept
{
	for (auto& edit : m_edits | std::views::reverse)
	{
		edit->Destroy();
	}
	m_edits.clear();
}

void CompoundEdit::UndoImpl()
{
	for (auto& edit : m_edits | std::views::reverse)
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

void CompoundEdit::SetName(const std::string& name)
{
	AbstractUndoableEdit::SetName(name);
	m_hasCustomName = true;
}

size_t CompoundEdit::GetEditCount() const
{
	return m_edits.size();
}

IUndoableEditPtr CompoundEdit::GetEdit(size_t index) const
{
	if (index >= m_edits.size())
	{
		throw std::invalid_argument("Index is out of range");
	}

	return m_edits[index];
}

} // namespace undo
