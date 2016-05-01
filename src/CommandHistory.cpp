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

#include <fstream>
#include <string>

#include "CommandHistory.hpp"

using namespace std;

void CommandHistory::LoadFromFile(const string &history_file_name)
{
	commands.clear();

	ifstream history_file(history_file_name);
	string command;

	while (getline(history_file, command))
	{
		commands.push_back(command);
	}
}

void CommandHistory::WriteToFile(const string &history_file_name)
{
	ofstream history_file(history_file_name);

	for (const string &command : commands)
	{
		history_file << command << "\n";
	}
}

void CommandHistory::AddNewCommand(const string &command)
{
	commands.push_back(command);

	while (commands.size() > history_size)
	{
		commands.pop_front();
	}
}
