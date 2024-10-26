#include <concepts>
#include <iostream>
#include <memory>

using namespace std;

namespace turkey_to_duck
{
struct IDuck
{
	virtual void Quack() = 0;
	virtual void Fly() = 0;
	virtual ~IDuck() = default;
};

class MallardDuck : public IDuck
{
public:
	void Quack() override
	{
		cout << "Quack\n";
	}
	void Fly() override
	{
		cout << "I'm flying\n";
	}
};

void TestDuck(IDuck& duck)
{
	duck.Quack();
	duck.Fly();
}

struct ITurkey
{
	virtual void Gobble() = 0;
	virtual void Fly() = 0;
	virtual ~ITurkey() = default;
};

class WildTurkey : public ITurkey
{
public:
	void Gobble() override
	{
		cout << "Gobble gobble\n";
	}
	void Fly() override
	{
		cout << "I'm flying a short distance\n";
	}
};

class TurkeyToDuckAdapter : public IDuck
{
public:
	TurkeyToDuckAdapter(ITurkey& turkey)
		: m_turkey(turkey)
	{
	}

	void Quack() override
	{
		m_turkey.Gobble();
	}
	void Fly() override
	{
		for (int i = 0; i < 5; ++i)
		{
			m_turkey.Fly();
		}
	}

private:
	ITurkey& m_turkey;
};

void Test()
{
	MallardDuck mallardDuck;
	TestDuck(mallardDuck);

	WildTurkey wildTurkey;
	TurkeyToDuckAdapter turkeyAdapter(wildTurkey);
	TestDuck(turkeyAdapter);
}

} // namespace turkey_to_duck

namespace template_adapter
{

class Target
{
public:
	virtual void Request() = 0;
	virtual ~Target() = default;
};

class Adaptee
{
public:
	void SpecificRequest()
	{
		Operation1();
		Operation2();
		Operation3();
	}

private:
	virtual void Operation1() = 0;
	void Operation2()
	{
		std::cout << "Adaptee::Operation2()\n";
	}
	virtual void Operation3()
	{
		std::cout << "Adaptee::Operation2()\n";
	}
};

class ConcreteAdaptee : public Adaptee
{
private:
	void Operation1() override
	{
		cout << "ConcreteAdaptee::Operation1\n";
	}
	void Operation3() override
	{
		std::cout << "ConcreteAdaptee::Operation2()\n";
	}
};

template <typename AdapteeType = Adaptee>
	requires(std::derived_from<AdapteeType, Adaptee>)
class AdapteeToTargetAdapter : public Target
	, private AdapteeType
{
public:
	void Request() override
	{
		this->SpecificRequest();
	}

private:
	void Operation1() override // Переопределяем операцию конкретного класса
	{
		cout << "AdapteeToTargetAdapter::Operation1\n";
	}
};

void TestClient(Target& target)
{
	target.Request();
}

void Test()
{
	AdapteeToTargetAdapter<> adapter1;
	TestClient(adapter1);

	AdapteeToTargetAdapter<ConcreteAdaptee> adapter2;
	TestClient(adapter2);
}

} // namespace template_adapter

int main()
{
	turkey_to_duck::Test();
	template_adapter::Test();

	return 0;
}