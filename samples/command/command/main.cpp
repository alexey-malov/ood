#include "Menu.h"
#include "MenuFP.h"
#include "Commands.h"
	
using namespace std;

class CMenuHelpCommand : public ICommand
{
public:
	CMenuHelpCommand(const CMenu & menu)
		:m_menu(menu)
	{}

	void Execute() override
	{
		m_menu.ShowInstructions();
	}
private:
	const CMenu & m_menu;
};

class CExitMenuCommand : public ICommand
{
public:
	CExitMenuCommand(CMenu & menu)
		:m_menu(menu)
	{}

	void Execute() override
	{
		m_menu.Exit();
	}
private:
	CMenu & m_menu;
};

template <typename Commands>
CMenuFP::Command CreateMacroCommand(Commands && commands)
{
	return [=] {
		for (auto & command : commands)
		{
			command();
		}
	};
}

void TestMenuWithClassicCommandPattern()
{
	Robot robot;
	CMenu menu;
	menu.AddItem("on", "Turns the Robot on",
		make_unique<CTurnOnCommand>(robot));
	menu.AddItem("off", "Turns the Robot off",
		make_unique<CTurnOffCommand>(robot));

	menu.AddItem("north", "Makes the Robot walk north",
		make_unique<CWalkCommand>(robot, WalkDirection::North));
	menu.AddItem("south", "Makes the Robot walk south",
		make_unique<CWalkCommand>(robot, WalkDirection::South));
	menu.AddItem("west", "Makes the Robot walk west",
		make_unique<CWalkCommand>(robot, WalkDirection::West));
	menu.AddItem("east", "Makes the Robot walk east",
		make_unique<CWalkCommand>(robot, WalkDirection::East));
	auto cmd = make_unique<CMacroCommand>();
	cmd->AddCommand(make_unique<CTurnOnCommand>(robot));
	cmd->AddCommand(make_unique<CWalkCommand>(robot, WalkDirection::North));
	cmd->AddCommand(make_unique<CWalkCommand>(robot, WalkDirection::East));
	cmd->AddCommand(make_unique<CWalkCommand>(robot, WalkDirection::South));
	cmd->AddCommand(make_unique<CWalkCommand>(robot, WalkDirection::West));
	cmd->AddCommand(make_unique<CTurnOffCommand>(robot));
	menu.AddItem("patrol", "Patrol the territory", move(cmd));

	menu.AddItem("stop", "Stops the Robot",
		make_unique<CStopCommand>(robot));

	menu.AddItem("help", "Show instructions",
		make_unique<CMenuHelpCommand>(menu));
	menu.AddItem("exit", "Exit from this menu",
		make_unique<CExitMenuCommand>(menu));
	
	menu.Run();
}

void TestMenuWithFunctionalCommandPattern()
{
	Robot robot;
	CMenuFP menu;

	menu.AddItem("on", "Turns the Robot on", [&] { 
		robot.TurnOn(); 
	});
	menu.AddItem("off", "Turns the Robot off",
		bind(&Robot::TurnOff, &robot));

	menu.AddItem("north", "Makes the Robot walk north", 
		bind(&Robot::Walk, &robot, WalkDirection::North));
	menu.AddItem("south", "Makes the Robot walk south", 
		bind(&Robot::Walk, &robot, WalkDirection::South));
	menu.AddItem("west", "Makes the Robot walk west", 
		bind(&Robot::Walk, &robot, WalkDirection::West));
	menu.AddItem("east", "Makes the Robot walk east", 
		bind(&Robot::Walk, &robot, WalkDirection::East));

	menu.AddItem("stop", "Stops the Robot", 
		bind(&Robot::Stop, &robot));

	menu.AddItem("patrol", "Patrol the territory", CreateMacroCommand<vector<CMenuFP::Command>>({
			bind(&Robot::TurnOn, &robot),
			bind(&Robot::Walk, &robot, WalkDirection::North),
			bind(&Robot::Walk, &robot, WalkDirection::South),
			bind(&Robot::Walk, &robot, WalkDirection::West),
			bind(&Robot::Walk, &robot, WalkDirection::East),
			bind(&Robot::TurnOff, &robot)
		}));

	menu.AddItem("help", "Show instructions", 
		bind(&CMenuFP::ShowInstructions, &menu));
	menu.AddItem("exit", "Exit from this menu", 
		bind(&CMenuFP::Exit, &menu));

	menu.Run();
}

int main()
{
	CMenuFP menu;
	menu.AddItem("c", "Classic command pattern implementation", [&] {
		TestMenuWithClassicCommandPattern(); 
		menu.ShowInstructions();
	});
	menu.AddItem("f", "Functional command pattern implementation", [&] {
		TestMenuWithFunctionalCommandPattern(); 
		menu.ShowInstructions();
	});

	menu.AddItem("q", "Exit Program", bind(&CMenuFP::Exit, &menu));
	menu.ShowInstructions();
	menu.Run();
	
	return 0;
}