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

void Hexa::sc_SwitchToTab(int tab_no)
{
	if (tab_no > (int)tabs.size() || tab_no < 1)
	{
		SetStatus(StatusType::ERROR, "No such tab: " + to_string(tab_no));
		return;
	}

	current_tab = tab_no - 1;
}

void Hexa::sc_Quit()
{
	quit_requested = true;
}

void Hexa::sc_Exec(string file_name)
{
	LoadScriptFile(file_name);
}
