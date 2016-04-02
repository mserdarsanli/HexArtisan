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

#include <cstring>
#include <cstdarg>
#include <exception>
#include <string>
#include <tuple>

#include "ScreenBuffer.hpp"
#include "TermColor.hpp"

class Painter
{
public:
	Painter()
	{
	}

	// Create painter targeted to whole ScreenBuffer area.
	Painter(ScreenBuffer *screen)
	  : screen(screen)
	  , paint_row_start(0)
	  , paint_row_end(screen->RowCount())
	  , paint_col_start(0)
	  , paint_col_end(screen->ColumnCount())
	{
	}

	// Clone another Painter including its pen state.
	Painter(const Painter &ot) = default;
	Painter& operator=(const Painter &ot) = default;

	enum class SplitDirection
	{
		Vertical,
		Horizontal,
	};

	enum class SplitEnd
	{
		Start,
		End,
	};

	class SplitException : public std::exception
	{
	public:
		SplitException() = delete;

		SplitException(std::string &&what_arg)
		  : what_str(std::move(what_arg))
		{
		}

		const char* what() const throw() override
		{
			return what_str.c_str();
		}

	private:
		std::string what_str;
	};

	// Splits painter into two Painters and returns them.
	// First return value is the Painter split per given params, and the
	// second value is the Painter for the remaining screen area.
	// Ex: tie(status_line, editor) = Split(entire_screen, Vertical, End, 1)
	// Can be used to get the last row for status_line, and remaining rows
	// for editor.
	// Pen positions are invalid after the split, and must be reset by the
	// caller.
	static std::tuple<Painter, Painter> Split(const Painter &orig,
	                                          SplitDirection split_direction,
	                                          SplitEnd split_end,
	                                          int split_amount)
	{
		if (split_direction == SplitDirection::Vertical)
		{
			if (split_amount > orig.paint_row_end - orig.paint_row_start)
			{
				char error_string[100 + std::strlen(__FILE__)];
				sprintf(error_string, "Invalid split on %s:%d", __FILE__, __LINE__);
				throw SplitException(error_string);
			}

			if (split_end == SplitEnd::Start)
			{
				return std::make_tuple(
				    Painter(orig.screen,
				        orig.paint_row_start, orig.paint_row_start + split_amount,
				        orig.paint_col_start, orig.paint_col_end),
				    Painter(orig.screen,
				        orig.paint_row_start + split_amount, orig.paint_row_end,
				        orig.paint_col_start, orig.paint_col_end));
			}
			else
			{
				return std::make_tuple(
				    Painter(orig.screen,
				        orig.paint_row_end - split_amount, orig.paint_row_end,
				        orig.paint_col_start,              orig.paint_col_end),
				    Painter(orig.screen,
				        orig.paint_row_start, orig.paint_row_end - split_amount,
				        orig.paint_col_start, orig.paint_col_end));
			}
		}
		else
		{
			if (split_amount > orig.paint_col_end - orig.paint_col_start)
			{
				char error_string[100 + std::strlen(__FILE__)];
				sprintf(error_string, "Invalid split on %s:%d", __FILE__, __LINE__);
				throw SplitException(error_string);
			}

			if (split_end == SplitEnd::Start)
			{
				return std::make_tuple(
				    Painter(orig.screen,
				        orig.paint_row_start, orig.paint_row_end,
				        orig.paint_col_start, orig.paint_col_start + split_amount),
				    Painter(orig.screen,
				        orig.paint_row_start, orig.paint_row_end,
				        orig.paint_col_start + split_amount, orig.paint_col_end));
			}
			else
			{
				return std::make_tuple(
				    Painter(orig.screen,
				        orig.paint_row_start, orig.paint_row_end,
				        orig.paint_col_end - split_amount, orig.paint_col_end),
				    Painter(orig.screen,
				        orig.paint_row_start, orig.paint_row_end,
				        orig.paint_col_start, orig.paint_col_end - split_amount));
			}
		}
	}

	void Clear()
	{
		for (int r = paint_row_start; r < paint_row_end; ++r)
		{
			for (int c = paint_col_start; c < paint_col_end; ++c)
			{
				(*screen)(r, c).SetFgColor(-1)
			                       .SetBgColor(-1)
			                       .SetUnderline(false)
			                       .SetCodePoint(' ', " ");
			}
		}
	}

	Painter FramedArea() const;

	void SetFgColor(TermColor color)
	{
		fg_color = static_cast<int>(color);
	}

	void SetBgColor(TermColor color)
	{
		bg_color = static_cast<int>(color);
	}

	void SetUnderline(bool underline)
	{
		this->underline = underline;
	}

	// Move pen to the position, irrelevant from the screen cursor.
	void MoveTo(int row, int column)
	{
		if (row < 0 || row + paint_row_start >= paint_row_end ||
		    column < 0 || column + paint_col_start >= paint_col_end)
		{
			char error_string[100 + std::strlen(__FILE__)];
			sprintf(error_string, "Invalid argument on %s:%d (%d, %d)", __FILE__, __LINE__, row, column);
			throw std::invalid_argument(error_string);
		}

		this->row = row + paint_row_start;
		this->column = column + paint_col_start;
	}

	void DrawFrame(std::string title);

	// Printed data must be valid utf-8 with no terminal escape sequences.
	void Printf(const char *fmt, ...);

	int RowCount() const
	{
		return paint_row_end - paint_row_start;
	}

	int ColumnCount() const
	{
		return paint_col_end - paint_col_start;
	}

private:

	// String must be valid utf-8
	void PutString(const std::string &s);
	void PutCodePoint(char32_t code_point, std::string utf8_enc);

private:
	Painter(ScreenBuffer *screen, int row_start, int row_end, int col_start, int col_end)
	  : screen(screen)
	  , paint_row_start(row_start)
	  , paint_row_end(row_end)
	  , paint_col_start(col_start)
	  , paint_col_end(col_end)
	{
	}

	bool underline = false;
	int fg_color = -1;
	int bg_color = -1;
	int row = 0;
	int column = 0;
	ScreenBuffer *screen = nullptr;

	// Half open ranges defining area the painter can paint to.
	// These are not exposed to Painter consumers, thus painting to 0,0
	// would mean painting to paint_row_start, paint_col_start in the
	// actual ScreenBuffer.
	int paint_row_start = -1;
	int paint_row_end = -1;
	int paint_col_start = -1;
	int paint_col_end = -1;
};
