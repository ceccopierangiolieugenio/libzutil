/* lzop.c -- "zutil"

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

#include <stdlib.h>
#include <endian.h>
#include <string.h>
#include <stdio.h>

#include <zutil/lzop.h>

#ifdef WIN32
#include <lzo/minilzo.h>
#else
#include <lzo/lzoconf.h>
#include <lzo/lzo1x.h>
#endif

// #define DEBUG

#ifdef DEBUG
#define PD(fmt, ...) printf("DEBUG " fmt, __VA_ARGS__)
#else
#define PD(fmt, ...) do{;}while(0)
#endif


#define __inv32(__num) \
           ((( __num & 0x000000FF ) << 24 )| \
            (( __num & 0x0000FF00 ) << 8  )| \
            (( __num & 0x00FF0000 ) >> 8  )| \
            (( __num & 0xFF000000 ) >> 24 ))

#define __inv16(__num) \
           ((( __num & 0x00FF ) << 8 )| \
            (( __num & 0xFF00 ) >> 8 ))

#if   __BYTE_ORDER == __BIG_ENDIAN
#define fromBe32(__be_num) (__be_num)
#define fromLe32(__be_num) __inv32(__be_num)
#define fromBe16(__be_num) (__be_num)
#define fromLe16(__be_num) __inv16(__be_num)
#define fromBe8(__be_num)  (__be_num)
#define fromLe8(__be_num)  (__be_num)
#define toBe32(__be_num) (__be_num)
#define toLe32(__be_num) __inv32(__be_num)
#define toBe16(__be_num) (__be_num)
#define toLe16(__be_num) __inv16(__be_num)
#define toBe8(__be_num)  (__be_num)
#define toLe8(__be_num)  (__be_num)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define fromBe32(__be_num) __inv32(__be_num)
#define fromLe32(__be_num) (__be_num)
#define fromBe16(__be_num) __inv16(__be_num)
#define fromLe16(__be_num) (__be_num)
#define fromBe8(__be_num)  (__be_num)
#define fromLe8(__be_num)  (__be_num)
#define toBe32(__be_num) __inv32(__be_num)
#define toLe32(__be_num) (__be_num)
#define toBe16(__be_num) __inv16(__be_num)
#define toLe16(__be_num) (__be_num)
#define toBe8(__be_num)  (__be_num)
#define toLe8(__be_num)  (__be_num)
#else
        Error: Endianes not defined
#endif

/*************************************************************************
// memory setup [ from lzop-1.04/src/lzop.c ]
**************************************************************************/

#define BLOCK_SIZE        (256*1024l)
#define MAX_BLOCK_SIZE    (64*1024l*1024l)        /* DO NOT CHANGE */

/* We want to compress the data block at 'in' with length 'IN_LEN' to
 * the block at 'out'. Because the input block may be incompressible,
 * we must provide a little more output space in case that compression
 * is not possible.
 */
#define ZBUFSIZELZOP_IN      ( BLOCK_SIZE )
#define ZBUFSIZELZOP_OUT     (ZBUFSIZELZOP_IN + ZBUFSIZELZOP_IN / 16 + 64 + 3)

static const uint8_t lzop_magic[9] =
    { 0x89, 0x4c, 0x5a, 0x4f, 0x00, 0x0d, 0x0a, 0x1a, 0x0a };

/*************************************************************************
// lzop file header [ from lzop-1.04/src/lzop.c ]
**************************************************************************/

/* header flags */
#define F_ADLER32_D     0x00000001L
#define F_ADLER32_C     0x00000002L
#define F_STDIN         0x00000004L
#define F_STDOUT        0x00000008L
#define F_NAME_DEFAULT  0x00000010L
#define F_DOSISH        0x00000020L
#define F_H_EXTRA_FIELD 0x00000040L
#define F_H_GMTDIFF     0x00000080L
#define F_CRC32_D       0x00000100L
#define F_CRC32_C       0x00000200L
#define F_MULTIPART     0x00000400L
#define F_H_FILTER      0x00000800L
#define F_H_CRC32       0x00001000L
#define F_H_PATH        0x00002000L
#define F_MASK          0x00003FFFL

