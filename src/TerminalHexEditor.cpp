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

#include <iomanip>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
using namespace std;

#include <termios.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/signalfd.h>
#include <signal.h>

#include "Terminal.hpp"
#include "Hexa.hpp"
#include "HexEditor.hpp"

#include "CommandLineFlags.hpp"

int MakeSignalFd()
{
	sigset_t mask;
	int sfd;

	sigemptyset(&mask);
	sigaddset(&mask, SIGWINCH);

	sigprocmask(SIG_BLOCK, &mask, nullptr);

	sfd = signalfd(-1, &mask, 0);

	return sfd;
}

int main(int argc, char *argv[])
{
	gengetopt_args_info args;
	if (cmdline_parser (argc, argv, &args) != 0)
	{
		exit(1);
	}

	if (args.inputs_num < 1)
	{
		cerr << gengetopt_args_info_usage << "\n";
		exit(1);
	}

	Hexa hexa{args};
	for (int i = 0; i < (int)args.inputs_num; ++i)
	{
		hexa.AddNewTab(args.inputs[i]);
	}

	// Create epooll structire with signalfd and STDIN
	int epoll_fd = epoll_create(100);

	enum EpollFd {
		EPOLL_STDIN,
		EPOLL_SIGNAL_FD,
	};

	{
		epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.u32 = EpollFd::EPOLL_STDIN;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
	}
	int signal_fd = MakeSignalFd(); // TODO store this with epoll_event
	{
		epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.u32 = EpollFd::EPOLL_SIGNAL_FD;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, signal_fd, &ev);
	}

	struct winsize term_size;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &term_size);

	Terminal terminal(STDIN_FILENO, STDOUT_FILENO, term_size.ws_row, term_size.ws_col);

	terminal.SetRawInputMode();
	terminal.SetAlternateScreen();
	terminal.Flush();

	Painter screen_painter(&terminal.GetScreenBuffer());
	hexa.RenderTo(screen_painter);
	terminal.SwapBuffers();

	while (1)
	{
		epoll_event ev;
		while (epoll_wait(epoll_fd, &ev, 1, -1) != 1);

		// TODO consume all the input here
		switch (ev.data.u32)
		{
		case EpollFd::EPOLL_STDIN:
		{
			Key k = terminal.GetKeyPress();
			if (k == Key::UNKNOWN)
			{
				continue;
			}

			hexa.InputKey(k);
			break;
		}
		case EpollFd::EPOLL_SIGNAL_FD:
		{
			// Read the signal
			struct signalfd_siginfo fdsi;
			read(signal_fd, &fdsi, sizeof(signalfd_siginfo));

			if (fdsi.ssi_signo == SIGWINCH)
			{
				struct winsize new_size;
				ioctl(STDIN_FILENO, TIOCGWINSZ, &new_size);
				terminal.UpdateSize(new_size.ws_row, new_size.ws_col);
			}

			break;
		}
		default:
			;
		}

		// Shitty way to check if there are more events
		if (epoll_wait(epoll_fd, &ev, 1, 0) > 0)
		{
			continue;
		}

		if (hexa.QuitRequested())
		{
			break;
		}

		screen_painter = Painter(&terminal.GetScreenBuffer());
		hexa.RenderTo(screen_painter);
		terminal.SwapBuffers();
	}

	return 0;
}
