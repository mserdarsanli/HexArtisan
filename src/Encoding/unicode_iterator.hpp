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

namespace unicode_iterator
{

// A given sequence of bytes are not necessarily correct utf8/utf16/utf32.
// Therefore unicode iterators need to handle various cases and return values
// indicating them, since they require special action.

// Null character, indicating end of string, iterator should not be forwarded
// after this. This is also a valid unicode codepoint.
constexpr char32_t CP_NULL = 0;

// When started decoding valid utf8/utf16 stream from a random place,
// first code point might be seen incompletely. This can be seen only for
// the first code point, and decoding continues normally.
constexpr char32_t CP_TRAIL_BYTES = 0xffffff00;

// When next sequence of bytes are forbidden by the encoding or if they encode
// an invalid code point, the decoding stops and this code point returned
// indicating the stream is corrupt. Iterator should not be forwarded after this.
constexpr char32_t CP_CORRUPT = 0xffffff01;

// If we are at the end of the range being iterated, it is marked with this
// code point, which means there should be more data to decode. Iterator should
// not be forwarded after this.
constexpr char32_t CP_END_OF_STREAM = 0xffffff02;


// Internal marker indicating the next code point has not been decoded yet.
// This code point will not be returned.
constexpr char32_t CP_NOT_DECODED = 0xffffffff;

} // namespace unicode_iterator
