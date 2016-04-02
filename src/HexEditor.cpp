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

#include <locale>

#include "Hexa.hpp"
#include "HexEditor.hpp"
#include "Unicode.hpp"

#include "Encoding/utf8_iterator.hpp"
#include "Encoding/utf16_iterator.hpp"
#include "Encoding/utf32_iterator.hpp"

using namespace std;

template <typename UnicodeIterator>
static void RenderStringToRow(Painter &p, int row, const char *enc, UnicodeIterator it)
{
	const StringInfo si(ExtractStringInfoFromUnicodeIterator(it, p.ColumnCount() - 20));

	p.MoveTo(row, 0);
	p.Printf("%10s | ", enc);

	// Starting with leading/trailing byte
	if (si.trails_skipped)
	{
		p.SetFgColor(TermColor::Red);
		p.Printf("…"); // Implying there were trailing bytes
		p.SetFgColor(TermColor::None);
	}
	else
	{
		p.Printf(" ");
	}

	p.Printf("%s", si.utf8_data.c_str());

	if (!si.null_terminated)
	{
		p.SetFgColor(TermColor::Red);
		p.Printf("�…"); // Implying there was a control character
		p.SetFgColor(TermColor::None);
	}
}

void HexEditor::RenderTo(Painter screen)
{
	// `using` like definitions
	constexpr auto Vertical = Painter::SplitDirection::Vertical;
	constexpr auto End = Painter::SplitEnd::End;
	constexpr auto Split = Painter::Split;

	if (last_column_count != screen.ColumnCount())
	{
		ScreenWidthUpdated(screen.ColumnCount());
	}

	Painter p(screen);
	p.Clear();

	Painter editor_painter, info_bar_painter, value_table_painter;

	// Prepare render layout
	try
	{
		tie(info_bar_painter, p) = Split(p, Vertical, End, 1);
		tie(value_table_painter, p) = Split(p, Vertical, End, 8);
		editor_painter = p;
	}
	catch (Painter::SplitException &ex)
	{
		// We do not have enough screen space to render anything.
		// TODO do not handle this here?
		Painter error_painter(screen);
		error_painter.MoveTo(0, 0);
		error_painter.Printf("Not enough screen space");
		return;
	}

	// TODO why was this?
	last_row_count = editor_painter.RowCount() - 1;

	FixScroll();

	RenderInfoBar(info_bar_painter);
	RenderEditor(editor_painter);
	RenderValueTable(value_table_painter);
}

void HexEditor::RenderInfoBar(Painter &p)
{
	p.MoveTo(0, 0);
	p.SetFgColor(TermColor::Black);
	p.SetBgColor(TermColor::BrightWhite);

	int fn_size = p.ColumnCount() - 18;

	// TODO this is cacheable?
	string file_name_display = file_name;

	// Clamp file name in the given size.
	if ((int)file_name_display.size() > fn_size)
	{
		file_name_display.resize(fn_size - 3);
		file_name_display += "...";
	}
	else
	{
		file_name_display += string(fn_size - file_name_display.size(), ' ');
	}

	// Show file name.
	p.Printf("%s ", file_name_display.c_str());

	// Print endianness.
	p.Printf("%s ", view_endianness == Endianness::BigEndian ? "BE" : "LE");

	// Show ruler.
	p.Printf("%9d %3d%%", cursor_pos, cursor_pos * 100 / data->size());
}

