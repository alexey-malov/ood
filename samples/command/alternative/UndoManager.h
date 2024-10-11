#pragma once
#include "CompoundEdit.h"

namespace undo
{

class UndoManager : public CompoundEdit
{
public:
	bool CanUndo() const final;
	bool CanRedo() const final;

	size_t GetCurrentEditIndex() const;

	void Clear();

private:
	bool AddEditImpl(const IUndoableEditPtr& edit) final;
	void UndoImpl() final;
	void RedoImpl() final;
	void EndImpl() final;
	void SetExecuted(bool executed) final;
	void DestroyImpl() throw() final;

	IUndoableEditPtr GetEditToBeUndone() const;
	IUndoableEditPtr GetEditToBeRedone() const;
	void UndoTo(const IUndoableEditPtr& edit);
	void RedoTo(const IUndoableEditPtr& edit);
	void TrimEdits(size_t from, size_t to);

private:
	size_t m_indexOfNextAdd = 0;
};

} // namespace undo