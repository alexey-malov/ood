#pragma once
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <algorithm>

#include "ICommand.h"


class CMenu
{
public:
	void AddItem(
		const std::string & shortcut,
		const std::string & description,
		std::unique_ptr<ICommand> && command)
	{
		m_items.emplace_back(shortcut, description, std::move(command));
	}

	void Run()
	{
		ShowInstructions();

		std::string command;
		while ((std::cout << ">")
			&& getline(std::cin, command)
			&& ExecuteCommand(command))
		{
		}
	}

	void ShowInstructions()const
	{
		std::cout << "Commands list:\n";
		for (auto & item : m_items)
		{
			std::cout << "  " << item.shortcut << ": " << item.description << "\n";
		}
	}

	void Exit()
	{
		m_exit = true;
	}
private:
	bool ExecuteCommand(const std::string & command)
	{
		m_exit = false;

		auto it = std::find_if(m_items.begin(), m_items.end(), [&](const Item & item) {
			return item.shortcut == command;
		});
		if (it != m_items.end())
		{
			it->command->Execute();
		}
		else
		{
			std::cout << "Unknown command\n";
		}

		return !m_exit;
	}

	struct Item
	{
		Item(const std::string & shortcut, const std::string & description, std::unique_ptr<ICommand> && command)
			: shortcut(shortcut)
			, description(description)
			, command(std::move(command))
		{}

		std::string shortcut;
		std::string description;
		std::unique_ptr<ICommand> command;
	};
	std::vector<Item> m_items;
	bool m_exit = false;
};