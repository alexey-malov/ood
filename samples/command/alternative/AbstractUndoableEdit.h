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
	bool ReplaceEdit(const IUndoableEditPtr& edit) final;
	std::string GetName() const override;
	void SetReplaceable(bool canBeReplaced);

protected:
	virtual void UndoImpl();
	virtual void RedoImpl();
	virtual void DestroyImpl() noexcept;
	virtual bool AddEditImpl(const IUndoableEditPtr& edit);
	virtual bool ReplaceEditImpl(const IUndoableEditPtr& edit);
	virtual bool IsExecuted() const;
	virtual void SetExecuted(bool executed);
	virtual void SetName(const std::string& name);

private:
	bool m_executed = true;
	bool m_isAlive = true;
	bool m_canBeReplaced = true;
	std::string m_name;
};

} // namespace undo