/* operating system & file system that created the file [mostly unused] */
#define F_OS_FAT        0x00000000L         /* DOS, OS2, Win95 */
#define F_OS_AMIGA      0x01000000L
#define F_OS_VMS        0x02000000L
#define F_OS_UNIX       0x03000000L
#define F_OS_VM_CMS     0x04000000L
#define F_OS_ATARI      0x05000000L
#define F_OS_OS2        0x06000000L         /* OS2 */
#define F_OS_MAC9       0x07000000L
#define F_OS_Z_SYSTEM   0x08000000L
#define F_OS_CPM        0x09000000L
#define F_OS_TOPS20     0x0a000000L
#define F_OS_NTFS       0x0b000000L         /* Win NT/2000/XP */
#define F_OS_QDOS       0x0c000000L
#define F_OS_ACORN      0x0d000000L
#define F_OS_VFAT       0x0e000000L         /* Win32 */
#define F_OS_MFS        0x0f000000L
#define F_OS_BEOS       0x10000000L
#define F_OS_TANDEM     0x11000000L
#define F_OS_SHIFT      24
#define F_OS_MASK       0xff000000L

/* character set for file name encoding [mostly unused] */
#define F_CS_NATIVE     0x00000000L
#define F_CS_LATIN1     0x00100000L
#define F_CS_DOS        0x00200000L
#define F_CS_WIN32      0x00300000L
#define F_CS_WIN16      0x00400000L
#define F_CS_UTF8       0x00500000L         /* filename is UTF-8 encoded */
#define F_CS_SHIFT      20
#define F_CS_MASK       0x00f00000L

/* these bits must be zero */
#define F_RESERVED      ((F_MASK | F_OS_MASK | F_CS_MASK) ^ 0xffffffffL)

#define ADLER32_INIT_VALUE  1
#define CRC32_INIT_VALUE    0

