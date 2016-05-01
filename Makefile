# Copyright 2016 Mustafa Serdar Sanli
#
# This file is part of HexArtisan.
#
# HexArtisan is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# HexArtisan is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with HexArtisan.  If not, see <http://www.gnu.org/licenses/>.

SRCS = src/ScreenBufferRenderer.cpp \
       src/TerminalHexEditor.cpp \
       src/HexEditor.cpp \
       src/Hexa.cpp \
       src/HexaFileTypes.cpp \
       src/HexaScriptFunctions.cpp \
       src/Painter.cpp \
       src/StyleSheet.cpp \
       src/Unicode.cpp \
       src/CommandLineFlags.cpp \
       src/HexaScript/HexaScript.cpp

HDRS = src/HexEditor.hpp \
       src/Hexa.hpp \
       src/Endianness.hpp \
       src/Encoding/unicode_iterator.hpp \
       src/Encoding/utf8_iterator.hpp \
       src/Encoding/utf16_iterator.hpp \
       src/Encoding/utf32_iterator.hpp \
       src/Unicode.hpp \
       src/Painter.hpp \
       src/ScreenBuffer.hpp \
       src/ScreenBufferRenderer.hpp \
       src/ScreenPixel.hpp \
       src/StyleSheet.hpp \
       src/Terminal.hpp \
       src/TermInput.hpp \
       src/TermColor.hpp \
       src/CommandLineFlags.hpp \
       src/HexaScript/HexaScript.hpp

hexa: $(SRCS) $(HDRS)
	g++ -Wall -std=c++1y $(SRCS) -licuuc -o hexa

tesths: src/HexaScript/HexaScriptTest.cpp \
        src/HexaScript/HexaScript.hpp \
        src/HexaScript/HexaScript.cpp
	g++ -Wall -std=c++1y src/HexaScript/HexaScript.cpp src/HexaScript/HexaScriptTest.cpp -o tesths

.PHONY: test
test: tesths
	./tesths

src/CommandLineFlags.hpp src/CommandLineFlags.cpp: src/CommandLineFlags.ggo
	gengetopt --input="src/CommandLineFlags.ggo" --unamed-opts=files \
	    --c-extension=cpp --header-extension=hpp \
	    --file-name="CommandLineFlags"  --output-dir="src/"

.PHONY: install
install: hexa
	cp hexa /usr/local/bin/
	mkdir --parents /usr/share/hexa/hexa-1.0.0/
	cp -R -f runtime/* /usr/share/hexa/hexa-1.0.0/
