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

#include "Hexa.hpp"

using namespace std;

void Hexa::MarkFileType_Tar()
try
{
	int header_offset = 0;

	while (header_offset < (int)GetCurrentEditor()->data->size())
	{
		GetCurrentEditor()->MarkRange(header_offset,       100, "File Name");
		GetCurrentEditor()->MarkRange(header_offset + 100, 8,   "File Mode");
		GetCurrentEditor()->MarkRange(header_offset + 108, 8,   "Owner's UID");
		GetCurrentEditor()->MarkRange(header_offset + 116, 8,   "Group's UID");
		GetCurrentEditor()->MarkRange(header_offset + 124, 12,  "File size (octal)");
		GetCurrentEditor()->MarkRange(header_offset + 136, 12,  "Last modification TS (octal)");
		GetCurrentEditor()->MarkRange(header_offset + 148, 8,   "Header Checksum");
		GetCurrentEditor()->MarkRange(header_offset + 156, 1,   "Link Indicator");
		GetCurrentEditor()->MarkRange(header_offset + 157, 100, "Linked File Name");
		GetCurrentEditor()->MarkRange(header_offset + 257, 255, "Pad");

		int file_offset = header_offset + 512;
		string file_size_octal((const char*)&(*GetCurrentEditor()->data)[header_offset + 124], 12);

		// Stoi does not accept leading zeros
		file_size_octal.erase(0, file_size_octal.find_first_not_of('0'));

		int file_size = stoi(file_size_octal, 0, 8);

		GetCurrentEditor()->MarkRange(file_offset, file_size, "File Data");
		int pad_size = (512 - (file_size % 512)) % 512;
		GetCurrentEditor()->MarkRange(file_offset + file_size, pad_size, "Padding");

		header_offset += file_offset + file_size + pad_size;
	}
}
catch (exception &e)
{
	// Do anything?
	return;
}