static void _lzop_print_header(lzop_streamp strm){
    printf("LZOP Header:\n");
    printf("  version = 0x%04X\n", strm->header.version);
    printf("  lib_version = 0x%04X\n", strm->header.lib_version);
    printf("  version_needed_to_extract = 0x%04X\n", strm->header.version_needed_to_extract);
    printf("  method = 0x%02X\n", strm->header.method);
    printf("  level = 0x%02X\n", strm->header.level);
    printf("  flags = 0x%08X\n", strm->header.flags);
    if (strm->header.flags & F_H_FILTER){
        printf("  filter = 0x%08X\n", strm->header.filter);
    }
    printf("  mode = 0x%08X\n", strm->header.mode);
    printf("  mtime_low = 0x%08X\n", strm->header.mtime_low);
    printf("  mtime_high = 0x%08X\n", strm->header.mtime_high);
    printf("  name_length = 0x%02X\n", strm->header.name_length);
    if (strm->header.name_length > 0){
        printf("  name = %s\n", strm->header.name);
    }
    printf("  chk = 0x%08X\n", strm->header.chk);
    printf("LZOP Header Flags : 0X%08X\n", strm->header.flags);

    if (strm->header.flags & F_ADLER32_D)     printf("  F_ADLER32_D\n");
    if (strm->header.flags & F_ADLER32_C)     printf("  F_ADLER32_C\n");
    if (strm->header.flags & F_STDIN)         printf("  F_STDIN\n");
    if (strm->header.flags & F_STDOUT)        printf("  F_STDOUT\n");
    if (strm->header.flags & F_NAME_DEFAULT)  printf("  F_NAME_DEFAULT\n");
    if (strm->header.flags & F_DOSISH)        printf("  F_DOSISH\n");
    if (strm->header.flags & F_H_EXTRA_FIELD) printf("  F_H_EXTRA_FIELD\n");
    if (strm->header.flags & F_H_GMTDIFF)     printf("  F_H_GMTDIFF\n");
    if (strm->header.flags & F_CRC32_D)       printf("  F_CRC32_D\n");
    if (strm->header.flags & F_CRC32_C)       printf("  F_CRC32_C\n");
    if (strm->header.flags & F_MULTIPART)     printf("  F_MULTIPART\n");
    if (strm->header.flags & F_H_FILTER)      printf("  F_H_FILTER\n");
    if (strm->header.flags & F_H_CRC32)       printf("  F_H_CRC32\n");
    if (strm->header.flags & F_H_PATH)        printf("  F_H_PATH\n");

    if ((strm->header.flags & F_OS_MASK ) == F_OS_FAT)      printf("  F_OS_FAT\n");
    if ((strm->header.flags & F_OS_MASK ) == F_OS_AMIGA)    printf("  F_OS_AMIGA\n");
    if ((strm->header.flags & F_OS_MASK ) == F_OS_VMS)      printf("  F_OS_VMS\n");
    if ((strm->header.flags & F_OS_MASK ) == F_OS_UNIX)     printf("  F_OS_UNIX\n");
    if ((strm->header.flags & F_OS_MASK ) == F_OS_VM_CMS)   printf("  F_OS_VM_CMS\n");
    if ((strm->header.flags & F_OS_MASK ) == F_OS_ATARI)    printf("  F_OS_ATARI\n");
    if ((strm->header.flags & F_OS_MASK ) == F_OS_OS2)      printf("  F_OS_OS2\n");
    if ((strm->header.flags & F_OS_MASK ) == F_OS_MAC9)     printf("  F_OS_MAC9\n");
    if ((strm->header.flags & F_OS_MASK ) == F_OS_Z_SYSTEM) printf("  F_OS_Z_SYSTEM\n");
    if ((strm->header.flags & F_OS_MASK ) == F_OS_CPM)      printf("  F_OS_CPM\n");
    if ((strm->header.flags & F_OS_MASK ) == F_OS_TOPS20)   printf("  F_OS_TOPS20\n");
    if ((strm->header.flags & F_OS_MASK ) == F_OS_NTFS)     printf("  F_OS_NTFS\n");
    if ((strm->header.flags & F_OS_MASK ) == F_OS_QDOS)     printf("  F_OS_QDOS\n");
    if ((strm->header.flags & F_OS_MASK ) == F_OS_ACORN)    printf("  F_OS_ACORN\n");
    if ((strm->header.flags & F_OS_MASK ) == F_OS_VFAT)     printf("  F_OS_VFAT\n");
    if ((strm->header.flags & F_OS_MASK ) == F_OS_MFS)      printf("  F_OS_MFS\n");
    if ((strm->header.flags & F_OS_MASK ) == F_OS_BEOS)     printf("  F_OS_BEOS\n");
    if ((strm->header.flags & F_OS_MASK ) == F_OS_TANDEM)   printf("  F_OS_TANDEM\n");

    if (strm->header.flags & F_CS_NATIVE) printf("  F_CS_NATIVE\n");
    if (strm->header.flags & F_CS_LATIN1) printf("  F_CS_LATIN1\n");
    if (strm->header.flags & F_CS_DOS)    printf("  F_CS_DOS\n");
    if (strm->header.flags & F_CS_WIN32)  printf("  F_CS_WIN32\n");
    if (strm->header.flags & F_CS_WIN16)  printf("  F_CS_WIN16\n");
    if (strm->header.flags & F_CS_UTF8)   printf("  F_CS_UTF8\n");
}

LZOP_STATUS lzop_inflateInit(lzop_streamp strm){
    strm->data.inbuf  = (uint8_t*) malloc(ZBUFSIZELZOP_IN);
    strm->data.insize = 0;
    strm->data.outbuf = (uint8_t*) malloc(ZBUFSIZELZOP_IN);
    strm->data.outsize = 0;

    strm->data.src_len = 0;
    strm->data.dst_len = 0;
    strm->header.ready = HEADER_NOT_READY;
    strm->header.size  = sizeof(lzop_magic);
    return LZOP_OK;
}

