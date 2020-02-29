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

/*
 * Configure pty to behave like a bi-directional pipe
 */
static int makefullyraw(int pty)
{
	struct termios conf;
	size_t i;

	if (-1 == tcgetattr(pty, &conf))
		return -1;
	for (i = 0; i < NCCS; ++i) {
		conf.c_cc[i] = 0;
	}
	cfmakeraw(&conf);
	if (-1 == tcsetattr(pty, 0, &conf))
		return -1;
	return 0;
}

int createfullyrawpty(int *master, int *slave)
{
	*master = posix_openpt(O_RDWR | O_NOCTTY);
	if (-1 == *master)
		goto err;

	if (-1 == grantpt(*master) || -1 == unlockpt(*master))
		goto err_master;

	*slave = open(ptsname(*master), O_RDWR);
	if (-1 == *slave)
		goto err_master;

	if (-1 == makefullyraw(*master))
		goto err_slave;

	return 0;
err_slave:
	close(*slave);
err_master:
	close(*master);
err:
	return -1;
}

int makenonblock(int fd)
{
	int fl;
	fl = fcntl(fd, F_GETFL);
	if (-1 == fl)
		return -1;
	fl |= O_NONBLOCK;
	if (-1 == fcntl(fd, F_SETFL, fl))
		return -1;
	return 0;
}

int makeblock(int fd)
{
	int fl;
	fl = fcntl(fd, F_GETFL);
	if (-1 == fl)
		return -1;
	fl &= ~O_NONBLOCK;
	if (-1 == fcntl(fd, F_SETFL, fl))
		return -1;
	return 0;
}

int fdcopy(int out, int in)
{
	ssize_t l;
	char buf[4096];

	l = read(in, buf, sizeof(buf));
	if (-1 == l) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return 0;
		return -1;
	}
	if (-1 == write(out, buf, l)) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return 0;
		return -1;
	}
	return 0;
}
