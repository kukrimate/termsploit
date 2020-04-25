/*
 * Termsploit API
 * Author: Mate Kukri
 * License: ISC
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>
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

	if (-1 == createfullyrawpty(&master, &slave))
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


static int sigint_flag = 0;

static void sigint_handler(int signum)
{
	sigint_flag = 1;
	signal(SIGINT, SIG_DFL);
}

int termsploit_interactive(termsploit_ctx *ctx)
{
	if (-1 == makenonblock(ctx->fd) ||
			-1 == makenonblock(STDIN_FILENO))
		return -1;

	if (SIG_ERR == signal(SIGINT, sigint_handler))
		return -1;

	while (!sigint_flag) {
		if (-1 == fdcopy(STDOUT_FILENO, ctx->fd))
			return -1;
		if (-1 == fdcopy(ctx->fd, STDIN_FILENO))
			return -1;
	}

	if (-1 == makeblock(ctx->fd) ||
			-1 == makeblock(STDIN_FILENO))
		return -1;

	return 0;
}

/*** Spawn only API ***/

int termsploit_kill(termsploit_ctx *ctx, int sig)
{
	if (-1 == ctx->pid) {
		errno = EMEDIUMTYPE;
		return -1;
	}
	return kill(ctx->pid, sig);
}

/* FIXME: refactor busy loop into a timer */
int termsploit_exitcode(termsploit_ctx *ctx, time_t timeout)
{
	time_t start, cur;
	int exitcode;

	if (-1 == ctx->pid) {
		errno = EMEDIUMTYPE;
		return -1;
	}

	start = time(0);
	if (-1 == start)
		return -1;
	exitcode = -1;

	for (;;) {
		if (-1 == waitpid(ctx->pid, &exitcode, WNOHANG))
			return -1;

		if (-1 != exitcode) {
			return exitcode;
		} else {
			cur = time(0);
			if (-1 == cur)
				return -1;
			if (cur - start >= timeout)
				break;
		}
	}

	if (-1 == kill(ctx->pid, SIGKILL) ||
			-1 == waitpid(ctx->pid, &exitcode, 0))
		return -1;
	return exitcode;
}