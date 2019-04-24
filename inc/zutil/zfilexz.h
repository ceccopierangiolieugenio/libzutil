/* zfilexz.h -- "zutil"

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

#ifndef ZFILEXZ_H
#define ZFILEXZ_H

#include <lzma.h>

#include <zutil/zfile.h>

/* Use custom allocator for the lzma lib */
// #define XZ_ALLOCATOR


class ZFileXZ: public ZFile
{
public:
    struct options{
        uint32_t preset;
        uint32_t dict_size;
        enum CHK{
            none = LZMA_CHECK_NONE,
            crc32 = LZMA_CHECK_CRC32,
            crc64 = LZMA_CHECK_CRC64, /* default */
            sha256 = LZMA_CHECK_SHA256
        } chk;
        enum FILTER{
            lzma2, /* default */
            arm,
            x86
        } filter;
        options():
            preset(LZMA_PRESET_DEFAULT /* 6 */),
            dict_size(LZMA_DICT_SIZE_DEFAULT /* 8M */),
            chk(crc64),
            filter(lzma2){}
    };

    ZFileXZ(const ZFileXZ::options &opt);
    ZFileXZ();
    ~ZFileXZ();

    size_t write (const char* s, size_t n);
    size_t read (char* s, size_t n);
    void open(const char* filename, std::ios_base::openmode mode);
    void close();

private:
#ifdef XZ_ALLOCATOR 
    static void *_alloc(void *opaque, size_t nmemb, size_t size);
    static void _free(void *opaque, void *ptr);
    lzma_allocator allocator;
#endif /* XZ_ALLOCATOR */
    lzma_stream strm = LZMA_STREAM_INIT;
    uint8_t * inbuf;
    uint8_t * outbuf;
    size_t offsetbuf;
    lzma_action action;
    lzma_ret status;
    lzma_filter * filters;
    ZFileXZ::options opt;
    lzma_options_lzma opt_lzma2;
};

#endif // ZFILEXZ_H
