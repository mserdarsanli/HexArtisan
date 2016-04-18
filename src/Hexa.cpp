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

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

#include <boost/regex.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Hexa.hpp"

using namespace std;
using namespace boost;

Hexa::Hexa(const gengetopt_args_info &args)
  : args(args)
{
	if (args.column_count_given)
	{
		if (args.column_count_arg > 0)
		{
			// TODO rename column_count arg
			base_style_sheet.SetMaxEditorCols(args.column_count_arg);
		}
	}
}

void Hexa::LoadScriptFile(const string &file_name)
{
	ifstream script(file_name);

	if (!script.is_open())
	{
		SetStatus(StatusType::ERROR, "Unable to load \"" + file_name + "\"");
		return;
	}

	// Each line is a command.
	string command;
	while (getline(script, command))
	{
		ProcessCommand(command);
	}
}

vector<uint8_t> ReadFile(const string &fileName)
{
	ifstream testFile(fileName, std::ios::binary);

	vector<uint8_t> fileContents;
	fileContents.assign(std::istreambuf_iterator<char>(testFile),
	                    std::istreambuf_iterator<char>());
	return fileContents;
}


void Hexa::AddNewTab(const string &file_name)
{
	if (file_contents.find(file_name) == file_contents.end())
	{
		// TODO handle file not existing?
		file_contents[file_name] = ReadFile(file_name);
	}

	TabInfo ti {file_name, {this, file_name, &(file_contents[file_name])}};
	ti.editor.style_sheet = base_style_sheet;
	tabs.push_back(ti);

	// TODO switch to new tab

	SetStatus(StatusType::NORMAL, "\"" + file_name + "\" opened");
}

void Hexa::InputKey(Key k)
{
	if (input_key_handler)
	{
		(this->*input_key_handler)(command_key, k);
		return;
	}

	if (entering_command)
	{
		if (k == Key::ENTER)
		{
			ProcessCommand(command_buffer);
			entering_command = false;
			command_buffer.clear();
		}
		else if (k == Key::ESCAPE)
		{
			entering_command = false;
			command_buffer.clear();
		}
		else if (k == Key::BACKSPACE)
		{
			if (command_buffer.size() > 0)
			{
				command_buffer.pop_back();
			}
			else
			{
				entering_command = false;
				command_buffer.clear();
			}
		}
		else
		{
			// TODO filter out special characters here.
			command_buffer.push_back((char) k);
		}
		return;
	}

	// Cursor movement logic, common for normal mode and visual mode
	// Returns if input key isfor cursor movement, continues otherwise
	if (mode == EditorMode::Normal || mode == EditorMode::Visual)
	{
		switch (k)
		{
			case Key::ARROW_DOWN:
			case Key::LOWERCASE_J:
				GetCurrentEditor()->MoveCursorDown();
				return;
			case Key::ARROW_UP:
			case Key::LOWERCASE_K:
				GetCurrentEditor()->MoveCursorUp();
				return;
			case Key::ARROW_LEFT:
			case Key::LOWERCASE_H:
				GetCurrentEditor()->MoveCursorLeft();
				return;
			case Key::ARROW_RIGHT:
			case Key::LOWERCASE_L:
				GetCurrentEditor()->MoveCursorRight();
				return;
			default:
				;
		}
	}

	if (mode == EditorMode::Insert)
	{
		if (k == Key::ESCAPE)
		{
			SwitchToNormalMode();
		}
		return;
	}

	if (mode == EditorMode::Visual)
	{
		if (k == Key::ESCAPE)
		{
			SwitchToNormalMode();
		}
		else if (k == Key::LOWERCASE_D)
		{
			GetCurrentEditor()->DeleteSelectedRegion();
			SwitchToNormalMode();
		}
		else if (k == Key::COLON)
		{
			entering_command = true;
			SetStatus(StatusType::NONE);
		}
		return;
	}

	// Normal mode
	switch (k)
	{
		case Key::COLON:
			entering_command = true;
			SetStatus(StatusType::NONE);
			break;
		case Key::LOWERCASE_I:
			SwitchToInsertMode();
			break;
		case Key::LOWERCASE_V:
			SwitchToVisualMode();
			break;
		case Key::CTRL_U:
			GetCurrentEditor()->ScrollUpHalfPage();
			break;
		case Key::CTRL_D:
			GetCurrentEditor()->ScrollDownHalfPage();
			break;
		case Key::LOWERCASE_G:
			command_key = k;
			input_key_handler = &Hexa::InputKeyGoto;
			break;
		case Key::UPPERCASE_G:
			GetCurrentEditor()->JumpToFileEnd();
			break;
		default:
			;
	}
}

