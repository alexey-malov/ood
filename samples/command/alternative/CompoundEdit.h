#pragma once
#include "AbstractUndoableEdit.h"
#include <vector>

namespace undo
{

class CompoundEdit : public AbstractUndoableEdit
{
public:
	bool CanUndo() const override;

	bool CanRedo() const override;

	void End();

private:
	bool AddEditImpl(const IUndoableEditPtr& edit) override;
	void DestroyImpl() noexcept override;
	void UndoImpl() override;
	void RedoImpl() override;

	virtual void EndImpl() {}

	std::vector<IUndoableEditPtr> m_edits;
	bool m_inProgress = true;
};

} // namespace undo
