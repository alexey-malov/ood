#pragma once
#include "IUndoableEdit.h"

namespace undo
{

class AbstractUndoableEdit : public IUndoableEdit
{
public:
	bool CanUndo() const override;

	bool CanRedo() const override;

	void Undo() final;

	void Redo() final;

	void Destroy() noexcept final;

	bool AddEdit(const IUndoableEditPtr& edit) final;

	bool ReplaceEdit(const IUndoableEditPtr& edit) override;

	std::string GetName() const override;

private:
	virtual void UndoImpl() {}
	virtual void RedoImpl() {}
	virtual void DestroyImpl() noexcept {}
	virtual bool AddEditImpl([[maybe_unused]] const IUndoableEditPtr& edit) { return false; }
	virtual bool ReplaceEditImpl([[maybe_unused]] const IUndoableEditPtr& edit) { return false; }

	std::string m_name;
	bool m_isAlive = true;
	bool m_executed = false;
};

} // namespace undo