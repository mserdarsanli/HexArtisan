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

#include <string>

#include "unicode/umachine.h"
#include "unicode/uchar.h"
#include "unicode/unistr.h"
#include "unicode/utf.h"

#include "Endianness.hpp"
#include "Unicode.hpp"

using namespace std;

// TODO FIXME u_isgraph ignores spaces.

// Checks whether the give code point belongs to one of the following char
// categories:
//   * Me. U_ENCLOSING_MARK,
//   * Mn. U_NON_SPACING_MARK
//   * Mc. U_COMBINING_SPACING_MARK
//
// See runtime/tools/unicode.vim from vim repo.
bool IsComposingCodePoint(char32_t code_point)
{
	UCharCategory category = static_cast<UCharCategory>(
	    u_getIntPropertyValue(code_point, UCHAR_GENERAL_CATEGORY));

	return (category == U_ENCLOSING_MARK
	     || category == U_NON_SPACING_MARK
	     || category == U_COMBINING_SPACING_MARK);
}

bool IsControlCodePoint(char32_t code_point)
{
	UCharCategory category = static_cast<UCharCategory>(
	    u_getIntPropertyValue(code_point, UCHAR_GENERAL_CATEGORY));

	return category == U_CONTROL_CHAR;
}

int GetCodePointWidth(char32_t code_point)
{
	if (IsComposingCodePoint(code_point))
		return 0;

	UEastAsianWidth width = static_cast<UEastAsianWidth>(
	    u_getIntPropertyValue(code_point, UCHAR_EAST_ASIAN_WIDTH));

	// TODO some of the enum values are not well defined.
	// Need some tests for these.
	if (width == U_EA_FULLWIDTH || width == U_EA_WIDE)
		return 2;

	return 1;
}

void AppendCodePointAsUtf8(std::string &s, char32_t code_point)
{
	UnicodeString((UChar32)code_point).toUTF8String(s);
}