void Hexa::InputKeyGoto(Key cmdKey, Key k)
{
	switch (k)
	{
		case Key::LOWERCASE_G:
			GetCurrentEditor()->JumpToFileStart();
			input_key_handler = nullptr;
			return;
		default:
			break;
	}

	// Unknown cmd
	input_key_handler = nullptr;
}

void Hexa::SetStatus(StatusType status_type, const string &status_text)
{
	this->status_type = status_type;
	this->status_text = status_text;
}

// TODO use regex for most of the commands
void Hexa::ProcessCommand(const string &cmd)
{
	if (cmd == "quit" || cmd == "q")
	{
		quit_requested = true;
		return;
	}

	if (cmd == "")
		return;

	if (cmd == "set big-endian")
	{
		GetCurrentEditor()->SetViewEndianness(Endianness::BigEndian);
		return;
	}

	if (cmd == "set little-endian")
	{
		GetCurrentEditor()->SetViewEndianness(Endianness::LittleEndian);
		return;
	}

	// Matches ":set variable-name=321"
	regex set_positive_integer_regex("set[[:space:]]*(([[:alpha:]]+)(-[[:alpha:]]+)*)[[:space:]]*=[[:space:]]*([[:digit:]]+)");

	smatch matches;
	if (regex_match(cmd, matches, set_positive_integer_regex))
	{
		string variable_name = matches[1].str();
		int value = atoi(matches[4].str().c_str());
		// TODO check value range.

		if (variable_name == "byte-padding-left")
		{
			GetCurrentEditor()->GetStyleSheet().SetBytePaddingLeft(value);
		}
		else if (variable_name == "byte-padding-right")
		{
			GetCurrentEditor()->GetStyleSheet().SetBytePaddingRight(value);
		}
		else
		{
			SetStatus(StatusType::ERROR, "Unknown variable: " + variable_name);
		}
		return;
	}

	// Matches ":set filetype=elf"
	regex set_basic_string_regex("set[[:space:]]*(([[:alpha:]]+)(-[[:alpha:]]+)*)[[:space:]]*=[[:space:]]*([[:alpha:]]+)");

	if (regex_match(cmd, matches, set_basic_string_regex))
	{
		string variable_name = matches[1].str();
		string value = matches[4].str();

		if (variable_name == "filetype")
		{
			LoadScriptFile(string(args.runtime_dir_arg) + "/marks/" + value + ".hexa");
		}
		else
		{
			SetStatus(StatusType::ERROR, "Unknown variable: " + variable_name);
		}
		return;
	}

	if (cmd.size() > 7
	    && cmd.substr(0, 6) == "exec \""
	    && cmd[cmd.size() - 1] == '\"')
	{
		string file_name = cmd.substr(6);
		file_name.pop_back();
		LoadScriptFile(file_name);
		return;
	}

	// Process :mark "sadasdas" // For selection
	// or :mark 0:4 "Header" // For absolute offset:length
	regex mark_selection_regex("mark[[:space:]]*\"([^\"]+)\"");
	regex mark_absolute_range_regex("mark[[:space:]]*(([0-9]+):([0-9]+))[[:space:]]*\"([^\"]+)\"");
	if (regex_match(cmd, matches, mark_selection_regex))
	{
		if (mode != EditorMode::Visual)
		{
			SetStatus(StatusType::ERROR, "Not in visual mode");
		}
		else
		{
			string comment = matches[1].str();
			GetCurrentEditor()->MarkSelection(comment);

			SetStatus(StatusType::NORMAL,
			    "Marked, total marks = " + to_string(GetCurrentEditor()->marks.size()));
		}
		return;
	}
	if (regex_match(cmd, matches, mark_absolute_range_regex))
	{
		string comment = matches[4].str();
		int offset, length;
		try
		{
			offset = stoi(matches[2].str());
			length = stoi(matches[3].str());

			if (offset < 0 || length <= 0)
			{
				throw invalid_argument("Invalid range");
			}
		}
		catch (std::exception &e)
		{
			SetStatus(StatusType::ERROR, e.what());
			return;
		}

		GetCurrentEditor()->MarkRange(offset, length, comment);

		SetStatus(StatusType::NORMAL,
		    "Marked, total marks = " + to_string(GetCurrentEditor()->marks.size()));

		return;
	}

	// Process :tab 2
	if (cmd.size() > 4
	    && cmd.substr(0, 4) == "tab ")
	{
		// TODO FIXME this command should only work in normal mode

		string tab_no_str = cmd.substr(4);

		if (strspn(tab_no_str.c_str(), "0123456789") != tab_no_str.size())
		{
			SetStatus(StatusType::ERROR, "Unable to parse tab no: " + tab_no_str);
			return;
		}

		// TODO use boost::lexical_cast ?
		int tab_no = atoi(tab_no_str.c_str());
		if (tab_no > (int)tabs.size() || tab_no < 1)
		{
			SetStatus(StatusType::ERROR, "No such tab: " + tab_no_str);
			return;
		}

		current_tab = tab_no - 1;
		return;
	}

	// Check if string is numeric, :123 should move cursor to 123rd byte.
	if (strspn(cmd.c_str(), "0123456789") == cmd.size())
	{
		int new_pos;
		try
		{
			new_pos = stoi(cmd);
		}
		catch (out_of_range ex)
		{
			new_pos = -1;
		}

		if (new_pos < 0 || new_pos >= (int)GetCurrentEditor()->data->size())
		{
			SetStatus(StatusType::ERROR, "Position out of file boundary: " + cmd);
		}
		else
		{
			GetCurrentEditor()->cursor_pos = new_pos;
		}
		return;
	}

	if (cmd == "")
	{
		return;
	}

	// No command found.
	SetStatus(StatusType::ERROR, "Not a command: " + cmd);
}

