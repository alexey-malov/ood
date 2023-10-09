#pragma once
#include <memory>

namespace undo
{

class IUndoableEdit;
using IUndoableEditPtr = std::shared_ptr<IUndoableEdit>;

} // namespace undo