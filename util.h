#ifndef UTIL_H
#define UTIL_H

/*
 * Open a TCP connection to a host on a port
 */
int tcpopen(char *host, uint16_t port);

/*
 * Create a "fully raw" pty, e.g. one that behaves like a bi-directional pipe
 */
int createfullyrawpty(int *master, int *slave);

/*
 * Set non-blocking mode on a file-descriptor
 */
int makenonblock(int fd);

/*
 * Un-set non-blocking mode on a file-descriptor
 */
int makeblock(int fd);

/*
 * Copy data between two file descriptors
 */
int fdcopy(int out, int in);

#endif
