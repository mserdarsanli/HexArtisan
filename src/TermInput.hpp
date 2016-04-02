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

enum Key
{
	UNKNOWN = -1,

	CTRL_A = 1,
	CTRL_B,
	CTRL_C,
	CTRL_D,
	CTRL_E,
	CTRL_F,
	CTRL_G,
	CTRL_H,
	CTRL_I,
	CTRL_J,
	CTRL_K,
	CTRL_L,
	ENTER,
	CTRL_N,
	CTRL_O,
	CTRL_P,
	CTRL_Q,
	CTRL_R,
	CTRL_S,
	CTRL_T,
	CTRL_U,
	CTRL_V,
	CTRL_W,
	CTRL_X,
	CTRL_Y,
	CTRL_Z,

	ESCAPE = 27,

	COLON = 58,

	UPPERCASE_A = 65,
	UPPERCASE_B,
	UPPERCASE_C,
	UPPERCASE_D,
	UPPERCASE_E,
	UPPERCASE_F,
	UPPERCASE_G,
	UPPERCASE_H,
	UPPERCASE_I,
	UPPERCASE_J,
	UPPERCASE_K,
	UPPERCASE_L,
	UPPERCASE_M,
	UPPERCASE_N,
	UPPERCASE_O,
	UPPERCASE_P,
	UPPERCASE_Q,
	UPPERCASE_R,
	UPPERCASE_S,
	UPPERCASE_T,
	UPPERCASE_U,
	UPPERCASE_V,
	UPPERCASE_W,
	UPPERCASE_X,
	UPPERCASE_Y,
	UPPERCASE_Z,

	LOWERCASE_A = 97,
	LOWERCASE_B,
	LOWERCASE_C,
	LOWERCASE_D,
	LOWERCASE_E,
	LOWERCASE_F,
	LOWERCASE_G,
	LOWERCASE_H,
	LOWERCASE_I,
	LOWERCASE_J,
	LOWERCASE_K,
	LOWERCASE_L,
	LOWERCASE_M,
	LOWERCASE_N,
	LOWERCASE_O,
	LOWERCASE_P,
	LOWERCASE_Q,
	LOWERCASE_R,
	LOWERCASE_S,
	LOWERCASE_T,
	LOWERCASE_U,
	LOWERCASE_V,
	LOWERCASE_W,
	LOWERCASE_X,
	LOWERCASE_Y,
	LOWERCASE_Z,

	BACKSPACE = 127,

	// Arbitrary range outside ASCII
	ARROW_LEFT = 300,
	ARROW_RIGHT,
	ARROW_UP,
	ARROW_DOWN,
};
