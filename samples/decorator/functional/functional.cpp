/*
 * Декораторы в функциональном стиле
 */

#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <ranges> // Требует C++20
#include <vector>

using namespace std;

// Преобразователь строк (аналог интерфейса Component)
using StringTransformer = function<string(string)>;

// Преобразователь, который возвращает саму строку (аналог конкретного компонента)
string DontChange(string s)
{
	return move(s);
}

string ToUpperCase(string s)
{
	// transform(s.begin(), s.end(), s.begin(), [](unsigned char ch) { return toupper(ch); });
	ranges::transform(s, s.begin(), [](unsigned char ch) { return toupper(ch); });
	return move(s);
}

string AddSquareBrackets(string s)
{
	s.append("[]");

	rotate(s.begin(), s.end() - 2, s.end() - 1);

	return move(s);
}

string AddCurlyBrackets(string s)
{
	s.append("{}");

	rotate(s.begin(), s.end() - 2, s.end() - 1);

	return move(s);
}

void PrintTransformedStrings(const vector<string>& strings,
	const StringTransformer& transformer)
{
	for (const auto& s : strings)
	{
		cout << transformer(s) << ", ";
	}
	cout << "\n";
}

StringTransformer Decorate(StringTransformer t, StringTransformer d)
{
	return [t = move(t), d = move(d)](auto&& s) {
		return d(t(forward<decltype(s)>(s)));
	};
}

struct StringDecorator_T
{
};
constexpr StringDecorator_T Decor{};

StringTransformer operator|(StringDecorator_T t, StringTransformer d)
{
	return move(d);
}

StringTransformer operator|(StringTransformer t, StringTransformer d)
{
	return Decorate(t, d);
}

string UpperCaseWithSquareBrackets(string s)
{
	return AddSquareBrackets(ToUpperCase(move(s)));
}

string SquareBracketsWithinCurlyBrackets(string s)
{
	return AddCurlyBrackets(AddSquareBrackets(move(s)));
}

string CurlyBracketsWithinSquareBrackets(string s)
{
	return AddSquareBrackets(AddCurlyBrackets(move(s)));
}


int main()
{
	using namespace literals;

	auto strings = vector{ "I"s, "love"s, "C++"s };

	PrintTransformedStrings(strings, DontChange);
	PrintTransformedStrings(strings, ToUpperCase);
	PrintTransformedStrings(strings, AddCurlyBrackets);
	PrintTransformedStrings(strings, AddSquareBrackets);

	PrintTransformedStrings(strings, UpperCaseWithSquareBrackets);
	PrintTransformedStrings(strings, SquareBracketsWithinCurlyBrackets);
	PrintTransformedStrings(strings, CurlyBracketsWithinSquareBrackets);
	// ???? OMG !!!!


	PrintTransformedStrings(strings, Decorate(ToUpperCase, AddSquareBrackets));
	PrintTransformedStrings(strings, Decor | ToUpperCase | AddSquareBrackets | AddCurlyBrackets);
}
