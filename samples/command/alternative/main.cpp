// alternative.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "AbstractUndoableEdit.h"
#include <iostream>

class EditText : public undo::AbstractUndoableEdit
{
public:
	EditText(std::string& text, std::string newText)
		: m_text(text)
		, m_oldText(text)
		, m_newText(std::move(newText))
	{
	}

private:
	void UndoImpl() override
	{
		m_text = m_oldText;
	}
	void RedoImpl()
	{
		m_text = m_newText;
	}

	bool ReplaceEditImpl(const undo::IUndoableEditPtr& edit)
	{
		if (auto otherEditText = std::dynamic_pointer_cast<EditText>(edit);
			otherEditText
			&& &otherEditText->m_text == &m_text
			&& otherEditText->m_oldText == m_newText)
		{
			m_newText = std::move(otherEditText->m_newText);
			return true;
		}
		return false;
	}

	std::string& m_text;
	std::string m_newText;
	std::string m_oldText;
};

int main()
{
	EditText e;
}
