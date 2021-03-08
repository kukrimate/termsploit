#ifndef FMTSTR_H
#define FMTSTR_H

// Target endianness
typedef enum {
    TGT_LE = 0,
    TGT_BE = 1,
} TargetEndian;

// Address size
typedef enum {
    TGT_ADDR_32 = 4,
    TGT_ADDR_64 = 8,
} AddrSize;

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
            size_t fmt_off, uint64_t addr, void *data, size_t data_size);

#endif
