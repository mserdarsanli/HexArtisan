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

#include <iomanip>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
using namespace std;

#include <termios.h>
#include <unistd.h>

#include "ScreenBuffer.hpp"
#include "ScreenBufferRenderer.hpp"
#include "Painter.hpp"
#include "TermInput.hpp"

// Class that caches terminal attibutes on initialization and restores them on exit.
class Terminal
{
public:
	Terminal(int input_fd, int output_fd, int row_count, int col_count)
	  : input_fd(input_fd), output_fd(output_fd),
	    term_row_count(row_count), term_col_count(col_count),
	    draft_screen(row_count, col_count), prev_screen(row_count, col_count)
	{
		tcgetattr(input_fd, &old_input_attr);
		cur_input_attr = old_input_attr;
	}

	~Terminal()
	{
		// Restore old terminal attributes.
		tcsetattr(input_fd, TCSANOW, &old_input_attr);
		if (alternate_screen_set)
		{
			ResetAlternateScreen();
		}
		Flush();
	}

	void SetRawInputMode()
	{
		cfmakeraw(&cur_input_attr);
		cur_input_attr.c_cc[VMIN] = 1; // return after 1 char
		cur_input_attr.c_cc[VTIME] = 0; // don't wait
		tcsetattr(input_fd, TCSANOW, &cur_input_attr);
	}

	void SetAlternateScreen()
	{
		output_buffer << "\033[?1049h";
		alternate_screen_set = true;
	}

	void ResetAlternateScreen()
	{
		output_buffer << "\033[?1049l";
		alternate_screen_set = false;
	}

	void SwapBuffers()
	{
		UpdateScreen(output_buffer, prev_screen, draft_screen);
		Flush();
		prev_screen = draft_screen;
	}

	void Print(const char *s)
	{
		output_buffer << s;
	}

	void Flush()
	{
		// Hide cursor during flushing, TODO check if this is needed?
		string cmds = "\033[?25l" + output_buffer.str() + "\033[?25h";
		output_buffer.str(string());
		write(output_fd, cmds.c_str(), cmds.size());
	}

	// Grabs the next key press from input and returns the key value.
	// Values here not same as ASCII values as some special characters like arrow keys
	// do not have ascii codes, but have escape sequences to represent them.
	//
	// Input parsing http://stjarnhimlen.se/snippets/vt100.txt
	// Fucking details about escape key.
	// http://redgrittybrick.org/terminals.html
	//
	// http://unixwiz.net/techtips/termios-vmin-vtime.html
	// is very useful
	Key GetKeyPress()
	{
		char input_buf[100];
		int s = read(input_fd, input_buf, 100);

		// TODO move parsing logic to TermInput.hpp so it will be defined in the
		// same place with the enum
		if (s == 1)
		{
			// Not an escape sequence, should be an ASCII value.
			return static_cast<Key>(input_buf[0]);
		}
		if (s == 3 && input_buf[0] == 033)
		{
			if (input_buf[1] == '[' && input_buf[2] == 'A')
				return Key::ARROW_UP;
			if (input_buf[1] == '[' && input_buf[2] == 'B')
				return Key::ARROW_DOWN;
			if (input_buf[1] == '[' && input_buf[2] == 'C')
				return Key::ARROW_RIGHT;
			if (input_buf[1] == '[' && input_buf[2] == 'D')
				return Key::ARROW_LEFT;
		}

		return Key::UNKNOWN;
	}

	ScreenBuffer& GetScreenBuffer()
	{
		return draft_screen;
	}

	void UpdateSize(int row_count, int col_count)
	{
		term_row_count = row_count;
		term_col_count = col_count;

		// Hack?
		prev_screen = ScreenBuffer(term_row_count, term_col_count);
		draft_screen = ScreenBuffer(term_row_count, term_col_count);

		// Clear screen since finding diffs are harder now as both screens are equal
		// and the term still keeping the old text.
		output_buffer << "\033[2J";
	}

private:
	int input_fd;
	int output_fd;

	int term_row_count;
	int term_col_count;

	// Draft screen can be edited by a Painter, prev screen is the last flushed data.
	// This is used to minimize outputs when most of the data is unchanged.
	// SwapBuffers will cause draft screen to be dumped.
	ScreenBuffer draft_screen;
	ScreenBuffer prev_screen;

	// Terminal attributes before any change was made by this class.
	// Used to restore the old values when program is exiting.
	struct termios old_input_attr;

	// Last set stdin attributes by this class.
	struct termios cur_input_attr;

	bool alternate_screen_set = false;

	// Buffer of output sequences, written when Flush() is called.
	stringstream output_buffer;
};
