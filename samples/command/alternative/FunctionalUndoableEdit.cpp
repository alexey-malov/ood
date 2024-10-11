#include "FunctionalUndoableEdit.h"
#include <cassert>

namespace undo
{

FunctionalUndoableEdit::FunctionalUndoableEdit(std::string const& name, const RedoFn& redo, const UndoFn& undo, const DestroyFn& destroy)
	: FunctionalUndoableEdit(name, redo, undo, DestroyFn1([=](bool) {
		destroy();
	}))
{
}

FunctionalUndoableEdit::FunctionalUndoableEdit(std::string const& name, const RedoFn& redo, const UndoFn& undo, const DestroyFn1& destroy)
	: m_undo(undo)
	, m_redo(redo)
	, m_destroy(destroy)
{
	SetName(name);
	assert(redo);
	assert(undo);
	assert(destroy);
	redo();
}

FunctionalUndoableEdit::~FunctionalUndoableEdit() noexcept
{
	Destroy();
}

void FunctionalUndoableEdit::UndoImpl()
{
	m_undo();
}

void FunctionalUndoableEdit::RedoImpl()
{
	m_redo();
}

void FunctionalUndoableEdit::DestroyImpl() noexcept
{
	try
	{
		m_destroy(IsExecuted());
	}
	catch (...)
	{ //-V565
		// Unknown exception
		assert(false);
	}
}

} // namespace undo
