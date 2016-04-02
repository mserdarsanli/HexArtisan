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

// TODO CP_TRAIL_BYTES not handled.
// InputIterator,
// Returns special values for Corrupt and Trail and Eof
class utf16_iterator
{
public:
	utf16_iterator(const char16_t *it, const char16_t *end,
	               Endianness endianness = Endianness::LittleEndian)
	  : it(it), end(end), next_it(it), endianness(endianness)
	{
	}

	utf16_iterator(const char *it, const char *end,
	               Endianness endianness = Endianness::LittleEndian)
	  : it((const char16_t*)it), end((const char16_t*)end)
	  , next_it((const char16_t*)it), endianness(endianness)
	{
	}

	utf16_iterator(const utf16_iterator &ot) = default;

	char32_t operator*()
	{
		if (next_code_point == ::unicode_iterator::CP_NOT_DECODED)
		{
			ExtractNextCodePoint();
		}
		return next_code_point;
	}

	utf16_iterator& operator++()
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
		char16_t high = FromEndianness(endianness, *(next_it++));

		if (!IsSurrogate(high))
		{
			next_code_point = high;
			return;
		}

		if (!IsLead(high))
		{
			next_code_point = ::unicode_iterator::CP_CORRUPT;
			return;
		}

		if (end - next_it <= 0)
		{
			next_code_point = ::unicode_iterator::CP_END_OF_STREAM;
			return;
		}
		char16_t low = FromEndianness(endianness, *(next_it++));

		if (!IsTrail(low))
		{
			next_code_point = ::unicode_iterator::CP_CORRUPT;
			return;
		}

		next_code_point = (char32_t(high - 0xd800) << 10) + (low - 0xdc00);
	}

	bool IsSurrogate(char16_t c)
	{
		return (c >= 0xd800 && c <= 0xdfff);
	}

	bool IsLead(char16_t c)
	{
		return (c >= 0xd800 && c <= 0xdbff);
	}

	bool IsTrail(char16_t c)
	{
		return (c >= 0xdc00 && c <= 0xdfff);
	}

	const char16_t *it = nullptr;
	const char16_t *end = nullptr;

	const char16_t *next_it = nullptr;
	char32_t next_code_point = ::unicode_iterator::CP_NOT_DECODED;

	Endianness endianness;
};

} // namespace unicode_iterator
