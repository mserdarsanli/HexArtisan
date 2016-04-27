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

#define PASS(a) {\
  std::cerr << "\033[32m[PASS]\033[0m" \
            << " at line " << __LINE__ \
            << " for test (" << #a << ")" << std::endl; }

#define FAIL(a) {\
  std::cerr << "\033[31m[FAIL]\033[0m" \
            << " at line " << __LINE__ \
            << " for test (" << #a << ")" << std::endl; }

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

void noop_0()
{
}

void noop_3(string a, string b, string c)
{
}

string last_arg;
void arg_copy(string s)
{
	last_arg = s;
}

int last_arg_cnt = -1;
string last_arg_1;
string last_arg_2;

void fn_overload_1(string arg1)
{
	last_arg_cnt = 1;
	last_arg_1 = arg1;
	last_arg_2 = "__NONE__";
}

void fn_overload_2(string arg1, string arg2)
{
	last_arg_cnt = 2;
	last_arg_1 = arg1;
	last_arg_2 = arg2;
}

int main()
{
	HexaScript hs;

	hs.RegisterFunction<int, int>("multiply", &multiply);
	hs.RegisterFunction<int>("factorial", &factorial);
	hs.RegisterFunction("noop_0", &noop_0);
	hs.RegisterFunction<string, string, string>("noop_3", &noop_3);
	hs.RegisterFunction<string>("arg_copy", &arg_copy);

	last_multiplication = -1;
	hs.CallFunction("multiply", {"4", "7"});
	EXPECT(last_multiplication == 28);

	last_factorial = -1;
	hs.CallFunction("factorial", {"5"});
	EXPECT(last_factorial == 120);

	last_multiplication = -1;
	hs.ExecLine("multiply 3 99");
	EXPECT(last_multiplication == 297);

	try
	{
		hs.ExecLine("    \t\t  ");
		PASS("Empty line test");
	}
	catch (HexaScript::Error &e)
	{
		FAIL("Empty line test");
	}

	try
	{
		hs.ExecLine("  _");
		FAIL("Invalid name test");
	}
	catch (HexaScript::Error &e)
	{
		cerr << e.ErrorInfo() << endl;
		PASS("Invalid name test");
	}

	try
	{
		hs.ExecLine(" noop_0 ");
		PASS("fn call no args");
	}
	catch (HexaScript::Error &e)
	{
		cerr << e.ErrorInfo() << endl;
		FAIL("fn call no args");
	}

	try
	{
		hs.ExecLine(" noop_3 arg=1 ar_g2 arg-3 ");
		PASS("fn call with args");
	}
	catch (HexaScript::Error &e)
	{
		cerr << e.ErrorInfo() << endl;
		FAIL("fn call with args");
	}

	try
	{
		hs.ExecLine(" noop_3 \"ewqewqeqw");
		FAIL("non terminating string");
	}
	catch (HexaScript::Error &e)
	{
		cerr << e.ErrorInfo() << endl;
		PASS("non terminating string");
	}

	try
	{
		hs.ExecLine(" noop_3 \"ewqe\\qwqeqw\" ");
		FAIL("unknown escape");
	}
	catch (HexaScript::Error &e)
	{
		cerr << e.ErrorInfo() << endl;
		PASS("unknown escape");
	}

	// Test unquoted arg
	hs.ExecLine("arg_copy wqewqewq");
	EXPECT(last_arg == "wqewqewq");

	// Test quoted arg
	hs.ExecLine("arg_copy \"wqewqewq\"");
	EXPECT(last_arg == "wqewqewq");

	// Test arg with escapes
	hs.ExecLine("arg_copy \"wqe\\\"wqe\\\\wq\"");
	EXPECT(last_arg == "wqe\"wqe\\wq");

	// Register some variables
	bool var_bool = false;
	hs.RegisterVariable<bool>("var_bool", [&var_bool](bool v) { var_bool = v; });

	int var_int = -1;
	hs.RegisterVariable<int>("var_int", [&var_int](int v) { var_int = v; });

	string var_string;
	hs.RegisterVariable<string>("var_string", [&var_string](string v) { var_string = v; });

	// Set the variables them
	hs.ExecLine("set var_bool=true");
	EXPECT(var_bool == true);
	hs.ExecLine("set var_bool=\"false\"");
	EXPECT(var_bool == false);

	hs.ExecLine("set var_int=100");
	EXPECT(var_int == 100);
	hs.ExecLine("set var_int=\"-11111\"");
	EXPECT(var_int == -11111);

	hs.ExecLine("set var_string=hello");
	EXPECT(var_string == "hello");
	hs.ExecLine("set var_string=\"hehe\\\"weeqw\\\\weqewq\"");
	EXPECT(var_string == "hehe\"weeqw\\weqewq");

	// Test variable with dashes in name
	string var_string_2;
	hs.RegisterVariable<string>("var-string-2", [&var_string_2](string v) { var_string_2 = v; });

	hs.ExecLine("set var-string-2=hello");
	EXPECT(var_string_2 == "hello");
	hs.ExecLine("set var-string-2=\"hehe\\\"weeqw\\\\weqewq\"");
	EXPECT(var_string_2 == "hehe\"weeqw\\weqewq");

	// Try setting unknown variable
	try
	{
		hs.ExecLine("set weqweqweq=123");
		FAIL("unknown variable");
	}
	catch (HexaScript::Error &e)
	{
		cerr << e.ErrorInfo() << endl;
		PASS("unknown variable");
	}

	// Test options (like variables, but with no value)
	string endianness = "default";
	hs.RegisterOption("big-endian", [&endianness]() { endianness = "BE"; });
	hs.RegisterOption("little-endian", [&endianness]() { endianness = "LE"; });

	EXPECT(endianness == "default");
	hs.ExecLine("set big-endian");
	EXPECT(endianness == "BE");
	hs.ExecLine("set little-endian");
	EXPECT(endianness == "LE");

	// Try setting unknown option
	try
	{
		hs.ExecLine("set unregistered-option");
		FAIL("unknown option");
	}
	catch (HexaScript::Error &e)
	{
		cerr << e.ErrorInfo() << endl;
		PASS("unknown option");
	}

	// Test function overloads (different arg counts)
	hs.RegisterFunction<string>("overloaded_fn", &fn_overload_1);
	hs.RegisterFunction<string, string>("overloaded_fn", &fn_overload_2);

	EXPECT(last_arg_cnt == -1);
	hs.ExecLine("overloaded_fn qwe");
	EXPECT(last_arg_cnt == 1 && last_arg_1 == "qwe" && last_arg_2 == "__NONE__");
	hs.ExecLine("overloaded_fn foo bar");
	EXPECT(last_arg_cnt == 2 && last_arg_1 == "foo" && last_arg_2 == "bar");

	// TODO test value cast errors

	return 0;
}
