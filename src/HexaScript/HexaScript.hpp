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

#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace _hexascript_helper
{
template <typename T>
struct from_string_trait;

template <>
struct from_string_trait<bool>
{
	static bool fn(const std::string &s)
	{
		if (s == "true") return true;
		if (s == "false") return false;
		return false; // THROW HERE
	}
};

template <>
struct from_string_trait<int>
{
	static int fn(const std::string &s)
	{
		return std::stoi(s);
	}
};

template <>
struct from_string_trait<std::string>
{
	static std::string fn(const std::string &s)
	{
		return s;
	}
};
}

class HexaScript
{
public:
	// Exception type containing errors occured during script execution.
	struct Error
	{
	public:
		const std::string& ErrorInfo() const
		{
			return error_info;
		}
	private:
		Error() = default;
		Error(Error &ot) = default;
		std::string error_info;

		friend class HexaScript;
	};

	HexaScript();
	HexaScript(const HexaScript &ot) = delete;
	HexaScript& operator=(const HexaScript &ot) = delete;


	// TODO merge RegisterFunction functions into one variadic function if possible?

	void RegisterFunction(const std::string &fn_name, std::function<void(void)> fn)
	{
		FunctionDef f;
		f.num_args = 0;
		f.stringified_function = [fn](const std::vector<std::string> &args)
		{
			return fn();
		};
		functions[fn_name] = f;
	}

	template <typename Arg1>
	void RegisterFunction(const std::string &fn_name, std::function<void(Arg1)> fn)
	{
		FunctionDef f;
		f.num_args = 1;
		f.stringified_function = [fn](const std::vector<std::string> &args)
		{
			return fn(::_hexascript_helper::from_string_trait<Arg1>::fn(args[0]));
		};
		functions[fn_name] = f;
	}

	template <typename Arg1, typename Arg2>
	void RegisterFunction(const std::string &fn_name, std::function<void(Arg1, Arg2)> fn)
	{
		FunctionDef f;
		f.num_args = 2;
		f.stringified_function = [fn](const std::vector<std::string> &args)
		{
			return fn(::_hexascript_helper::from_string_trait<Arg1>::fn(args[0]),
			          ::_hexascript_helper::from_string_trait<Arg2>::fn(args[1]));
		};
		functions[fn_name] = f;
	}

	template <typename Arg1, typename Arg2, typename Arg3>
	void RegisterFunction(const std::string &fn_name, std::function<void(Arg1, Arg2, Arg3)> fn)
	{
		FunctionDef f;
		f.num_args = 3;
		f.stringified_function = [fn](const std::vector<std::string> &args)
		{
			return fn(::_hexascript_helper::from_string_trait<Arg1>::fn(args[0]),
			          ::_hexascript_helper::from_string_trait<Arg2>::fn(args[1]),
			          ::_hexascript_helper::from_string_trait<Arg3>::fn(args[2]));
		};
		functions[fn_name] = f;
	}

	void CallFunction(const std::string &fn_name, const std::vector<std::string> &args)
	{
		std::unordered_map<std::string, FunctionDef>::iterator fn = functions.find(fn_name);
		if (fn == functions.end())
		{
			Error e;
			e.error_info = "No such function";
			throw e;
		}

		fn->second.stringified_function(args);
	}

	template <typename T>
	void RegisterVariable(const std::string &var_name, std::function<void(T)> setter_fn)
	{
		VariableDef v;
		v.setter_function = [setter_fn](std::string s)
		{
			setter_fn(::_hexascript_helper::from_string_trait<T>::fn(s));
		};
		variables[var_name] = v;
	}

	void ExecLine(const std::string &line);

private:
	void Set(std::string expr);

	struct FunctionDef
	{
		int num_args;
		std::function<void(std::vector<std::string>)> stringified_function;
	};

	struct VariableDef
	{
		std::function<void(std::string s)> setter_function;
	};

	std::unordered_map<std::string, FunctionDef> functions;
	std::unordered_map<std::string, VariableDef> variables;
};
