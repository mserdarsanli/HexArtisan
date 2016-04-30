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

enum class Endianness
{
	BigEndian,
	LittleEndian,
};

inline uint8_t FromEndianness(Endianness e, uint8_t num)
{
	return num;
}

inline uint16_t FromEndianness(Endianness e, uint16_t num)
{
	return (e == Endianness::BigEndian ? be16toh(num) : le16toh(num));
}

inline uint32_t FromEndianness(Endianness e, uint32_t num)
{
	return (e == Endianness::BigEndian ? be32toh(num) : le32toh(num));
}

inline uint64_t FromEndianness(Endianness e, uint64_t num)
{
	return (e == Endianness::BigEndian ? be64toh(num) : le64toh(num));
}

inline int8_t FromEndianness(Endianness e, int8_t num)
{
	return num;
}

inline int16_t FromEndianness(Endianness e, int16_t num)
{
	return (e == Endianness::BigEndian ? be16toh(num) : le16toh(num));
}

inline int32_t FromEndianness(Endianness e, int32_t num)
{
	return (e == Endianness::BigEndian ? be32toh(num) : le32toh(num));
}

inline int64_t FromEndianness(Endianness e, int64_t num)
{
	return (e == Endianness::BigEndian ? be64toh(num) : le64toh(num));
}

inline char16_t FromEndianness(Endianness e, char16_t num)
{
	return (e == Endianness::BigEndian ? be16toh(num) : le16toh(num));
}

inline char32_t FromEndianness(Endianness e, char32_t num)
{
	return (e == Endianness::BigEndian ? be32toh(num) : le32toh(num));
}

inline uint8_t ToEndianness(Endianness e, uint8_t num)
{
	return num;
}

inline uint16_t ToEndianness(Endianness e, uint16_t num)
{
	return (e == Endianness::BigEndian ? htobe16(num) : htole16(num));
}

inline uint32_t ToEndianness(Endianness e, uint32_t num)
{
	return (e == Endianness::BigEndian ? htobe32(num) : htole32(num));
}

inline uint64_t ToEndianness(Endianness e, uint64_t num)
{
	return (e == Endianness::BigEndian ? htobe64(num) : htole64(num));
}

inline int8_t ToEndianness(Endianness e, int8_t num)
{
	return num;
}

inline int16_t ToEndianness(Endianness e, int16_t num)
{
	return (e == Endianness::BigEndian ? htobe16(num) : htole16(num));
}

inline int32_t ToEndianness(Endianness e, int32_t num)
{
	return (e == Endianness::BigEndian ? htobe32(num) : htole32(num));
}

inline int64_t ToEndianness(Endianness e, int64_t num)
{
	return (e == Endianness::BigEndian ? htobe64(num) : htole64(num));
}

inline char16_t ToEndianness(Endianness e, char16_t num)
{
	return (e == Endianness::BigEndian ? htobe16(num) : htole16(num));
}

inline char32_t ToEndianness(Endianness e, char32_t num)
{
	return (e == Endianness::BigEndian ? htobe32(num) : htole32(num));
}
