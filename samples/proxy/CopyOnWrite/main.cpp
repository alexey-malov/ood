#include "CoW.h"
#include <iostream>

using namespace std;

struct Logger
{
	Logger()
	{
		cout << "New Logger was created" << endl;
	}
	Logger(Logger const& logger)
	{
		cout << "Logger was copied" << endl;
	}

	int data = 42;
};

struct ISomething
{
	virtual int GetData()const = 0;
	virtual void SetData(int newData) = 0;
	virtual unique_ptr<ISomething> Clone()const = 0;
	virtual ~ISomething() = default;
};

struct CSomething : public ISomething
{
	void DoSomethingElse()
	{
		cout << "something else" << endl;
	}

	void SetData(int newData) override
	{
		data = newData;
	}

	virtual int GetData() const override
	{
		return data;
	}

	unique_ptr<ISomething> Clone() const override
	{
		return make_unique<CSomething>(*this);
	}
private:
	int data = 0;

};


using namespace std::literals;

int main()
{
	CoW<std::string> s1("Hello");
	CoW<std::string> s2{ s1 };
	// Для модификации значения нужно разыменовать результат вызова Write()
	*s2.Write() = "Wor";
	*s2.Write() += "ld";
	// Можно вызывать неконстантные методы, используя ->
	s2.Write()->append("!");
	
	s2--->append("!");

	auto writer = s2.Write();
	//writer->append("!!!"); // А вот так не скомпилируется
}
