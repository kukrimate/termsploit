/*
 * Termsploit API
 * Author: Mate Kukri
 * License: ISC
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
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

termsploit_ctx *termsploit_spawn(char *args[])
{
	termsploit_ctx *ctx;

	int master, slave;
	pid_t pid;

	ctx = malloc(sizeof(termsploit_ctx));
	if (!ctx)
		goto err;

	if (-1 == openrawpty(&master, &slave))
		goto err_free;

	pid = fork();
	switch (pid) {
	case -1:
		goto err_close;

	case 0:
		/* Child process */
		close(master);
		if (-1 == dup2(slave, STDIN_FILENO) ||
				-1 == dup2(slave, STDOUT_FILENO) ||
				-1 == dup2(slave, STDERR_FILENO)) {
			perror("dup2");
			goto err_child;
		}
		close(slave);
		if (-1 == setsid()) {
			perror("setsid");
			goto err_child;
		}
		if (-1 == ioctl(0, TIOCSCTTY, 1)) {
			perror("ioctl");
			goto err_child;
		}
		execv(args[0], args);
		perror("exec");
err_child:
		exit(1);
	}

	ctx->pid = pid;
	ctx->fd  = master;
	close(slave);
	return ctx;

err_close:
	close(master);
	close(slave);
err_free:
	free(ctx);
err:
	return NULL;
}

termsploit_ctx *termsploit_connect(char *host, uint16_t port)
{
	termsploit_ctx *ctx;

	ctx = malloc(sizeof(termsploit_ctx));
	if (!ctx)
		return NULL;

	ctx->pid = -1;
	ctx->fd  = tcpopen(host, port);
	if (-1 == ctx->fd) {
		free(ctx);
		return NULL;
	}
	return ctx;
}

/*** Core API ***/

void termsploit_free(termsploit_ctx *ctx)
{
	close(ctx->fd);
	free(ctx);
}

ssize_t termsploit_read(termsploit_ctx *ctx, char *buf, size_t len)
{
	return read(ctx->fd, buf, len);
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
	if (-1 == ctx->pid) {
		errno = EMEDIUMTYPE;
		return -1;
	}

	return kill(ctx->pid, signum);
}

int termsploit_wait(termsploit_ctx *ctx)
{
	int exitcode;

	if (-1 == ctx->pid) {
		errno = EMEDIUMTYPE;
		return -1;
	}

	if (-1 == waitpid(ctx->pid, &exitcode, 0))
		return -1;
	return WEXITSTATUS(exitcode);
}