LZOP_STATUS lzop_deflateInit(lzop_streamp strm, int level){
    strm->data.inbuf  = (uint8_t*) malloc(ZBUFSIZELZOP_IN);
    strm->data.insize = 0;
    strm->data.outbuf = (uint8_t*) malloc(ZBUFSIZELZOP_OUT);
    strm->data.outsize = 0;
    strm->data.wrkmem = (uint8_t*) malloc(LZO1X_999_MEM_COMPRESS);
    strm->data.wrksize = 0;

    strm->data.src_len = 0;
    strm->data.dst_len = 0;
    strm->header.ready = HEADER_NOT_READY;
    strm->header.size  = 38;

    /* Populate Header */
    strm->header.version = 0x1040; /* this implementation is based on LZOP 1.04 */
    strm->header.version_needed_to_extract = 0x0940;
    strm->header.lib_version = lzo_version() & 0xffff;
    strm->header.method = 3; /* M_LZO1X_999 */
    strm->header.level = level;
    strm->header.flags = F_OS_UNIX | F_ADLER32_D | F_STDIN | F_STDOUT ;
    strm->header.filter = 0;
    strm->header.mtime_low = 0;
    strm->header.mtime_high = 0;

    strm->header.chk = 0;

#ifdef DEBUG
    _lzop_print_header(strm);
#endif
    return LZOP_OK;
}

LZOP_STATUS lzop_inflateEnd(lzop_streamp strm){
    if (strm->data.inbuf)
        free(strm->data.inbuf);
    if (strm->data.outbuf)
        free(strm->data.outbuf);
    strm->data.inbuf  = 0;
    strm->data.outbuf = 0;
    return LZOP_OK;
}

LZOP_STATUS lzop_deflateEnd(lzop_streamp strm){
    if (strm->data.inbuf)
        free(strm->data.inbuf);
    if (strm->data.outbuf)
        free(strm->data.outbuf);
    if (strm->data.wrkmem)
        free(strm->data.wrkmem);
    strm->data.inbuf  = 0;
    strm->data.outbuf = 0;
    strm->data.wrkmem = 0;
    return LZOP_OK;
}

static size_t _lzop_fillbuffer_in(lzop_streamp strm, size_t size){
    // PD("FBI size: %ld %ld\n", size, strm->avail_in);
    if (strm->data.insize < size && strm->avail_in){
        size_t leftBytes = size - strm->data.insize;
        size_t toBeCopyed =  leftBytes < strm->avail_in ? leftBytes : strm->avail_in;
        // PD("FBI lb: %ld tbc:%ld\n", leftBytes, toBeCopyed);
        memcpy(strm->data.inbuf + strm->data.insize, strm->next_in, toBeCopyed);
        strm->data.insize += toBeCopyed;
        strm->avail_in -= toBeCopyed;
        if (strm->avail_in > 0){
            memcpy(strm->next_in, strm->next_in+toBeCopyed, strm->avail_in);
        }
    }
    return strm->data.insize;
}

/*
static size_t _lzop_fillbuffer_out(lzop_streamp strm, size_t size){
    PD("FBO size: %ld %ld\n", size, strm->avail_in);
    if (strm->data.outsize && strm->avail_out < size){
        size_t leftBytes = size - strm->avail_out;
        size_t toBeCopyed =  leftBytes < strm->data.outsize ? leftBytes : strm->data.outsize;
        PD("FBO lb: %ld tbc:%ld\n", leftBytes, toBeCopyed);
        memcpy(strm->next_out + strm->offset_out, strm->data.outbuf, toBeCopyed);
        strm->avail_out -= toBeCopyed;
        strm->offset_out += toBeCopyed;
        strm->data.outsize -= toBeCopyed;
        if (strm->data.outsize > 0){
            memcpy(strm->data.outbuf, strm->data.outbuf + toBeCopyed, strm->data.outsize);
        }
    }
    return strm->data.insize;
}
*/

