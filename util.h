#ifndef UTIL_H
#define UTIL_H

/*
 * Number of elements in an array
 */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))

/*
 * Open a TCP connection to a host on a port
 */
int tcpopen(char *host, uint16_t port);

/*
 * Create a master-slave pty pair and set it to raw mode
 */
int openrawpty(int *master, int *slave);

#endif
