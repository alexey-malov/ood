#pragma once
#include "AbstractUndoableEdit.h"
#include <functional>

namespace undo
{

class FunctionalUndoableEdit final : public AbstractUndoableEdit
{
public:
	using UndoFn = std::function<void()>;
	using RedoFn = std::function<void()>;
	using DestroyFn = std::function<void()>;
	using DestroyFn1 = std::function<void(bool executed)>;

	FunctionalUndoableEdit(
		std::string const& name, const RedoFn& redo, const UndoFn& undo, const DestroyFn& destroy = [] {});
	FunctionalUndoableEdit(std::string const& name, const RedoFn& redo, const UndoFn& undo, const DestroyFn1& destroy);
	~FunctionalUndoableEdit() noexcept;

private:
	void UndoImpl() final;
	void RedoImpl() final;
	void DestroyImpl() noexcept final;

private:
	UndoFn m_undo;
	RedoFn m_redo;
	DestroyFn1 m_destroy;
};

} // namespace undo
