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
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
using namespace std;

#include <termios.h>
#include <unistd.h>

#include "Endianness.hpp"
#include "Terminal.hpp"
#include "StyleSheet.hpp"
#include "TermColor.hpp"

class Hexa;

// TODO this should be called FileView?
class HexEditor
{
private:
	struct MarkData
	{
		int start_address;
		int length;
		string user_comment;

		bool operator<(const MarkData &o) const
		{
			return start_address < o.start_address;
		}
	};

public:
	HexEditor(const Hexa *hexa, string file_name, vector<uint8_t> *data)
	  : hexa(hexa), file_name(file_name), data(data)
	{
	}

	void RenderTo(Painter p);

	template <typename IntegerType>
	string RenderIntegerOnCursor()
	{
		if (cursor_pos + sizeof(IntegerType) > data->size())
			return "~";
		IntegerType bytes = *reinterpret_cast<IntegerType*>(&(*data)[cursor_pos]);
		IntegerType num = FromEndianness(view_endianness, bytes);
		return std::to_string(num);
	}

	void MoveCursorDown()
	{
		if (cursor_pos + editor_column_count < (int)data->size())
			cursor_pos += editor_column_count;
	}
	void MoveCursorUp()
	{
		if (cursor_pos >= editor_column_count)
			cursor_pos -= editor_column_count;
	}
	void MoveCursorLeft()
	{
		if (cursor_pos > 0)
			--cursor_pos;
	}
	void MoveCursorRight()
	{
		if (cursor_pos + 1 < (int)data->size())
			++cursor_pos;
	}
	void DeleteSelectedRegion();

	void ScrollUpHalfPage()
	{
		cursor_pos -= (last_row_count / 2) * editor_column_count;
		if (cursor_pos < 0)
			cursor_pos = 0;
	}

	void ScrollDownHalfPage()
	{
		// Scroll down half a page.
		cursor_pos += (last_row_count / 2) * editor_column_count;
		if (cursor_pos >= (int)data->size())
			cursor_pos = data->size() - 1;
	}

	void JumpToFileEnd()
	{
		cursor_pos = data->size() - 1;
	}

	void JumpToFileStart()
	{
		cursor_pos = 0;
	}

private:
	// When cursor is near end of the visible region, update rows shown if needed.
	// Should ideally be called after cursor movements, but since HexEditor does not
	// know the screen size, this is called in every render method with the known
	// screen size.
	void FixScroll()
	{
		// Handle scroll up
		if (cursor_pos < first_byte_shown)
		{
			first_byte_shown = cursor_pos - (cursor_pos % editor_column_count);
			return;
		}

		// Handle scroll down
		// TODO being conservative here as computation is wrong.
		int line_quota = last_row_count;

		int cursor_row_first_byte = cursor_pos - (cursor_pos % editor_column_count);
		// i is the min first_byte_shown that is required to render cursor line.
		int i = cursor_row_first_byte;

		for (; i > 0 && line_quota >= 2; i -= editor_column_count)
		{
			line_quota -= (1 + IsRowMarked(i));

		}
		if (line_quota == 1 && i > 0 && !IsRowMarked(i))
		{
			i -= editor_column_count;
		}

		first_byte_shown = max(first_byte_shown, i);
	}

	// Widgets render functions
	void RenderEditor(Painter editor_painter);
	void RenderLine(Painter editor_painter, int row_first_byte);
	void RenderValueTable(Painter &p);
	void RenderInfoBar(Painter &p);

	// Functions to create marks.
	void MarkRange(int offset, int length, const string &comment);
	void MarkSelection(const string &comment);

	const MarkData* GetMarkUnder(int addr)
	{
		for (auto const &mark : marks)
		{
			if (mark.start_address <= addr && mark.start_address + mark.length > addr)
			{
				// TODO this is ugly.
				return &(mark);
			}
		}
		return nullptr;
	}

	bool IsRowMarked(int row_first_byte) const
	{
		for (auto const &mark : marks)
		{
			if (mark.start_address < row_first_byte + editor_column_count &&
			     mark.start_address + mark.length - 1 >= row_first_byte)
			{
				return true;
			}
		}
		return false;
	}

	// Callback needed for recalculating editor_column_count.
	void ScreenWidthUpdated(int new_screen_width);

	void SetViewEndianness(Endianness endianness)
	{
		this->view_endianness = endianness;
	}

	StyleSheet& GetStyleSheet()
	{
		return this->style_sheet;
	}

private:
	const Hexa *hexa = nullptr;

	// File name being edited.
	string file_name;

	// Index of the character the cursor is on.
	int cursor_pos = 0;

	// Number of editor columns shown.
	int editor_column_count = -1;

	// Index of the first byte shown in the first visible line.
	// Should be a multiple of editor_column_count.
	int first_byte_shown = 0;

	// Cached from the last RenderTo call.
	int last_row_count = -1;
	int last_column_count = -1;

	// Selection is between cursor_pos and this index
	int selection_start_byte = -1;

	// File contents to operate on.
	// TODO support large files with mmap?
	vector<uint8_t> *data;

	set<MarkData> marks;

	// Helper members. TODO remove those.
	vector< pair<const MarkData*, int > > mark_colors;
	int last_mark_color;

	Endianness view_endianness = Endianness::LittleEndian;

	StyleSheet style_sheet;

friend class Hexa;
};
