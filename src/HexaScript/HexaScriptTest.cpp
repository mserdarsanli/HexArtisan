// Copyright 2016 Mustafa Serdar Sanli
//
// This file is part of HexArtisan.
//
// HexArtisan is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// HexArtisan is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HexArtisan.  If not, see <http://www.gnu.org/licenses/>.

#include "HexaScript.hpp"

#include <iostream>
#include <vector>
using namespace std;

#define EXPECT(a) {\
  std::cerr << ((a) ? "\033[32m[PASS]\033[0m" :"\033[31m[FAIL]\033[0m")  \
            << " at line " << __LINE__ \
            << " expr (" << #a << ")" << std::endl; }

// Since return values are not supported, global values are used here to
// see if the functions are working as expected.
int last_factorial = -1;
int last_multiplication = -1;

void factorial(int a)
{
	last_factorial = 1;
	for (int i = 1; i <= a; ++i)
	{
		last_factorial *= i;
	}
}

void multiply(int a, int b)
{
	last_multiplication = a * b;
}

int main()
{
	HexaScript hs;

	hs.RegisterFunction<int, int>("multiply", &multiply);
	hs.RegisterFunction<int>("factorial", &factorial);

	last_multiplication = -1;
	hs.CallFunction("multiply", {"4", "7"});
	EXPECT(last_multiplication == 28);

	last_factorial = -1;
	hs.CallFunction("factorial", {"5"});
	EXPECT(last_factorial == 120);

	last_multiplication = -1;
	hs.ExecLine("multiply 3 99");
	EXPECT(last_multiplication == 297);

	return 0;
}
