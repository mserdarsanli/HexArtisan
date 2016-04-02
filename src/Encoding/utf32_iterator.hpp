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

#include <iostream>

#include "unicode_iterator.hpp"
#include "../Endianness.hpp"

namespace unicode_iterator
{

// InputIterator,
// Returns special values for Corrupt and Trail and Eof
class utf32_iterator
{
public:
	utf32_iterator(const char32_t *it, const char32_t *end,
	               Endianness endianness = Endianness::LittleEndian)
	  : it(it), end(end), next_it(it), endianness(endianness)
	{
	}

	utf32_iterator(const char *it, const char *end,
	               Endianness endianness = Endianness::LittleEndian)
	  : it((const char32_t*)it), end((const char32_t*)end)
	  , next_it((const char32_t*)it), endianness(endianness)
	{
	}

	utf32_iterator(const utf32_iterator &ot) = default;

	char32_t operator*()
	{
		if (next_code_point == ::unicode_iterator::CP_NOT_DECODED)
		{
			ExtractNextCodePoint();
		}
		return next_code_point;
	}

	utf32_iterator& operator++()
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
	void ExtractNextCodePoint()
	{
		next_it = it;

		if (end - next_it <= 0)
		{
			next_code_point = ::unicode_iterator::CP_END_OF_STREAM;
			return;
		}

		// TODO check corruption (invalid code points).
		next_code_point = FromEndianness(endianness, *(next_it++));
	}

	const char32_t *it = nullptr;
	const char32_t *end = nullptr;

	const char32_t *next_it = nullptr;
	char32_t next_code_point = ::unicode_iterator::CP_NOT_DECODED;

	Endianness endianness;
};

} // namespace unicode_iterator

