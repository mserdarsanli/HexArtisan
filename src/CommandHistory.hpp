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

#include <deque>
#include <string>

class CommandHistory
{
public:
	CommandHistory() = default;

	void LoadFromFile(const std::string &history_file_name);
	void WriteToFile(const std::string &history_file_name);

	void AddNewCommand(const std::string &command);

public:
	std::deque<std::string> commands;
	const size_t history_size = 2000;
};
