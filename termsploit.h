#ifndef TERMSPLOIT_H
#define TERMSPLOIT_H

/*
 * Termsploit context, opaque to the user
 */
typedef struct termsploit_ctx termsploit_ctx;

/*** Core API ***/

/*
 * Allocate a termsploit context
 */
termsploit_ctx *termsploit_alloc();

/*
 * Free a termsploit context
 */
void termsploit_free(termsploit_ctx *ctx);

/*
 * Spawn a process attached to this context
 */
int termsploit_spawn(termsploit_ctx *ctx, char *args[]);

/*
 * Attach a socket to a context
 */
int termsploit_connect(termsploit_ctx *ctx, char *host, uint16_t port);


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
 * NOTE: this function will take control of the user's terminal
 */
void termsploit_interactive(termsploit_ctx *ctx);

/*** Spawn only API ***/

/*
 * Send a signal to the process
 */
int termsploit_kill(termsploit_ctx *ctx, int signum);

/*
 * Wait for the process to exit
 */
int termsploit_wait(termsploit_ctx *ctx);

#endif
