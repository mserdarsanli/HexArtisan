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

class ScreenPixel
{
public:
	ScreenPixel& SetBgColor(int color)
	{
		this->bg_color = color;
		return *this;
	}

	ScreenPixel& SetFgColor(int color)
	{
		this->fg_color = color;
		return *this;
	}

	ScreenPixel& SetUnderline(bool underline)
	{
		this->underline = underline;
		return *this;
	}

	// TODO hack.
	ScreenPixel& SetCodePoint(int code_point, std::string utf8_enc)
	{
		this->code_point = code_point;
		this->utf8_enc = utf8_enc;
		return *this;
	}

	std::string AsUtf8() const
	{
		return this->utf8_enc;
	}

	int BgColor() const
	{
		return bg_color;
	}

	int FgColor() const
	{
		return fg_color;
	}

	bool Underline() const
	{
		return underline;
	}

	int CodePoint() const
	{
		return code_point;
	}

	bool operator==(const ScreenPixel &ot) const
	{
		return bg_color == ot.bg_color &&
		       fg_color == ot.fg_color &&
		       underline == ot.underline &&
		       code_point == ot.code_point;
	}
private:
	int bg_color = -1;
	int fg_color = -1;
	bool underline = false;
	char32_t code_point = ' ';
	std::string utf8_enc;
};

