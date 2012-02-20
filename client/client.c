
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <poll.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

#include <curses.h>

// Ncurses HOWTO
// http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/index.html

int main(int argc, char **argv)
{
	if(argc < 3) {
		printf("Usage: %s host port\n", argv[0]);
		return 1;
	}

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0) {
		perror("socket");
		return 2;
	}
	struct    sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	
	struct hostent *host = gethostbyname(argv[1]);
	if(!host) {
		perror("gethostbyname");
		return 3;
	}
	
	memcpy(&servaddr.sin_addr, host->h_addr_list[0], host->h_length);
	if(connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr))) {
		perror("connect");
		return 4;
	}

	initscr();
	cbreak();
	noecho();
	timeout(0);

	printw("%s\n", "Ember client!");

	while(1) {
		struct pollfd pfds[2];
		pfds[0].fd = sock;
		pfds[0].events = POLLIN;
		
		pfds[1].fd = STDIN_FILENO;
		pfds[1].events = POLLIN;

		int res = poll(pfds, 2, 1000);
		if(res > 0) {
			if(pfds[1].revents & POLLIN) {
				// Input from stdin available
				int c;
				while((c = getch()) != ERR) {
					write(sock, &c, 1);
					if(c == '\n') {
						addch('\n');
						addch('\r');
					} else {
						addch(c);
					}
					refresh();
				}
			}
			if(pfds[0].revents & POLLIN) {
				// Input from socket
				char buf[1024];
				int r = read(sock, buf, sizeof(buf) - 1);
				if(r > 0) {
					buf[r] = 0;
					printw("%s", buf);
					refresh();
				}
			}
		}
	}
}

		