void HexEditor::RenderEditor(Painter p)
{
	// `using` like definitions
	constexpr auto Vertical = Painter::SplitDirection::Vertical;
	constexpr auto Start = Painter::SplitEnd::Start;
	constexpr auto Split = Painter::Split;

	// TODO FIXME this part is lost
	// Render column headers
	p.SetFgColor(TermColor::Yellow);
	for (int col = 0; col < editor_column_count; ++col)
	{
		// TODO align this with style_sheet
		p.MoveTo(0, 9 + 4 * col);
		p.Printf("%3d", col);
	}
	p.SetFgColor(TermColor::None);

	mark_colors.clear();

	Painter remaining_rows_painter = p;

	for (int row_first_byte = first_byte_shown; ;row_first_byte += editor_column_count)
	{
		int required_render_rows = 1 + IsRowMarked(row_first_byte);
		if (required_render_rows > remaining_rows_painter.RowCount())
		{
			break;
		}

		RenderLine(remaining_rows_painter, row_first_byte);

		tie(ignore, remaining_rows_painter) =
		    Split(remaining_rows_painter, Vertical, Start, required_render_rows);
	}
}

void HexEditor::RenderLine(Painter p, int row_first_byte)
{
	const int byte_padding_left = style_sheet.GetBytePaddingLeft();
	const int byte_padding_right = style_sheet.GetBytePaddingRight();
	const int byte_cols = 2 + byte_padding_left + byte_padding_right;

	p.MoveTo(0, 0);

	// Painter for marker texts.
	Painter pmark(p);

	// Yellow row number (decimal).
	// Showing the start index like 120, 140 (if editor_column_count is 20)

	if (row_first_byte < (int)data->size())
	{
		// Render row number only if not EOF
		p.SetFgColor(TermColor::Yellow);
		p.Printf("%7d  ", row_first_byte);
		p.SetFgColor(TermColor::None);
	}
	else
	{
		p.SetFgColor(TermColor::Magenta);
		p.Printf("~~~~~~~   ");
		p.SetFgColor(TermColor::None);
		return;
	}

	for (int col = 0; col < editor_column_count; ++col)
	{
		int cid = row_first_byte + col;

		const MarkData *mark = GetMarkUnder(cid);
		if (mark && mark->start_address == cid)
		{
			// A new mark type is being used now.
			// Assign it a color and store for future display.
			last_mark_color = 99 + 11 * mark_colors.size();
			mark_colors.push_back( make_pair(mark, last_mark_color) );
		}

		if (mark)
		{
			pmark.MoveTo(1, 9 + (col * byte_cols));
			pmark.SetFgColor(static_cast<TermColor>(last_mark_color));

			int mark_text_start = byte_cols * (cid - mark->start_address);
			if ((int)mark->user_comment.size() > mark_text_start)
			{
				pmark.Printf("%.*s", byte_cols, mark->user_comment.c_str() + mark_text_start);
			}
		}

		if (cid >= (int)data->size())
		{
			// EOF marker tat he next character (magenta).
			p.SetFgColor(TermColor::Magenta);
			p.Printf("%*s~~%*s", byte_padding_left, "", byte_padding_right, "");
			p.SetFgColor(TermColor::None);
		}
		else
		{
			if (cursor_pos == cid) p.SetFgColor(TermColor::Cyan);

			// Highlight backgrount if in selection mode and byte
			// is in the selected range
			bool hl = (hexa->GetEditorMode() == Hexa::EditorMode::Visual
			  && cid >= min(cursor_pos, selection_start_byte)
			  && cid <= max(cursor_pos, selection_start_byte));


			p.SetBgColor(hl ? TermColor::Yellow : TermColor::None);

			if (mark)
			{
				p.SetUnderline(mark->start_address != cid);
				p.SetFgColor(static_cast<TermColor>(last_mark_color));
				p.Printf("%*s", byte_padding_left, "");

				p.SetUnderline(true);
				p.SetFgColor(cursor_pos == cid ? TermColor::Cyan
				                               : static_cast<TermColor>(last_mark_color));

				p.Printf("%02x", (int)(*data)[cid]);

				p.SetUnderline(mark->start_address + mark->length - 1 != cid);
				p.SetFgColor(static_cast<TermColor>(last_mark_color));
				p.Printf("%*s", byte_padding_right, "");
				p.SetUnderline(false);
			}
			else
			{
				p.SetFgColor(cursor_pos == cid ? TermColor::Cyan : TermColor::None);
				p.Printf("%*s%02x%*s", byte_padding_left, "", (int)(*data)[cid],
				                       byte_padding_right, "");
			}

			p.SetBgColor(TermColor::None);
			p.SetFgColor(TermColor::None);
		}
	}

	// Render ASCII view

	// Padding
	p.Printf("  ");

	for (int col = 0; col < editor_column_count; ++col)
	{
		int cid = row_first_byte + col;

		if (cid >= (int)data->size())
		{
			p.SetFgColor(TermColor::Magenta);
			p.Printf("~");
			p.SetFgColor(TermColor::None);
		}
		else
		{
			char print_char = (isprint((*data)[cid]) ? (*data)[cid] : '.');
			p.Printf("%c", print_char);
		}
	}

}

