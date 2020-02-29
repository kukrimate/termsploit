#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>
#include "termsploit.h"

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

/*
 * Disable all special input handling on a pty and make it behave like a
 * bi-directional pipe
 */
static void makefullyraw(struct termios *conf)
{
	size_t i;
	for (i = 0; i < NCCS; ++i) {
		conf->c_cc[i] = 0;
	}
	cfmakeraw(conf);
}

termsploit_ctx *termsploit_spawn(char *args[])
{
	termsploit_ctx *ctx;

	int master, slave;
	struct termios conf;
	pid_t pid;

	ctx = malloc(sizeof(termsploit_ctx));
	if (!ctx)
		return NULL;

	if (-1 == (master = posix_openpt(O_RDWR | O_NOCTTY)))
		goto err;
	if (-1 == grantpt(master) ||
			-1 == unlockpt(master) ||
			-1 == (slave = open(ptsname(master), O_RDWR)))
		goto err_master;

	if (-1 == tcgetattr(master, &conf))
		goto err_slave;
	makefullyraw(&conf);
	if (-1 == tcsetattr(master, 0, &conf))
		goto err_slave;

	pid = fork();
	switch (pid) {
	case -1:
		goto err_slave;
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
err_slave:
	close(slave);
err_master:
	close(master);
err:
	free(ctx);
	return NULL;
}

termsploit_ctx *termsploit_sock(int sockfd)
{
	termsploit_ctx *ctx;

	ctx = malloc(sizeof(termsploit_ctx));
	if (!ctx)
		return NULL;

	ctx->pid = -1;
	ctx->fd  = sockfd;
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


static int makenonblock(int fd)
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

static int makeblock(int fd)
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

static int relay(int out, int in)
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
		if (-1 == relay(STDOUT_FILENO, ctx->fd))
			return -1;
		if (-1 == relay(ctx->fd, STDIN_FILENO))
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
