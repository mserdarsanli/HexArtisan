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

#include "StyleSheet.hpp"

void StyleSheet::SetBytePaddingLeft(int new_value)
{
	this->byte_padding_left = new_value;
}

void StyleSheet::SetBytePaddingRight(int new_value)
{
	this->byte_padding_right = new_value;
}

void StyleSheet::SetMaxEditorCols(int new_value)
{
	this->max_editor_cols = new_value;
}

int StyleSheet::GetBytePaddingLeft()
{
	return this->byte_padding_left;
}

int StyleSheet::GetBytePaddingRight()
{
	return this->byte_padding_right;
}

int StyleSheet::GetMaxEditorCols()
{
	return this->max_editor_cols;
}
