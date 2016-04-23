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
		const std::string& ErrorInfo() const;
	private:
		Error() = default;
		Error(Error &ot) = default;
		std::string error_info;

		friend class HexaScript;
	};

	HexaScript() = default;
	HexaScript(const HexaScript &ot) = delete;
	HexaScript& operator=(const HexaScript &ot) = delete;


	// TODO merge RegisterFunction functions into one variadic function if possible?

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

	void ExecLine(const std::string &line)
	{
		std::string fn_name;
		std::vector<std::string> args;

		std::string::const_iterator it = line.begin();

		// Temporary shitty parser with no error checks etc.
		// TODO FIXME replace this with boost regex (with BOOST_REGEX_MATCH_EXTRA)
		// to capture repeating groups.
		while (*it == ' ')
		{
			++it;
		}
		while (isalnum(*it))
		{
			fn_name.push_back(*it);
			++it;
		}
		while (*it)
		{
			while (*it == ' ')
			{
				++it;
			}
			std::string arg;
			while (isalnum(*it))
			{
				arg.push_back(*it);
				++it;
			}
			if (arg.size() > 0)
			{
				args.push_back(arg);
			}
			else
			{
				break;
			}
		}

		CallFunction(fn_name, args);
	}

private:

	struct FunctionDef
	{
		int num_args;
		std::function<void(std::vector<std::string>)> stringified_function;
	};

	std::unordered_map<std::string, FunctionDef> functions;
};