/*
    magic         9
    version       2
    lib_version   2
    version_needed_to_extract  2
    method        1
    level         1
    flags         4
    filter        (flags & F_H_FILTER)?4:0
    mode          4
    mtime_low     4
    mtime_high    4
    name_length   1
    name[]        name_length
    chk           4
                  =
                  38 -> 298
    38 is enough to reach the name length;
 */
static LZOP_STATUS _lzop_header_read(lzop_streamp strm){
    while (!strm->header.ready){
        if (_lzop_fillbuffer_in(strm, strm->header.size) < strm->header.size){
            return LZOP_OK;
        }
        // PD("H_r size: %d %d\n",strm->data.insize, strm->header.size);
        if (strm->data.insize == sizeof(lzop_magic)){
            /* check magic */
            if (memcmp(strm->data.inbuf, lzop_magic, sizeof(lzop_magic))){
                return LZOP_ERROR;
            }
            strm->header.size = 38;
            continue;
        }else
        if (strm->data.insize == 38){
            size_t offset = sizeof(lzop_magic);
            strm->header.version     = fromBe16(*(uint16_t*)(&strm->data.inbuf[offset]));
            offset+=2;
            strm->header.lib_version = fromBe16(*(uint16_t*)(&strm->data.inbuf[offset]));
            offset+=2;
            strm->header.version_needed_to_extract = fromBe16(*(uint16_t*)(&strm->data.inbuf[offset]));
            offset+=2;
            strm->header.method      = fromBe8(*(uint8_t*)(&strm->data.inbuf[offset]));
            offset+=1;
            strm->header.level       = fromBe8(*(uint8_t*)(&strm->data.inbuf[offset]));
            offset+=1;
            strm->header.flags       = fromBe32(*(uint32_t*)(&strm->data.inbuf[offset]));
            offset+=4;
            if (strm->header.flags & F_H_FILTER){
                strm->header.filter  = fromBe32(*(uint32_t*)(&strm->data.inbuf[offset]));
                offset+=4;
            }
            strm->header.mode        = fromBe32(*(uint32_t*)(&strm->data.inbuf[offset]));
            offset+=4;
            strm->header.mtime_low   = fromBe32(*(uint32_t*)(&strm->data.inbuf[offset]));
            offset+=4;
            strm->header.mtime_high  = fromBe32(*(uint32_t*)(&strm->data.inbuf[offset]));
            offset+=4;
            strm->header.name_length = fromBe8(*(uint8_t*)(&strm->data.inbuf[offset]));
            offset+=1;

            strm->header.size += (strm->header.flags & F_H_FILTER)?4:0;
            strm->header.size += strm->header.name_length;
            if (strm->header.size == 38){
                strm->header.chk = fromBe32(*(uint32_t*)(&strm->data.inbuf[offset]));
                strm->header.ready = HEADER_READY;
                return LZOP_OK;
            }else{
                continue;
            }
        }else{
            size_t offset = (strm->header.flags & F_H_FILTER)?34:38;
            if (strm->header.name_length > 0){
                memcpy(strm->header.name,&strm->data.inbuf[offset],strm->header.name_length);
                strm->header.name[strm->header.name_length]='\0';
                offset += strm->header.name_length;
            }
            strm->header.chk = fromBe32(*(uint32_t*)(&strm->data.inbuf[offset]));
            strm->header.ready = HEADER_READY;
            return LZOP_OK;
        }
    }
}

