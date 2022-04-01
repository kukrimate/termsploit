/*
 * Termsploit API
 * Author: Mate Kukri
 * License: ISC
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <poll.h>
#include "termsploit.h"
#include "util.h"

struct termsploit_ctx {
	/*
	 * Process ID of the victim
	 * Set to -1 when using a socket
	 */
	pid_t pid;

	/* File descriptor
	 * Either:
	 *   - a socket
	 *   - or master side of a pty
	 */
	int fd;
};

/*** Core API ***/

termsploit_ctx *termsploit_alloc()
{
	termsploit_ctx *ctx;

	ctx = malloc(sizeof(termsploit_ctx));
	if (!ctx)
		abort();

	ctx->pid = -1;
	ctx->fd  = -1;
	return ctx;
}

void termsploit_free(termsploit_ctx *ctx)
{
	close(ctx->fd);
	free(ctx);
}

int termsploit_spawn(termsploit_ctx *ctx, char *args[])
{
	/* Create pty for communication */
	int master, slave;

	if (openrawpty(&master, &slave) < 0)
		return -1;

	/* Create the error pipe */
	int errfds[2];

	if (pipe2(errfds, O_CLOEXEC) < 0)
		goto err_pty;

	/* Create child process */
	pid_t pid = fork();

	if (pid < 0)
		goto err_pipe;

	if (pid == 0) {
		/* Only the child enters this */

		/* Close read side of the pipe */
		close(errfds[0]);
		/* Close master side of the pty */
		close(master);

		/* Make pty slave the child's console */
		if (dup2(slave, STDIN_FILENO) < 0 ||
				dup2(slave, STDOUT_FILENO) < 0 ||
				dup2(slave, STDERR_FILENO) < 0)
			goto err_child;
		close(slave);

		/* Make the pty the child's controlling terminal */
		if (setsid() < 0 || ioctl(STDIN_FILENO, TIOCSCTTY, 1) < 0)
			goto err_child;

		/* Execute requested binary */
		execv(args[0], args);

err_child:
		/* Write error code to pipe */
		write(errfds[1], &errno, sizeof errno);
		/* Exit child */
		exit(1);
	}

	/* Only the parent reaches this */

	/* No need for the write side */
	close(errfds[1]);
	/* Read error from pipe */
	if (read(errfds[0], &errno, sizeof errno) > 0)
		goto err_pipe0;
	/* No need for the read side anymore */
	close(errfds[0]);
	/* No need for the slave side of the pty */
	close(slave);

	ctx->pid = pid;
	ctx->fd  = master;
	return 0;

err_pipe:
	close(errfds[1]);
err_pipe0:
	close(errfds[0]);
err_pty:
	close(master);
	close(slave);
	return -1;
}

int termsploit_connect(termsploit_ctx *ctx, char *host, uint16_t port)
{
	ctx->pid = -1;
	ctx->fd  = tcpopen(host, port);
	if (ctx->fd < 0) {
		return -1;
	}
	return 0;
}

ssize_t termsploit_read(termsploit_ctx *ctx, char *buf, size_t len)
{
	return read(ctx->fd, buf, len);
}

char *termsploit_getline(termsploit_ctx *ctx)
{
	char ch;
	CharVec vec;
	ssize_t ret;

	init_vector(&vec);
	for (;;) {
		// Read character
		if ((ret = termsploit_read(ctx, &ch, sizeof ch)) < 0) {
			free(vec.array);
			return NULL;
		}
		// Add character if we got one
		if (ret)
			append_char(&vec, ch);
		// End of file or NL
		if (!ret || ch == '\n') {
			append_char(&vec, 0);
			return vec.array;
		}
	}
}

ssize_t termsploit_write(termsploit_ctx *ctx, char *buf, size_t len)
{
	return write(ctx->fd, buf, len);
}

void termsploit_interactive(termsploit_ctx *ctx)
{
	struct pollfd fds[2];
	char buf[4096];
	size_t l;


	fds[0].fd = STDIN_FILENO;
	fds[0].events = POLLIN;
	fds[1].fd = ctx->fd;
	fds[1].events = POLLIN;

	while (-1 != poll(fds, ARRAY_SIZE(fds), -1)) {
		if (fds[0].revents & POLLHUP || fds[1].revents & POLLHUP)
			break;

		if (fds[0].revents & POLLIN) {
			l = read(STDIN_FILENO, buf, sizeof(buf));
			if (l > 0)
				write(ctx->fd, buf, l);
		}

		if (fds[1].revents & POLLIN) {
			l = read(ctx->fd, buf, sizeof(buf));
			if (l > 0)
				write(STDOUT_FILENO, buf, l);
		}
	}
}

/*** Spawn only API ***/

int termsploit_kill(termsploit_ctx *ctx, int signum)
{
	if (ctx->pid < 0) {
		errno = EMEDIUMTYPE;
		return -1;
	}

	return kill(ctx->pid, signum);
}

int termsploit_wait(termsploit_ctx *ctx)
{
	int exitcode;

	if (ctx->pid < 0) {
		errno = EMEDIUMTYPE;
		return -1;
	}

	if (waitpid(ctx->pid, &exitcode, 0) < 0)
		return -1;
	return WEXITSTATUS(exitcode);
}
