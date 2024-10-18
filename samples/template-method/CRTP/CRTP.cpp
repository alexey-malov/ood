// CRTP.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

using namespace std;

template <typename Derived>
class CaffeineBeverage
{
public:
	virtual ~CaffeineBeverage() = default;

	void PrepareRecipe()
	{
		BoilWater();
		AsDerived().Brew();
		PourInCup();
		AsDerived().AddCondiments();
	}

private:
	const Derived& AsDerived() const noexcept { return static_cast<const Derived&>(*this); }
	Derived& AsDerived() noexcept { return static_cast<Derived&>(*this); }

	void BoilWater()
	{
		cout << "Boiling water" << endl;
	}

	void PourInCup()
	{
		cout << "Pouring into cup" << endl;
	}
};

class Coffee : public CaffeineBeverage<Coffee>
{
	friend class CaffeineBeverage<Coffee>;

private:
	void Brew()
	{
		cout << "Dripping Coffee through filter" << endl;
	}

	void AddCondiments()
	{
		cout << "Adding sugar and milk" << endl;
	}
};

class Tea : public CaffeineBeverage<Tea>
{
	friend class CaffeineBeverage<Tea>;

private:
	void Brew()
	{
		cout << "Steeping the tea" << endl;
	}

	void AddCondiments()
	{
		cout << "Adding Lemon" << endl;
	}
};

int main()
{
	Tea tea;
	tea.PrepareRecipe();
	Coffee coffee;
	coffee.PrepareRecipe();
}