static LZOP_STATUS _lzop_header_write(lzop_streamp strm){
    if (!strm->header.ready){
        memcpy(strm->data.outbuf, lzop_magic, sizeof(lzop_magic));
        strm->data.outsize = sizeof(lzop_magic);
        *(uint16_t*)(&strm->data.outbuf[strm->data.outsize]) = toBe16(strm->header.version);
        strm->data.outsize += 2;
        *(uint16_t*)(&strm->data.outbuf[strm->data.outsize]) = toBe16(strm->header.lib_version);
        strm->data.outsize += 2;
        *(uint16_t*)(&strm->data.outbuf[strm->data.outsize]) = toBe16(strm->header.version_needed_to_extract);
        strm->data.outsize += 2;
        *(uint8_t*)(&strm->data.outbuf[strm->data.outsize])  = toBe8(strm->header.method);
        strm->data.outsize += 1;
        *(uint8_t*)(&strm->data.outbuf[strm->data.outsize])  = toBe8(strm->header.level);
        strm->data.outsize += 1;
        *(uint32_t*)(&strm->data.outbuf[strm->data.outsize]) = toBe32(strm->header.flags);
        strm->data.outsize += 4;
        *(uint32_t*)(&strm->data.outbuf[strm->data.outsize]) = toBe32(strm->header.mode);
        strm->data.outsize += 4;
        *(uint32_t*)(&strm->data.outbuf[strm->data.outsize]) = toBe32(strm->header.mtime_low);
        strm->data.outsize += 4;
        *(uint32_t*)(&strm->data.outbuf[strm->data.outsize]) = toBe32(strm->header.mtime_high);
        strm->data.outsize += 4;
        *(uint8_t*)(&strm->data.outbuf[strm->data.outsize])  = toBe8(strm->header.name_length);
        strm->data.outsize += 1;
        strm->header.chk = lzo_adler32(ADLER32_INIT_VALUE, strm->data.outbuf+sizeof(lzop_magic), strm->data.outsize-sizeof(lzop_magic));
        *(uint32_t*)(&strm->data.outbuf[strm->data.outsize]) = toBe32(strm->header.chk);
        strm->data.outsize += 4;

        strm->header.ready = HEADER_READY;
#if 0
        PD("_lzop_header_write: avail_out:%ld, outsize:%ld - %02x %02x %02x %02x %02x %02x %02x %02x\n",
            strm->avail_out, strm->data.outsize,
            strm->data.outbuf[0],
            strm->data.outbuf[1],
            strm->data.outbuf[2],
            strm->data.outbuf[3],
            strm->data.outbuf[4],
            strm->data.outbuf[5],
            strm->data.outbuf[6],
            strm->data.outbuf[7]
                );
#endif
        return LZOP_OK;
    }
}

/*
 * Inflate Workflow 
 *    --->  next_in, avail_in
 *           \--> inbuf, insize == dst_len
 *                  \-> inflate -> outbuf, outsize
 *    <---  next_out, avail_out  <--/
 */
