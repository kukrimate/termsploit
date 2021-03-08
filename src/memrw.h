/*
 * CPU independent memory read and write
 */

#ifndef ENDIAN_H
#define ENDIAN_H

/*
 * Read a byte from a buffer at offset off
 */
#define UCHAR_AT(buf, off) *((unsigned char *) buf + off)

/*
 * Little-endian
 */

static inline uint16_t read_le16(void *buf)
{
    return (uint16_t) UCHAR_AT(buf, 1) << 8 | (uint16_t) UCHAR_AT(buf, 0);
}

static inline uint32_t read_le32(void *buf)
{
    return (uint32_t) UCHAR_AT(buf, 3) << 24 | (uint32_t) UCHAR_AT(buf, 2) << 16
            | (uint32_t) UCHAR_AT(buf, 1) << 8 | (uint32_t) UCHAR_AT(buf, 0);
}

static inline uint64_t read_le64(void *buf)
{
    return (uint64_t) UCHAR_AT(buf, 7) << 56 | (uint64_t) UCHAR_AT(buf, 6) << 48
            | (uint64_t) UCHAR_AT(buf, 5) << 40 | (uint64_t) UCHAR_AT(buf, 4) << 32
            | (uint64_t) UCHAR_AT(buf, 3) << 24 | (uint64_t) UCHAR_AT(buf, 2) << 16
            | (uint64_t) UCHAR_AT(buf, 1) << 8 | (uint64_t) UCHAR_AT(buf, 0);
}

static inline void write_le16(void *buf, uint16_t val)
{
    UCHAR_AT(buf, 1) = val >> 8 & 0xff;
    UCHAR_AT(buf, 0) = val      & 0xff;
}

static inline void write_le32(void *buf, uint32_t val)
{
    UCHAR_AT(buf, 3) = val >> 24 & 0xff;
    UCHAR_AT(buf, 2) = val >> 16 & 0xff;
    UCHAR_AT(buf, 1) = val >> 8  & 0xff;
    UCHAR_AT(buf, 0) = val       & 0xff;
}

static inline void write_le64(void *buf, uint64_t val)
{
    UCHAR_AT(buf, 7) = val >> 56 & 0xff;
    UCHAR_AT(buf, 6) = val >> 48 & 0xff;
    UCHAR_AT(buf, 5) = val >> 40 & 0xff;
    UCHAR_AT(buf, 4) = val >> 32 & 0xff;
    UCHAR_AT(buf, 3) = val >> 24 & 0xff;
    UCHAR_AT(buf, 2) = val >> 16 & 0xff;
    UCHAR_AT(buf, 1) = val >> 8  & 0xff;
    UCHAR_AT(buf, 0) = val       & 0xff;
}

/*
 * Big-endian
 */

static inline uint16_t read_be16(void *buf)
{
    return (uint16_t) UCHAR_AT(buf, 0) << 8 | (uint16_t) UCHAR_AT(buf, 1);
}

static inline uint32_t read_be32(void *buf)
{
    return (uint32_t) UCHAR_AT(buf, 0) << 24 | (uint32_t) UCHAR_AT(buf, 1) << 16
            | (uint32_t) UCHAR_AT(buf, 2) << 8 | (uint32_t) UCHAR_AT(buf, 3);
}

static inline uint64_t read_be64(void *buf)
{
    return (uint64_t) UCHAR_AT(buf, 0) << 56 | (uint64_t) UCHAR_AT(buf, 1) << 48
            | (uint64_t) UCHAR_AT(buf, 2) << 40 | (uint64_t) UCHAR_AT(buf, 3) << 32
            | (uint64_t) UCHAR_AT(buf, 4) << 24 | (uint64_t) UCHAR_AT(buf, 5) << 16
            | (uint64_t) UCHAR_AT(buf, 6) << 8 | (uint64_t) UCHAR_AT(buf, 7);
}

static inline void write_be16(void *buf, uint16_t val)
{
    UCHAR_AT(buf, 0) = val >> 8 & 0xff;
    UCHAR_AT(buf, 1) = val      & 0xff;
}

static inline void write_be32(void *buf, uint32_t val)
{
    UCHAR_AT(buf, 0) = val >> 24 & 0xff;
    UCHAR_AT(buf, 1) = val >> 16 & 0xff;
    UCHAR_AT(buf, 2) = val >> 8  & 0xff;
    UCHAR_AT(buf, 3) = val       & 0xff;
}

static inline void write_be64(void *buf, uint64_t val)
{
    UCHAR_AT(buf, 0) = val >> 56 & 0xff;
    UCHAR_AT(buf, 1) = val >> 48 & 0xff;
    UCHAR_AT(buf, 2) = val >> 40 & 0xff;
    UCHAR_AT(buf, 3) = val >> 32 & 0xff;
    UCHAR_AT(buf, 4) = val >> 24 & 0xff;
    UCHAR_AT(buf, 5) = val >> 16 & 0xff;
    UCHAR_AT(buf, 6) = val >> 8  & 0xff;
    UCHAR_AT(buf, 7) = val       & 0xff;
}

#endif
