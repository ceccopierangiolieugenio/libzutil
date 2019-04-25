/* lzop.h -- "zutil"

   This file is part of the zutil library.

   Copyright (C) 2019 Eugenio Parodi
   All Rights Reserved.

   the zutil library is free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   Eugenio Parodi
   <ceccopierangiolieugenio@googlemail.com>
   https://github.com/ceccopierangiolieugenio/libzutil
 */

#ifndef __EU_LZOP_H
#define __EU_LZOP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <unistd.h>

#define EU_LZOP_V_MAJOR ( 0x00 )
#define EU_LZOP_V_MINOR ( 0x01 )
#define EU_LZOP_V_MICRO ( 0x03 )
#define EU_LZOP_VERSION ( \
        ( EU_LZOP_V_MAJOR << 16 ) | \
        ( EU_LZOP_V_MINOR << 8  ) | \
        ( EU_LZOP_V_MICRO << 0  ) )

typedef enum {
    HEADER_NOT_READY = 0,
    HEADER_READY = 1
} H_STATUS;

typedef struct lzop_header_s{
    H_STATUS  ready;
    size_t size /* Header Size */;
    size_t blocksize; /* sizeof block descriptor */
    /* > Magic */
    uint16_t version;
    uint16_t lib_version;
    uint16_t version_needed_to_extract;
    uint8_t  method;
    uint8_t  level;
    uint32_t flags;
    uint32_t filter;
    uint32_t mode;
    uint32_t mtime_low;
    uint32_t mtime_high;
    uint8_t  name_length;
    char name[256+1];
    uint32_t chk;
} lzop_header;

typedef struct lzop_data_s{
    uint8_t *inbuf;
    size_t insize;
    uint8_t *outbuf;
    size_t outsize;
    uint8_t *wrkmem;
    size_t wrksize;
    uint32_t src_len;
    uint32_t dst_len;
    uint32_t src_adler32;
    uint32_t src_crc32;
    uint32_t dst_adler32;
    uint32_t dst_crc32;
} lzop_data;

typedef struct lzop_stream_s {
    uint8_t *next_in;     /* next input byte */
    size_t avail_in;  /* number of bytes available at next_in */
    // uint32_t total_in;  /* total number of input bytes read so far */

    uint8_t *next_out; /* next output byte will go here */
    size_t avail_out; /* remaining free space at next_out */
    // uint32_t total_out; /* total number of bytes output so far */

    lzop_header header;
    lzop_data data;

    char *msg;  /* last error message, NULL if no error */
} lzop_stream;

typedef lzop_stream *lzop_streamp;

typedef enum {
    LZOP_OK,
    LZOP_ERROR
} LZOP_STATUS;

typedef enum {
    LZOP_FLUSH,
    LZOP_NO_FLUSH
} LZOP_FLUSH_TYPE;

LZOP_STATUS lzop_inflateInit(lzop_streamp strm);
LZOP_STATUS lzop_deflateInit(lzop_streamp strm, int level);

LZOP_STATUS lzop_inflateEnd(lzop_streamp strm);
LZOP_STATUS lzop_deflateEnd(lzop_streamp strm);

LZOP_STATUS lzop_inflate(lzop_streamp strm);
LZOP_STATUS lzop_deflate(lzop_streamp strm, LZOP_FLUSH_TYPE flush);

#ifdef __cplusplus
}
#endif

#endif /* __EU_LZOP_H */