LZOP_STATUS lzop_inflate(lzop_streamp strm){
    size_t out_offset = 0;
    while(strm->avail_in > 0 || strm->avail_out > 0 ){
        /* check if there are left bytes that can be copyed in the avail_out */
        if ( strm->avail_out > 0 && strm->data.outsize ){
            size_t toBeCopyed = strm->data.outsize > strm->avail_out ? strm->avail_out : strm->data.outsize;
            memcpy(strm->next_out+out_offset, strm->data.outbuf, toBeCopyed);
            strm->avail_out -= toBeCopyed;
            out_offset += toBeCopyed;
            strm->data.outsize -= toBeCopyed;
            if(strm->data.outsize > 0){
                memcpy(strm->data.outbuf, strm->data.outbuf+toBeCopyed, strm->data.outsize);
            }
            if (strm->avail_out==0){
                return LZOP_OK;
            }
        }
        PD("Deflate 001 avail_in: %ld  h_ready: %d  dst_len: %d\n", strm->avail_in, strm->header.ready, strm->data.dst_len);
        /* phase 1, decode the header */
        if (!strm->header.ready){
            if (LZOP_OK != _lzop_header_read(strm)){
                return LZOP_ERROR;
            }
            if (strm->header.ready){
#ifdef DEBUG
                _lzop_print_header(strm);
#endif
                strm->header.blocksize = 4 + 4 + 
                    ((strm->header.flags & F_ADLER32_D)?4:0) +
                    ((strm->header.flags & F_CRC32_D)?4:0) +
                    ((strm->header.flags & F_ADLER32_C)?4:0) +
                    ((strm->header.flags & F_CRC32_C)?4:0) ;
                strm->data.insize = 0;
                PD("Inflate blocksize: 0x%08lX\n", strm->header.blocksize);
            }
        }

        if (strm->header.ready){
            /*
             * lzop - Block
             *    uint32 - src_len (uncompressed data len)
             *    uint32 - dst_len (compressed block size)
             *    uint32 - src_chk (ADLER32 or CRC32)
             *    uint32 - dst_chk (ADLER32 or CRC32)
             *    char[] - compressed block data
             *
             */
            /* if src_len / dst_len == 0 we need to retrieve the block header */
            if (strm->data.dst_len == 0){
                if (_lzop_fillbuffer_in(strm, strm->header.blocksize) < strm->header.blocksize){
                    return LZOP_OK;
                }else{
                    size_t offset = 0;
                    strm->data.src_len = fromBe32(*(uint32_t*)(&strm->data.inbuf[offset]));
                    offset += 4;
                    strm->data.dst_len = fromBe32(*(uint32_t*)(&strm->data.inbuf[offset]));
                    offset += 4;
                    if ((strm->header.flags & F_ADLER32_D)){
                        strm->data.src_adler32 = fromBe32(*(uint32_t*)(&strm->data.inbuf[offset]));
                        offset += 4;
                    }
                    if ((strm->header.flags & F_CRC32_D)){
                        strm->data.src_crc32 = fromBe32(*(uint32_t*)(&strm->data.inbuf[offset]));
                        offset += 4;
                    }
                    if ((strm->header.flags & F_ADLER32_C)){
                        strm->data.dst_adler32 = fromBe32(*(uint32_t*)(&strm->data.inbuf[offset]));
                        offset += 4;
                    }
                    if ((strm->header.flags & F_CRC32_C)){
                        strm->data.dst_crc32 = fromBe32(*(uint32_t*)(&strm->data.inbuf[offset]));
                        offset += 4;
                    }
                    strm->data.insize = 0;
                    PD("Inflate src_len: 0x%08X\n", strm->data.src_len);
                    PD("Inflate dst_len: 0x%08X\n", strm->data.dst_len);
                    PD("Inflate src_adler32: 0x%08X\n", strm->data.src_adler32);
                    PD("Inflate src_crc32: 0x%08X\n", strm->data.src_crc32);
                    PD("Inflate dst_adler32: 0x%08X\n", strm->data.dst_adler32);
                    PD("Inflate dst_crc32: 0x%08X\n", strm->data.dst_crc32);
                }
            }

            if (strm->data.dst_len != 0){
                size_t inlen = strm->data.dst_len < strm->data.src_len ? strm->data.dst_len : strm->data.src_len ;
                if (_lzop_fillbuffer_in(strm, inlen) < inlen){
                    return LZOP_OK;
                }else{
                    if (strm->data.dst_len < strm->data.src_len){
                        int r = lzo1x_decompress(strm->data.inbuf, strm->data.dst_len, strm->data.outbuf, &strm->data.outsize, NULL);
                    }else{
                        memcpy(strm->data.outbuf, strm->data.inbuf, strm->data.insize);
                        strm->data.outsize = strm->data.insize;
                    }
                    strm->data.insize = 0;
                    strm->data.dst_len = 0;
                }
            }
        }
    }
    return LZOP_OK;
}

