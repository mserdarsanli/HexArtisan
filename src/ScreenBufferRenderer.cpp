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

#include "ScreenBufferRenderer.hpp"
#include "Unicode.hpp"

using namespace std;

void UpdateScreen(std::ostream &out, const ScreenBuffer &prev_screen, const ScreenBuffer &draft_screen)
{
	// Updating color is only done when needed, these vars will keep the last color code.
	// Initialized to an invalid color value to force updating for the first rendered char.
	bool prev_underline = false;
	out << "\033[24m"; // Turn off underline, TODO find better way?

	int prev_fg_color = -2;
	int prev_bg_color = -2;

	int term_row_count = draft_screen.RowCount();
	int term_col_count = draft_screen.ColumnCount();

	for (int r = 0; r < term_row_count; ++r)
	{
		int c = 0;
		while (c < term_col_count && draft_screen(r, c) == prev_screen(r, c))
		{
			++c;
		}

		// c is the first differing column in the row
		if (c > term_col_count)
		{
			// No change needed for this row
			continue;
		}

		// Jump to column
		out << "\033[" << (r+1) << ";" << (c+1) << "H";

		// Copy the rest
		for (; c < term_col_count; ++c)
		{
			bool underline = draft_screen(r, c).Underline();
			if (underline != prev_underline)
			{
				prev_underline = underline;
				out << (underline ? "\033[4m" : "\033[24m");
			}

			int fg_col = draft_screen(r, c).FgColor();
			// Put fg color
			if (fg_col != prev_fg_color)
			{
				prev_fg_color = fg_col;

				if (fg_col == -1)
				{
					out << "\033[39m"; // Reset fg color.
				}
				else
				{
					out << "\033[38;5;" << fg_col << "m";
				}
			}

			int bg_col = draft_screen(r, c).BgColor();
			// Put bg color
			if (bg_col != prev_bg_color)
			{
				prev_bg_color = bg_col;

				if (bg_col == -1)
				{
					out << "\033[49m"; // Reset bg color.
				}
				else
				{
					out << "\033[48;5;" << bg_col << "m";
				}
			}

			out << draft_screen(r, c).AsUtf8();
			if (GetCodePointWidth(draft_screen(r, c).CodePoint()) == 2)
				++c;
		}
	}
}
