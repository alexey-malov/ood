#include <algorithm>
#include <functional>
#include <iostream>
#include <string>

using namespace std;

using Strategy = function<int(int x, int y)>;

class Context
{
public:
	int PerformOperation(int x, int y)
	{
		if (m_strategy)
		{
			return m_strategy(x, y);
		}
		return 0;
	}
	void SetStrategy(const Strategy& s)
	{
		m_strategy = s;
	}

private:
	Strategy m_strategy;
};

int AddingStrategy(int x, int y)
{
	return x + y;
}

int SubtractionStrategy(int x, int y)
{
	return x - y;
}

int main()
{
	{
		int numbers[] = { 6, 3, 2, 1, 3, 4 };
		sort(begin(numbers), end(numbers), [](int x, int y) { return x < y; });

		sort(begin(numbers), end(numbers), less<int>());

		function<bool(int, int)> compare = [](int x, int y) { return x < y; };
		sort(begin(numbers), end(numbers), compare);
	}
	{
		Context ctx;

		ctx.SetStrategy([](int x, int y) { return x + y; });
		cout << ctx.PerformOperation(1, 2) << endl;

		ctx.SetStrategy(AddingStrategy);
		cout << ctx.PerformOperation(1, 2) << endl;

		ctx.SetStrategy(SubtractionStrategy);
		cout << ctx.PerformOperation(1, 2) << endl;
	}
}
