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

#include <string>
#include <utility>

#include "Encoding/unicode_iterator.hpp"

bool IsComposingCodePoint(char32_t code_point);
bool IsControlCodePoint(char32_t code_point);
int GetCodePointWidth(char32_t code_point);

void AppendCodePointAsUtf8(std::string &s, char32_t code_point);

// A struct to represent printable string starting from or next to given address.
//
// See `utf_iscomposing` from vim source.
class StringInfo
{
public:
	StringInfo(StringInfo &&ot) = default;

	StringInfo() = default;

public:
	// Whether trailing bytes and composing characters are skipped to find
	// actual starting point we can point.
	bool trails_skipped = false;

	// Whether the given stream contains valid utf8/utf16.
	// If this value is false, rest of the values are undefined.
	bool is_valid = true;

	// At some point we need to trim the string, though this will be set
	// to true if the string terminates naturally.
	bool null_terminated = false;

	// How many 'characters' will be seen when the data is printed.
	int glyphs_count = 0;

	// String starting after trail bytes, when rendered moves cursor
	// `glyphs_count` to right.
	std::string utf8_data;
};

// Iterator that understands character composing and widths.
template <typename UnicodeIteratorType>
class unicode_glyphs_extractor
{
public:
	unicode_glyphs_extractor(int max_width, const UnicodeIteratorType &it)
	  : unicode_iterator(it), width_left(max_width)
	{
	}

	void ExtractInto(StringInfo &si)
	{
		si.trails_skipped = SkipTrails();

		while (width_left > 0)
		{
			char32_t code_point = *unicode_iterator;

			// Code point should not be a composing character here.
			if (code_point == 0)
			{
				si.null_terminated = true;
				break;
			}

			if (code_point == ::unicode_iterator::CP_END_OF_STREAM)
			{
				break;
			}

			if (code_point == ::unicode_iterator::CP_CORRUPT)
			{
				si.is_valid = false;
				break;
			}

			width_left -= GetCodePointWidth(code_point);
			if (width_left < 0)
			{
				break;
			}

			// Consider the string is corrupt if it contains control
			// characters, like escape and backspace.
			if (IsControlCodePoint(code_point))
			{
				si.is_valid = false;
				break;
			}

			++unicode_iterator;
			AppendCodePointAsUtf8(si.utf8_data, code_point);
			AppendComposingCodePointsAsUtf8(si.utf8_data);
		}
	}

private:
	bool SkipTrails()
	{
		bool skipped = false;

		while (1)
		{
			char32_t code_point = *unicode_iterator;

			if (code_point == ::unicode_iterator::CP_TRAIL_BYTES
			    || IsComposingCodePoint(code_point))
			{
				++unicode_iterator;
				skipped = true;
			}
			else
			{
				break;
			}
		}

		return skipped;
	}

	void AppendComposingCodePointsAsUtf8(std::string &s)
	{
		while (1)
		{
			char32_t code_point = *unicode_iterator;

			if (IsComposingCodePoint(code_point))
			{
				++unicode_iterator;
				AppendCodePointAsUtf8(s, code_point);
			}
			else
			{
				break;
			}
		}
	}

	UnicodeIteratorType unicode_iterator;
	int width_left;
};

template <typename UnicodeIteratorType>
inline StringInfo ExtractStringInfoFromUnicodeIterator(const UnicodeIteratorType &it, int max_glyphs)
{
	StringInfo si;
	unicode_glyphs_extractor<UnicodeIteratorType> extractor(max_glyphs, it);
	extractor.ExtractInto(si);
	return si;
}
