/*
 * Utility functions
 * Author: Mate Kukri
 * License: ISC
 */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
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
	if (sockfd < 0)
		goto err_free;
	if (connect(sockfd, ai->ai_addr, ai->ai_addrlen) < 0)
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
	if (*master < 0)
		goto err;

	if (grantpt(*master) < 0 || unlockpt(*master) < 0)
		goto err_master;

	*slave = open(ptsname(*master), O_RDWR);
	if (*slave < 0)
		goto err_master;

	if (tcgetattr(*slave, &termios) < 0)
		goto err_slave;
	cfmakeraw(&termios);
	if (tcsetattr(*slave, 0, &termios) < 0)
		goto err_slave;

	return 0;
err_slave:
	close(*slave);
err_master:
	close(*master);
err:
	return -1;
}

void init_vector(CharVec *vec)
{
	vec->n = 0;
	vec->size = 10;
	vec->array = reallocarray(NULL, vec->size, sizeof *vec->array);
}

void append_char(CharVec *vec, char ch)
{
	if (++vec->n >= vec->size) {
		vec->size = vec->n * 2;
		vec->array = reallocarray(vec->array, vec->size, sizeof *vec->array);
	}
	vec->array[vec->n - 1] = ch;
}

void append_chars(CharVec *vec, char *buf, size_t n)
{
	vec->n += n;
	if (vec->n >= vec->size) {
		vec->size = vec->n * 2;
		vec->array = reallocarray(vec->array, vec->size, sizeof *vec->array);
	}
	memcpy(vec->array + vec->n - n, buf, n * sizeof *buf);
}

void append_printf(CharVec *vec, char *fmt, ...)
{
	va_list va;
	char *out;
	size_t n;

	// Do printf to allocated buffer
	va_start(va, fmt);
	n = vasprintf(&out, fmt, va);
	va_end(va);
	// Append chars to vector
	append_chars(vec, out, n);
	// Free buffer
	free(out);
}

void pad_vector(CharVec *vec, char val, size_t n)
{
	size_t dif;

	// Calculate padding size
	assert(n >= vec->n);
	dif = n - vec->n;
	if (!dif)
		return;
	// Alocate space
	vec->n += dif;
	if (vec->n >= vec->size) {
		vec->size = vec->n * 2;
		vec->array = reallocarray(vec->array, vec->size, sizeof *vec->array);
	}
	// Set the padding
	memset(vec->array + vec->n - dif, val, dif * sizeof *vec->array);
}
