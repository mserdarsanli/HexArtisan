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

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "unicode/umachine.h"
#include "unicode/utf.h"

#include "Painter.hpp"
#include "Unicode.hpp"

using namespace std;

void Painter::Printf(const char *fmt, ...)
{
	char buf[1000]; // TODO make this secure
	va_list args;
	va_start (args, fmt);
	vsprintf(buf, fmt, args);
	va_end (args);
	PutString(buf);
}

void Painter::PutString(const string &s)
{
	const char *it = s.c_str();
	const char *end = it + s.size();

	while (it != end)
	{
		UChar32 code_point;
		int offset = 0;

		U8_NEXT(it, offset, end - it, code_point);

		if (code_point < 0)
		{
			throw invalid_argument("Invalid utf-8 sequence");
		}

		if (code_point == 0)
		{
			break;
		}

		// Print the code point and iterate the cursor.
		// TODO Composing characters should be handled before moving the cursor?.
		PutCodePoint(code_point, string(it, offset));
		it += offset;
	}
}

// Puts char and advances the pen accordingly.
// Pen might advance 0, 1, or 2 cells.
void Painter::PutCodePoint(char32_t code_point, std::string utf8_enc)
{
	// TODO throw exception when a control character is put.

	// Should this be an error or be ignored?
	if (column >= paint_col_end)
	{
		char error_string[100 + strlen(__FILE__)];
		sprintf(error_string, "Pen overflow %s:%d pen(%d, %d)", __FILE__, __LINE__, row, column);
		throw std::runtime_error(error_string);
	}

	if (IsComposingCodePoint(code_point))
	{
		// TODO store composing characters along with the main char.
		// Currently not handling composing characters.
		return;
	}

	(*screen)(row, column).SetFgColor(fg_color)
	                      .SetBgColor(bg_color)
	                      .SetUnderline(underline)
	                      .SetCodePoint(code_point, utf8_enc);

	column += GetCodePointWidth(code_point);
}

void Painter::DrawFrame(string title)
{
	title = " " + title + " ";
	if ((int)title.size() + 3 > ColumnCount())
	{
		title.resize(ColumnCount() - 3);
	}
	string top_padding;
	for (int i = 0; i < ColumnCount() - 3 - (int)title.size(); ++i)
		top_padding += "━";
	string bottom_padding;
	for (int i = 0; i < ColumnCount() - 2; ++i)
		bottom_padding += "─";

	string top = "┍━" + title + top_padding + "┑";
	string bottom = "└" + bottom_padding + "┘";

	MoveTo(0, 0);
	Printf("%s", top.c_str());

	for (int r = 1; r < RowCount() - 1; ++r)
	{
		MoveTo(r, 0);
		Printf("│");
		MoveTo(r, ColumnCount() - 1);
		Printf("│");
	}

	MoveTo(RowCount() - 1, 0);
	Printf("%s", bottom.c_str());
}

Painter Painter::FramedArea() const
{
	if (RowCount() < 2 || ColumnCount() < 2)
	{
		char error_string[100 + strlen(__FILE__)];
		sprintf(error_string, "Invalid split on %s:%d", __FILE__, __LINE__);
		throw SplitException(error_string);
	}

	Painter framed{*this};
	++framed.paint_row_start;
	--framed.paint_row_end;
	++framed.paint_col_start;
	--framed.paint_col_end;
	return framed;
}
