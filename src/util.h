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


/*
 * Character vector
 */
typedef struct {
    size_t n;
    size_t size;
    char *array;
} CharVec;

/*
 * Initialize a character vector
 */
void init_vector(CharVec *vec);

/*
 * Append a character to a vector
 */
void append_char(CharVec *vec, char ch);

/*
 * Append multiple characters to a vector
 */
void append_chars(CharVec *vec, char *buf, size_t n);

/*
 * Printf appending to a vector
 */
void append_printf(CharVec *vec, char *fmt, ...);

/*
 * Pad a vector to n elements
 */
void pad_vector(CharVec *vec, char val, size_t n);

#endif
