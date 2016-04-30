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

using namespace std;

static string::const_iterator SkipWs(string::const_iterator it)
{
	while (*it == ' ' || *it == '\t')
		++it;
	return it;
}

static string::const_iterator ExtractName(string::const_iterator it, std::string &n)
{
	n.clear();

	if (!isalpha(*it))
	{
		// Unexpected token.
		throw it;
	}
	n.push_back(*(it++));

	while (isalnum(*it) || *it == '_' || *it == '-')
		n.push_back(*(it++));

	return it;
}

static string::const_iterator ExtractQuotedString(string::const_iterator it, std::string &arg)
{
	if (*it != '"')
		throw it;
	++it;

	while (*it)
	{
		// End of string
		if (*it == '"')
		{
			++it;
			return it;
		}

		if (*it == '\\')
		{
			++it;

			// Not accepting all escapes here, should we?
			switch (*it)
			{
				case '\\':
					arg.push_back('\\');
					++it;
					break;
				case '"':
					arg.push_back('"');
					++it;
					break;
				default:
					// Unknown escape
					throw it;
			}
		}

		arg.push_back(*(it++));
	}

	// String is not terminated until the end, so it should not be accepted.
	throw it;
}

static string::const_iterator ExtractUnquotedString(string::const_iterator it, std::string &arg)
{
	// Allowed characters are [a-zA-Z0-9_\-=:]
	if (!(isalnum(*it) || *it == '_' || *it == '-' || *it == '=' || *it == ':'))
	{
		// Unexpected token.
		throw it;
	}
	arg.push_back(*(it++));

	// TODO, should anything be accepted here?
	while (isalnum(*it) || *it == '_' || *it == '-' || *it == '=' || *it == ':' || *it == '"' || *it == '\\')
		arg.push_back(*(it++));

	return it;
}

static string::const_iterator ExtractArg(string::const_iterator it, std::string &arg)
{
	arg.clear();

	if (*it == '"')
		return ExtractQuotedString(it, arg);
	else
		return ExtractUnquotedString(it, arg);
}

// Registers the global `set` function, which handles
// commands for settings variables.
HexaScript::HexaScript()
{
	RegisterFunction<string>("set", [this](string s) { this->Set(s); });
}

// Handles expressions like `a=3` and sets the variables registered.
void HexaScript::Set(string expr)
try
{
	string name;
	string value;

	string::const_iterator it = expr.begin();

	it = ExtractName(it, name);

	if (*it != '=')
	{
		// Must be an option like `:set big-endian`
		std::unordered_map<std::string, OptionDef>::iterator opt = options.find(name);
		if (opt == options.end())
		{
			Error e;
			e.error_info = "No such option: " + name;
			throw e;
		}
		opt->second.setter_function();
		return;
	}
	++it;

	it = ExtractArg(it, value);

	// Set the var
	std::unordered_map<std::string, VariableDef>::iterator var = variables.find(name);
	if (var == variables.end())
	{
		Error e;
		e.error_info = "No such variable: " + name;
		throw e;
	}
	var->second.setter_function(value);
}
catch (string::const_iterator it)
{
	Error e;
	e.error_info = "HexaScript Error: Unexpected token\n"
	               "On expr: " + expr + "\n"
	               "         " + string(it - expr.begin(), ' ') + "^ here";
	throw e;
}

void HexaScript::ExecLine(const string &line)
try
{
	string fn_name;
	vector<string> args;

	string::const_iterator it = line.begin();

	it = SkipWs(it);
	if (*it == '\0') return;

	it = ExtractName(it, fn_name);

	// Extract the arguments
	while (1)
	{
		it = SkipWs(it);
		if (*it == '\0') break;

		string arg;
		it = ExtractArg(it, arg);
		args.push_back(arg);
	}

	CallFunction(fn_name, args);
}
catch (string::const_iterator it)
{
	Error e;
	e.error_info = "HexaScript Error: Unexpected token\n"
	               "On line: " + line + "\n"
	               "         " + string(it - line.begin(), ' ') + "^ here";
	throw e;
}
