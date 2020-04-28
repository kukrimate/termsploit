/*
 * Utility functions
 * Author: Mate Kukri
 * License: ISC
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "util.h"

int tcpopen(char *host, uint16_t port)
{
	char buffer[10];
	int err, sockfd;
	struct addrinfo *ai;

	snprintf(buffer, sizeof(buffer), "%d", port);
	err = getaddrinfo(host, buffer, NULL, &ai);
	if (err) {
		/*fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));*/
		errno = ENOENT;
		goto err;
	}

	sockfd = socket(ai->ai_addr->sa_family, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == sockfd)
		goto err_free;
	if (-1 == connect(sockfd, ai->ai_addr, ai->ai_addrlen))
		goto err_close;

	freeaddrinfo(ai);
	return sockfd;
err_close:
	close(sockfd);
err_free:
	freeaddrinfo(ai);
err:
	return -1;
}

int openrawpty(int *master, int *slave)
{
	struct termios termios;

	*master = posix_openpt(O_RDWR | O_NOCTTY);
	if (-1 == *master)
		goto err;

	if (-1 == grantpt(*master) || -1 == unlockpt(*master))
		goto err_master;

	*slave = open(ptsname(*master), O_RDWR);
	if (-1 == *slave)
		goto err_master;

	if (-1 == tcgetattr(*slave, &termios))
		goto err_slave;
	cfmakeraw(&termios);
	if (-1 == tcsetattr(*slave, 0, &termios))
		goto err_slave;

	return 0;
err_slave:
	close(*slave);
err_master:
	close(*master);
err:
	return -1;
}
