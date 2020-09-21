#include <memory>

using namespace std;

struct IQuackBehavior
{
	virtual void Quack() = 0;
	virtual ~IQuackBehavior() {}
};

struct IFlyBehavior
{
	virtual void Fly() = 0;
	virtual ~IFlyBehavior() {}
};

class CDuck
{
public:
	void PerformQuack()
	{
		m_quackBehavior->Quack();
	}

	void PerformFly()
	{
		m_flyBehavior->Fly();
	}
protected:
	unique_ptr<IQuackBehavior> m_quackBehavior;
	unique_ptr<IFlyBehavior> m_flyBehavior;
};

class CQuack : public IQuackBehavior
{
public:
	void Quack() override {}
};

class CFlyWithWings : public IFlyBehavior
{
public:
	void Fly() override {}
};


class CMallardDuck : public CDuck
{
public:
	CMallardDuck()
	{
		m_quackBehavior = make_unique<CQuack>();
		m_flyBehavior = make_unique<CFlyWithWings>();
	}
};

int main()
{

}