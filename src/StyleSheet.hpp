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

// Each tab has its own stylesheet which can be configured.
class StyleSheet
{
public:
	StyleSheet() = default;

	void SetBytePaddingLeft(int new_value);
	void SetBytePaddingRight(int new_value);
	void SetMaxEditorCols(int new_value);

	int GetBytePaddingLeft();
	int GetBytePaddingRight();
	int GetMaxEditorCols();

private:
	// Controls padding between bytes in the editor.
	// If both of these are set to zero, there won't be any space between
	// hex values, like FF00FF.
	// Each byte is rendered like: " FF "
	//                              ^  ^
	//          byte_padding_left  /    \ byte_padding_right
	int byte_padding_left = 1;
	int byte_padding_right = 1;

	// Maximum number of bytes shown in each line.
	// This limits the horizontal space used even if the terminal is
	// resized to a larger width.
	int max_editor_cols = 100;
};
