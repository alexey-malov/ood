#include <iostream>
#include <string>

using namespace std;

namespace basic
{

class Coffee
{
public:
	void PrepareRecipe()
	{
		BoilWater();
		BrewCoffeeGrinds();
		PourInCup();
		AddSugarAndMilk();
	}

private:
	void BoilWater()
	{
		cout << "Boiling water" << endl;
	}

	void BrewCoffeeGrinds()
	{
		cout << "Dripping Coffee through filter" << endl;
	}

	void PourInCup()
	{
		cout << "Pouring into cup" << endl;
	}

	void AddSugarAndMilk()
	{
		cout << "Adding sugar and milk" << endl;
	}
};

class Tea
{
public:
	void PrepareRecipe()
	{
		BoilWater();
		SteepTeaBag();
		PourInCup();
		AddLemon();
	}

private:
	void BoilWater()
	{
		cout << "Boiling water" << endl;
	}

	void SteepTeaBag()
	{
		cout << "Steeping the tea" << endl;
	}

	void PourInCup()
	{
		cout << "Pouring into cup" << endl;
	}

	void AddLemon()
	{
		cout << "Adding Lemon" << endl;
	}
};

} // namespace basic

namespace naive
{

class CaffeineBeverage
{
public:
	virtual void PrepareRecipe() = 0;
	virtual ~CaffeineBeverage() = default;

protected:
	void BoilWater()
	{
		cout << "Boiling water" << endl;
	}

	void PourInCup()
	{
		cout << "Pouring into cup" << endl;
	}
};

class Coffee : public CaffeineBeverage
{
public:
	void PrepareRecipe() override
	{
		BoilWater();
		BrewCoffeeGrinds();
		PourInCup();
		AddSugarAndMilk();
	}

private:
	void BrewCoffeeGrinds()
	{
		cout << "Dripping Coffee through filter" << endl;
	}

	void AddSugarAndMilk()
	{
		cout << "Adding sugar and milk" << endl;
	}
};

class Tea : public CaffeineBeverage
{
public:
	void PrepareRecipe() override
	{
		BoilWater();
		SteepTeaBag();
		PourInCup();
		AddLemon();
	}

private:
	void SteepTeaBag()
	{
		cout << "Steeping the tea" << endl;
	}

	void AddLemon()
	{
		cout << "Adding Lemon" << endl;
	}
};

} // namespace naive

namespace template_method
{

class CaffeineBeverage
{
public:
	virtual ~CaffeineBeverage() = default;

	void PrepareRecipe()
	{
		BoilWater();
		Brew();
		PourInCup();
		AddCondiments();
	}

private:
	void BoilWater()
	{
		cout << "Boiling water" << endl;
	}

	void PourInCup()
	{
		cout << "Pouring into cup" << endl;
	}

	virtual void Brew() = 0;
	virtual void AddCondiments() = 0;
};

class Coffee : public CaffeineBeverage
{
private:
	void Brew() override
	{
		cout << "Dripping Coffee through filter" << endl;
	}

	void AddCondiments() override
	{
		cout << "Adding sugar and milk" << endl;
	}
};

class Tea : public CaffeineBeverage
{
private:
	void Brew() override
	{
		cout << "Steeping the tea" << endl;
	}

	void AddCondiments() override
	{
		cout << "Adding Lemon" << endl;
	}
};

} // namespace template_method

namespace template_method_structure
{

class CAbstractClass
{
public:
	void TemplateMethod()
	{
		PrimitiveOperation1();
		PrimitiveOperation2();
		ConcreteOperation1();
		ConcreteOperation2();
		Hook();
	}

	virtual void PrimitiveOperation1() = 0;
	virtual void PrimitiveOperation2() = 0;

	virtual void ConcreteOperation1()
	{
		// реализация
	}

	void ConcreteOperation2()
	{
		// Реализация
	}

	virtual void Hook() {}
};

} // namespace template_method_structure

namespace hooks
{

class CaffeineBeverageWithHook
{
public:
	virtual ~CaffeineBeverageWithHook() = default;
	void PrepareRecipe()
	{
		BoilWater();
		Brew();
		PourInCup();
		if (CustomerWantsCondiments())
		{
			AddCondiments();
		}
	}

private:
	void BoilWater()
	{
		cout << "Boiling water" << endl;
	}

	void PourInCup()
	{
		cout << "Pouring into cup" << endl;
	}

	virtual bool CustomerWantsCondiments()
	{
		return true;
	}

	virtual void Brew() = 0;
	virtual void AddCondiments() = 0;
};

class CoffeeWithHook : public CaffeineBeverageWithHook
{
private:
	void Brew() override
	{
		cout << "Dripping Coffee through filter" << endl;
	}
	void AddCondiments() override
	{
		cout << "Adding sugar and milk" << endl;
	}

	bool CustomerWantsCondiments() override
	{
		cout << "Would you like milk and sugar with your coffee (y/n)? ";
		string userInput;
		return getline(cin, userInput)
			&& (userInput == "y" || userInput == "Y");
	}
};

void Test()
{
	CoffeeWithHook coffeeHook;
	coffeeHook.PrepareRecipe();
}

} // namespace hooks

void main()
{
	hooks::Test();
}
