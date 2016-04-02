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

#include <stdexcept>
#include <vector>

#include "ScreenPixel.hpp"

// Screen buffer for the terminal, caches all the characters in the screen for making minimum I/O.
class ScreenBuffer
{
public:
	ScreenBuffer(int rows, int columns)
	  : row_count(rows), column_count(columns)
	{
		pixels.assign(rows * columns, ScreenPixel());
	}

	// 1-indexed row and col.
	ScreenPixel& operator()(int row, int col)
	{
		AssertBounds(row, col);
		return pixels[row * column_count + col];
	}

	const ScreenPixel& operator()(int row, int col) const
	{
		AssertBounds(row, col);
		return pixels[row * column_count + col];
	}

	int RowCount() const
	{
		return row_count;
	}

	int ColumnCount() const
	{
		return column_count;
	}

	// 1 indexed row and col
	void MoveCursor(int row, int col)
	{
		cursor_row = row;
		cursor_col = col;
	}

private:
	void AssertBounds(int row, int col) const
	{
		if (row < 0 || row >= row_count  || col < 0 || col >= column_count)
		{
			char err[100];
			sprintf(err, "Error accessiong screen pixel at row %d col %d", row, col);
			throw std::runtime_error(err);
		}
	}

private:
	int row_count;
	int column_count;

	int cursor_row = 0;
	int cursor_col = 0;

	std::vector<ScreenPixel> pixels;
};
