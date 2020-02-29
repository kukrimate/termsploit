#ifndef TERMSPLOIT_H
#define TERMSPLOIT_H

/*
 * Termsploit context, opaque to the user
 */
typedef struct termsploit_ctx termsploit_ctx;

/*
 * Create a new termsploit context and spawn a process in it
 */
termsploit_ctx *termsploit_spawn(char *args[]);

/*
 * Create a new termsploit context and attach it to a socket
 */
termsploit_ctx *termsploit_sock(int sockfd);

/*** Core API ***/

/*
 * Free a termsploit context
 */
void termsploit_free(termsploit_ctx *ctx);

/*
 * Read from a context's stdout/stderr
 */
ssize_t termsploit_read(termsploit_ctx *ctx, char *buf, size_t len);

/*
 * Write to a context's stdin
 */
ssize_t termsploit_write(termsploit_ctx *ctx, char *buf, size_t len);

/*
 * Connect an interactive shell to a context
 * NOTE: this function is *NOT* thread-safe
 */
int termsploit_interactive(termsploit_ctx *ctx);

/*** Spawn only API ***/

/*
 * Send a signal to the process
 */
int termsploit_kill(termsploit_ctx *ctx, int sig);

/*
 * Wait for the process to exit and return the exit code
 */
int termsploit_exitcode(termsploit_ctx *ctx, time_t timeout);

#endif