LZOP_STATUS lzop_deflate(lzop_streamp strm, LZOP_FLUSH_TYPE flush){
    size_t out_offset = 0;
    while(strm->avail_in > 0 || strm->avail_out > 0 ){
        /* check if there are left bytes that can be copyed in the avail_out */
        if ( strm->avail_out > 0 && strm->data.outsize ){
            size_t toBeCopyed = strm->data.outsize > strm->avail_out ? strm->avail_out : strm->data.outsize;
            memcpy(strm->next_out+out_offset, strm->data.outbuf, toBeCopyed);
#if 0
            PD("Deflate 001 Copy: avail_out:%ld, outsize:%ld, tbc:%ld off: %lx - %02x %02x %02x %02x %02x %02x %02x %02x\n",
                strm->avail_out, strm->data.outsize,
                toBeCopyed,
                out_offset,
                strm->next_out[out_offset+0],
                strm->next_out[out_offset+1],
                strm->next_out[out_offset+2],
                strm->next_out[out_offset+3],
                strm->next_out[out_offset+4],
                strm->next_out[out_offset+5],
                strm->next_out[out_offset+6],
                strm->next_out[out_offset+7]
                    );
#endif
            strm->avail_out -= toBeCopyed;
            out_offset += toBeCopyed;
            strm->data.outsize -= toBeCopyed;
            if(strm->data.outsize > 0){
                memcpy(strm->data.outbuf, strm->data.outbuf+toBeCopyed, strm->data.outsize);
            }

            if (strm->avail_out==0){
                return LZOP_OK;
            }
            if (LZOP_FLUSH == flush && strm->data.outsize == 0 && strm->data.insize == 0){
                return LZOP_OK;
            }
        }
        //PD("Deflate 001 avail_in: %ld  h_ready: %d  dst_len: %d\n", strm->avail_in, strm->header.ready, strm->data.dst_len);
        /* phase 1, decode the header */
        if (!strm->header.ready){
            if (LZOP_OK != _lzop_header_write(strm)){
                return LZOP_ERROR;
            }
        }

        if (strm->header.ready){
            /*
             * lzop - Block
             *    uint32 - src_len (uncompressed data len)
             *    uint32 - dst_len (compressed block size)
             *    uint32 - src_chk (ADLER32 or CRC32)
             *    char[] - compressed block data
             *
             */
            /* if src_len / dst_len == 0 we need to retrieve the block header */
            switch(flush){
                case LZOP_FLUSH:
                    _lzop_fillbuffer_in(strm, strm->data.insize + strm->avail_in);
                    break;
                case LZOP_NO_FLUSH:
                    if (strm->data.insize < ZBUFSIZELZOP_IN){
                        if (_lzop_fillbuffer_in(strm, ZBUFSIZELZOP_IN) < ZBUFSIZELZOP_IN){
                            return LZOP_OK;
                        }
                    }
                    break;
            }

            if (LZOP_FLUSH == flush || strm->data.insize == ZBUFSIZELZOP_IN){
                size_t outsize;
                if(LZO_E_OK != lzo1x_999_compress_level(
                            strm->data.inbuf,   strm->data.insize,
                            strm->data.outbuf + strm->data.outsize+(3*4), &outsize,
                            strm->data.wrkmem,
                            NULL, 0, 0, strm->header.level)){
                    return LZOP_ERROR;
                }
                strm->data.src_adler32 = lzo_adler32(ADLER32_INIT_VALUE, (lzo_bytep)strm->data.inbuf, strm->data.insize);
                *(uint32_t*)(&strm->data.outbuf[strm->data.outsize]) = toBe32(strm->data.insize);
                strm->data.outsize += 4;
                *(uint32_t*)(&strm->data.outbuf[strm->data.outsize]) = toBe32(outsize);
                strm->data.outsize += 4;
                *(uint32_t*)(&strm->data.outbuf[strm->data.outsize]) = toBe32(strm->data.src_adler32);
                strm->data.outsize += 4;
                //memcpy(&strm->data.outbuf[strm->data.outsize], strm->data.outbuf, outsize);
                PD("Deflate 010, insize :%ld, outsize:%ld\n", strm->data.insize, outsize);
                strm->data.outsize += outsize;
                strm->data.insize = 0;
                if (LZOP_FLUSH == flush){
                    /* ENDFILE */
                    *(uint32_t*)(&strm->data.outbuf[strm->data.outsize]) = 0;
                    strm->data.outsize += 4;
                }
            }
        }
    }
    return LZOP_OK;
}
