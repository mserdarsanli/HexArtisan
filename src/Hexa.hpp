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

#include <map>
#include <string>
#include <vector>
#include <utility>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "CommandLineFlags.hpp"

#include "CommandHistory.hpp"
#include "HexEditor.hpp"
#include "Painter.hpp"
#include "StyleSheet.hpp"
#include "TermInput.hpp"

#include "HexaScript/HexaScript.hpp"

class Hexa
{
public:
	Hexa(const gengetopt_args_info &args);
	Hexa(const Hexa &ot) = delete;
	Hexa& operator=(const Hexa &ot) = delete;

	void AddNewTab(const std::string &file_name);
	void RenderTo(Painter &p);

	bool QuitRequested() const
	{
		return quit_requested;
	}

	// Key handlers, for commands requiring multiple key presses, like `gg`
	void InputKey(Key k);
	void InputKeyGoto(Key cmdKey, Key k);

	enum class EditorMode
	{
		Normal,
		Insert,
		Visual,
	};

	enum StatusType
	{
		NONE,
		NORMAL,
		ERROR,
	};

	EditorMode GetEditorMode() const
	{
		return mode;
	}

	CommandHistory& GetCommandHistory()
	{
		return command_history;
	}

private:
	// Stuff to show in status bar when no command is being typed.
	void SetStatus(StatusType status_type, const string &status_text = "");
	void ProcessCommand(const std::string &cmd);
	HexEditor* GetCurrentEditor();

	void SwitchToNormalMode();
	void SwitchToInsertMode();
	void SwitchToVisualMode();

	void RenderStatusLine(Painter &p);
	void RenderTabsList(Painter &p);

	// Loads file and executes commands from it
	void LoadScriptFile(const std::string &file_name);

private:
	// Functions registered to `HexaScript` engine
	// They are prefixed with `sc_` for no reason.
	void sc_Exec(string file_name);
	void sc_MarkAbsoluteRange(string range, string comment);
	void sc_MarkSelection(string comment);
	void sc_Replace(string type, string value);
	void sc_SwitchToTab(int tab_no);
	void sc_Quit();

	// Functions to use when marking some file types.
	void MarkFileType_Tar();

private:
	// TODO make FileBuffer class.
	std::map< std::string, std::vector<uint8_t> > file_contents;

	struct TabInfo
	{
		std::string file_name;
		HexEditor editor; // TODO rename to view.
	};
	std::vector< TabInfo > tabs;

private:
	HexaScript script_engine;
	CommandHistory command_history;

	// Style sheet to copy when new editors are added.
	StyleSheet base_style_sheet;

	// Type of the status, might change its color etc.
	StatusType status_type = StatusType::NONE;
	// Test to show in the line.
	string status_text;

	EditorMode mode = EditorMode::Normal;

	// Signal to main loop for termination.
	bool quit_requested = false;

	// Entering a command after clicking ':'
	bool entering_command = false;
	// Command that is being entered
	string command_buffer;

	int current_tab = 0;

	// Cached value for column count computation.
	int last_screen_width = -1;

	const gengetopt_args_info &args;

	void (Hexa::*input_key_handler)(Key cmdKey, Key k) = nullptr;
	Key command_key = Key::UNKNOWN;
};