HexEditor* Hexa::GetCurrentEditor()
{
	return &(tabs[current_tab].editor);
}

void Hexa::RenderTo(Painter &p)
{
	// `using` like definitions
	constexpr auto Vertical = Painter::SplitDirection::Vertical;
	constexpr auto Start = Painter::SplitEnd::Start;
	constexpr auto End = Painter::SplitEnd::End;
	constexpr auto Split = Painter::Split;

	last_screen_width = p.ColumnCount();

	p.Clear();

	Painter status_line_painter, tabs_list_painter;
	tie(status_line_painter, p) = Split(p, Vertical, End, 1);
	tie(tabs_list_painter, p) = Split(p, Vertical, Start, 1);

	RenderStatusLine(status_line_painter);
	RenderTabsList(tabs_list_painter);
	GetCurrentEditor()->RenderTo(p);
}

void SetStatusTypeColorsFor(Painter &p, Hexa::StatusType status_type)
{
	switch (status_type)
	{
		case Hexa::StatusType::NORMAL:
			p.SetFgColor(TermColor::None);
			p.SetBgColor(TermColor::None);
			break;
		case Hexa::StatusType::ERROR:
			p.SetFgColor(TermColor::White);
			p.SetBgColor(TermColor::Red);
			break;
		case Hexa::StatusType::NONE:
			break;
	}
}

void Hexa::RenderTabsList(Painter &p)
{
	p.SetBgColor(TermColor::BrightWhite);
	p.MoveTo(0, 0);
	for (int c = 0; c < p.ColumnCount(); ++c)
		p.Printf(" ");
	p.MoveTo(0, 0);

	// Minus two for padding
	int max_cols = (p.ColumnCount() / tabs.size()) - 2;

	for (int i = 0; i < (int)tabs.size(); ++i)
	{
		const TabInfo &ti = tabs[i];

		// Like vim, display last n characters of the file name.
		// TODO handle when filename is unicode
		string file_name_display = ti.file_name.substr(max(0, (int)ti.file_name.size() - max_cols));

		if (i == current_tab)
		{
			p.SetFgColor(TermColor::None);
			p.SetBgColor(TermColor::None);
			p.SetUnderline(false);
		}
		else
		{
			p.SetFgColor(TermColor::Black);
			p.SetBgColor(TermColor::White);
			p.SetUnderline(true);
		}

		p.Printf(" %s ", file_name_display.c_str());
	}

}

void Hexa::RenderStatusLine(Painter &p)
{
	// Render the command/status line.
	if (entering_command)
	{
		p.MoveTo(0, 0);
		p.Printf(":%s", command_buffer.c_str());
	}
	else
	{
		// Check if there is a status instead.
		if (status_type != StatusType::NONE)
		{
			p.MoveTo(0, 0);
			SetStatusTypeColorsFor(p, status_type);
			p.Printf("%s", status_text.c_str());
		}
	}
}

void Hexa::SwitchToNormalMode()
{
	mode = EditorMode::Normal;
	SetStatus(StatusType::NONE);
}

void Hexa::SwitchToInsertMode()
{
	mode = EditorMode::Insert;
	SetStatus(StatusType::NORMAL, "-- INSERT --");
}

void Hexa::SwitchToVisualMode()
{
	mode = EditorMode::Visual;
	SetStatus(StatusType::NORMAL, "-- VISUAL --");
	GetCurrentEditor()->selection_start_byte = GetCurrentEditor()->cursor_pos;
}
