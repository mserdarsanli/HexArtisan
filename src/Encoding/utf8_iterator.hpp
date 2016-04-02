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

#include "unicode_iterator.hpp"

namespace unicode_iterator
{

// TODO CP_TRAIL_BYTES not handled.
// InputIterator,
// Returns special values for Corrupt and Trail and Eof
class utf8_iterator
{
public:
	utf8_iterator(const char *it, const char *end)
	  : it(reinterpret_cast<const uint8_t*>(it))
	  , end(reinterpret_cast<const uint8_t*>(end))
	  , next_it(reinterpret_cast<const uint8_t*>(it))
	{
	}

	utf8_iterator(const utf8_iterator &ot) = default;

	char32_t operator*()
	{
		if (next_code_point == ::unicode_iterator::CP_NOT_DECODED)
		{
			ExtractNextCodePoint();
		}
		return next_code_point;
	}

	utf8_iterator& operator++()
	{
		if (next_it == it)
		{
			ExtractNextCodePoint();
		}
		it = next_it;
		next_code_point = ::unicode_iterator::CP_NOT_DECODED;
		return *this;
	}

private:
	// Valid UTF-8 code points:
	// 0xxxxxxx
	// 110xxxxx  10xxxxxx
	// 1110xxxx  10xxxxxx  10xxxxxx
	// 11110xxx  10xxxxxx  10xxxxxx  10xxxxxx
	//
	// Unused:
	// 111110xx  10xxxxxx  10xxxxxx  10xxxxxx  10xxxxxx
	// 1111110x  10xxxxxx  10xxxxxx  10xxxxxx  10xxxxxx  10xxxxxx
	void ExtractNextCodePoint()
	{
		next_it = it;

		if (end - next_it <= 0)
		{
			next_code_point = ::unicode_iterator::CP_END_OF_STREAM;
			return;
		}

		if (*next_it < 0b10000000)
		{
			next_code_point = *(next_it++);
			return;
		}

		// TODO FIXME corrupt before EOF not found here.

		// Two byte seq
		if (*next_it < 0b11100000)
		{
			if (end - next_it < 2)
			{
				next_code_point = ::unicode_iterator::CP_END_OF_STREAM;
				return;
			}
			if (!IsTrailByte(next_it[1]))
			{
				next_code_point = ::unicode_iterator::CP_CORRUPT;
				return;
			}

			next_code_point = 0b00011111 & *(next_it++);
			next_code_point <<= 6;
			next_code_point |= 0b00111111 & *(next_it++);
			return;
		}

		// Three byte seq
		if (*next_it < 0b11110000)
		{
			if (end - next_it < 3)
			{
				next_code_point = ::unicode_iterator::CP_END_OF_STREAM;
				return;
			}
			if (!IsTrailByte(next_it[1]) || !IsTrailByte(next_it[2]))
			{
				next_code_point = ::unicode_iterator::CP_CORRUPT;
				return;
			}

			next_code_point = 0b00011111 & *(next_it++);
			next_code_point <<= 6;
			next_code_point |= 0b00111111 & *(next_it++);
			next_code_point <<= 6;
			next_code_point |= 0b00111111 & *(next_it++);
			return;
		}

		// Four byte seq
		if (*next_it < 0b11111000)
		{
			if (end - next_it < 4)
			{
				next_code_point = ::unicode_iterator::CP_END_OF_STREAM;
				return;
			}
			if (!IsTrailByte(next_it[1]) || !IsTrailByte(next_it[2]) || !IsTrailByte(next_it[3]))
			{
				next_code_point = ::unicode_iterator::CP_CORRUPT;
				return;
			}

			next_code_point = 0b00011111 & *(next_it++);
			next_code_point <<= 6;
			next_code_point |= 0b00111111 & *(next_it++);
			next_code_point <<= 6;
			next_code_point |= 0b00111111 & *(next_it++);
			next_code_point <<= 6;
			next_code_point |= 0b00111111 & *(next_it++);
			return;
		}

		next_code_point = ::unicode_iterator::CP_CORRUPT;
		return;
	}

	bool IsTrailByte(char c)
	{
		return (0b11000000 & c) == 0b10000000;
	}

	const uint8_t *it = nullptr;
	const uint8_t *end = nullptr;

	const uint8_t *next_it = nullptr;
	char32_t next_code_point = ::unicode_iterator::CP_NOT_DECODED;
};

} // namespace unicode_iterator