void HexEditor::RenderValueTable(Painter &p)
{
	using ::unicode_iterator::utf8_iterator;
	using ::unicode_iterator::utf16_iterator;
	using ::unicode_iterator::utf32_iterator;
	// TODO move this check near layout logic.
	// Requires ? columns and ? rows

	p.DrawFrame("Value Table");

	// Unframe the painter
	p = p.FramedArea();
	p.MoveTo(0, 0);

	// Max digits required:
	//                   |   -128 |  -32768 | -2147483648 | -9223372036854775808 |
	p.Printf("           | int8_t | int16_t |   int32_t   |        int64_t       |");
	p.MoveTo(1, 0);
	p.Printf("    signed | %6s | %7s | %11s | %20s |",
	         RenderIntegerOnCursor<int8_t>().c_str(),
	         RenderIntegerOnCursor<int16_t>().c_str(),
	         RenderIntegerOnCursor<int32_t>().c_str(),
	         RenderIntegerOnCursor<int64_t>().c_str());

	p.MoveTo(2, 0);
	p.Printf("  unsigned | %6s | %7s | %11s | %20s |",
	         RenderIntegerOnCursor<uint8_t>().c_str(),
	         RenderIntegerOnCursor<uint16_t>().c_str(),
	         RenderIntegerOnCursor<uint32_t>().c_str(),
	         RenderIntegerOnCursor<uint64_t>().c_str());

	const char *begin = reinterpret_cast<const char*>(data->data() + cursor_pos);
	const char *end =   reinterpret_cast<const char*>(data->data() + data->size());

	RenderStringToRow(p, 3, "UTF-8",  utf8_iterator(begin, end));
	RenderStringToRow(p, 4, "UTF-16", utf16_iterator(begin, end, view_endianness));
	RenderStringToRow(p, 5, "UTF-32", utf32_iterator(begin, end, view_endianness));
}

void HexEditor::DeleteSelectedRegion()
{
	int range_begin = min(cursor_pos, selection_start_byte);
	int range_end = max(cursor_pos, selection_start_byte) + 1;
	data->erase(data->begin() + range_begin,
	            data->begin() + range_end);

	cursor_pos = range_begin;
	if (cursor_pos >= (int)data->size())
	{
		// TODO handle when data->size() == 0
		--cursor_pos;
	}
}

void HexEditor::ScreenWidthUpdated(int new_screen_width)
{
	const int line_number_columns = 9;
	const int ascii_view_padding = 2;
	const int byte_cols_for_editor = 2 + style_sheet.GetBytePaddingLeft() + style_sheet.GetBytePaddingRight();
	const int byte_cols_for_ascii_view = 1;

	const int remaining_columns = new_screen_width - line_number_columns - ascii_view_padding;
	const int max_editor_cols = remaining_columns / (byte_cols_for_editor + byte_cols_for_ascii_view);

	editor_column_count = min(max_editor_cols, style_sheet.GetMaxEditorCols());

	// TODO what happens to first_byte_shown? maybe call FixScroll?
}
