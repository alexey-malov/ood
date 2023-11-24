#include <cmath>
#include <iostream>

#include "drawer.h"

constexpr int STAR_COUNT = 20;

int main()
{
	{
		Image img = LoadImage(
			" CCCC             \n"
			"CC  CC   ##    ## \n"
			"CC      ####  ####\n"
			"CC  CC   ##    ## \n"
			" CCCC             \n");
		Print(img, std::cout);
	}

	{
		Image img{ { 30, 20 }, '.' };

		DrawLine(img, { 3, 2 }, { 26, 5 }, '#');
		DrawLine(img, { 26, 5 }, { 21, 18 }, '#');
		DrawLine(img, { 21, 18 }, { 3, 2 }, '#');

		Print(img, std::cout);
	}
}
