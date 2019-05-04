/* zfilegz.cpp -- "zutil"

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

#include <iostream>
#include <cstring>

#include <zutil/zfilegz.h>

#ifdef TEST_BUFFER
#define ZBUFSIZEGZIP ( TEST_BUFFER )
#else
#define ZBUFSIZEGZIP (0x1000 * 0x80) /* 512k */
#endif

#define DEBUG

#ifdef DEBUG
#define PD(_d) do { std::cout << " #(gz) " << _d ;}while(0)
#else
#define PD(_d) do {;}while(0)
#endif

ZFileGZ::ZFileGZ()
    : inbuf(nullptr), outbuf(nullptr)
{
    this->inbuf = new uint8_t[ZBUFSIZEGZIP];
    this->outbuf = new uint8_t[ZBUFSIZEGZIP];
};

ZFileGZ::~ZFileGZ(){
    if (this->inbuf)  delete[] this->inbuf;
    if (this->outbuf) delete[] this->outbuf;
};

void ZFileGZ::open(const char* filename, std::ios_base::openmode mode){
    ZFile::open(filename, mode);
    if (this->mode == std::ios_base::in){
        /* allocate inflate state */
        this->offsetbuf = 0;
        this->status = Z_OK;
        this->strm.zalloc = nullptr;
        this->strm.zfree = nullptr;
        this->strm.opaque = nullptr;
        this->strm.next_in = nullptr;
        this->strm.avail_in = 0;
        this->strm.next_out = this->outbuf;
        this->strm.avail_out = ZBUFSIZEGZIP;
        if(Z_OK != inflateInit2(&this->strm, (15 + 32))){
            std::cerr << "Error initializing the decoder!\n";
            throw "Decoder Not initialized!";
        }
    }
    if (this->mode == std::ios_base::out){
        this->strm.zalloc = nullptr;
        this->strm.zfree = nullptr;
        this->strm.opaque = nullptr;
        this->strm.next_in = nullptr;
        this->strm.avail_in = 0;
        this->strm.next_out = this->outbuf;
        this->strm.avail_out = ZBUFSIZEGZIP;
        const int windowsBits = 15;
        const int GZIP_ENCODING = 16;

        //deflateInit(&this->strm, 9);

        deflateInit2 (&this->strm, Z_BEST_COMPRESSION, Z_DEFLATED,
                      windowsBits | GZIP_ENCODING,
                      8,
                      Z_DEFAULT_STRATEGY);
    }
}

void ZFileGZ::close(){
    if (this->mode == std::ios_base::out){
        int ret = deflate(&this->strm,Z_FINISH);
        // PD("D [close](out) deflate_end:"<< ret << "avail_in:"<<this->strm.avail_in<<" avail_out:"<<this->strm.avail_out<<std::endl);
        if (this->strm.avail_out != ZBUFSIZEGZIP) {
            size_t write_size = ZBUFSIZEGZIP - this->strm.avail_out;
            this->fs.write((char*)(this->outbuf), write_size);
        }
        ret = deflateEnd(&this->strm);
        PD("D [close](out) deflate_end:"<< ret << "avail_in:"<<this->strm.avail_in<<" avail_out:"<<this->strm.avail_out<<std::endl);
    }else{
        PD("D [close](in) inflate_end");
        (void)inflateEnd(&this->strm);
    }
    ZFile::close();
}

size_t ZFileGZ::write (const char* s, size_t n){
    if (this->mode != std::ios_base::out){
        // Error, Not possible to read here
        return 0;
    }
    size_t s_offset = 0;

    while (true) {
        //PD("D 001 ZBUFSIZEGZIP:"<<ZBUFSIZEGZIP<<" n:"<<n<<" avail_in:"<<this->strm.avail_in<<" avail_out:"<<this->strm.avail_out<<std::endl);
        if (this->strm.avail_in == 0 && 0 != n) {
            size_t copy_size = ZBUFSIZEGZIP > n ? n : ZBUFSIZEGZIP;
            std::memcpy(this->inbuf, s + s_offset, copy_size);
            this->strm.avail_in = copy_size;
            s_offset += copy_size;
            n -= copy_size;
        }

        this->strm.next_in = this->inbuf;
        this->strm.next_out = this->outbuf;

        PD("D 002 eof:"<<this->fs.eof()<<" avail_in:"<<this->strm.avail_in<<" avail_out:"<<this->strm.avail_out<<std::endl);

        while (this->strm.avail_in){
            int ret = deflate(&strm, Z_NO_FLUSH);
            PD("<--- D 010 eof:"<<this->fs.eof()<<" gzip_ret:"<<ret<<" avail_in:"<<this->strm.avail_in<<" avail_out:"<<this->strm.avail_out<<std::endl);

            if (ret != Z_OK) {
                return 0;
            }

            if (this->strm.avail_out != ZBUFSIZEGZIP) {
                size_t write_size = ZBUFSIZEGZIP - this->strm.avail_out;
                this->fs.write((char*)(this->outbuf), write_size);
                this->strm.avail_out = ZBUFSIZEGZIP;
                this->strm.next_out = this->outbuf;
            }
        }

        if (0 == n)
            return s_offset;

    }
}
/*
 * Decompress Routine taken from:
 *   https://www.zlib.net/zlib_how.html
 */
size_t ZFileGZ::read (char* s, size_t n){
    if (this->mode != std::ios_base::in){
        // Error, Not possible to write here
        return 0;
    }

    size_t s_offset = 0;

    while (true) {
        /*
         * 1) Check the available bytes in the outbuf to be copyed and
         *    return them if enough;
         *  this->strm.next_out   = [xxxxxxxxxx------]
         *  this->strm.avail_out  =            <---->
         *  this->offsetbuf            ^
         */
        size_t out_size = ZBUFSIZEGZIP - this->strm.avail_out - this->offsetbuf;
        size_t copy_size = out_size > n ? n : out_size;

        std::memcpy(s + s_offset, this->outbuf + this->offsetbuf, copy_size);

        this->offsetbuf += copy_size;
        s_offset += copy_size;
        n -= copy_size;

        PD("D 001 eof:"<<this->fs.eof()<<" n:"<<n<<" out_size:"<<out_size
           <<" copy_size:"<<copy_size<<" s_offset:"<<s_offset<<std::endl);

        if (0 == n || Z_STREAM_END == this->status ){
            return s_offset;
        }

        /* the outbuf is empty and we need to fetch more data */
        this->offsetbuf = 0;
        this->strm.next_out = this->outbuf;
        this->strm.avail_out = ZBUFSIZEGZIP;

        PD("D 003 eof:"<<this->fs.eof()<<std::endl);

        if (this->strm.avail_in == 0 && !this->fs.eof()) {
            this->strm.next_in = this->inbuf;
             // read data as a block:
             this->fs.read((char*)(this->inbuf), ZBUFSIZEGZIP);

             if (this->fs){
                 this->strm.avail_in = ZBUFSIZEGZIP;
             }else{
                 this->strm.avail_in = this->fs.gcount();
             }
             PD("D 004 eof:"<<this->fs.eof()<<" avail_in:"<<this->strm.avail_in<<std::endl);
        }

        PD("D 009 eof:"<<this->fs.eof()<<" gzip_ret:"<<9<<" avail_in:"<<this->strm.avail_in<<" avail_out:"<<this->strm.avail_out<<std::endl);
        int ret = inflate(&strm, Z_NO_FLUSH);
        PD("D 010 eof:"<<this->fs.eof()<<" gzip_ret:"<<ret<<" avail_in:"<<this->strm.avail_in<<" avail_out:"<<this->strm.avail_out<<std::endl);

        switch (ret) {
            case Z_NEED_DICT:
                PD("D 015 Z_NEED_DICT!!!"<<std::endl);
                return 0;
            case Z_DATA_ERROR:
                PD("D 015 Z_DATA_ERROR!!!"<<std::endl);
                return 0;
            case Z_MEM_ERROR:
                // (void)inflateEnd(&this->strm);
                PD("D 015 Z_MEM_ERROR!!!"<<std::endl);
                return 0;
            case Z_STREAM_END:
                if (this->strm.avail_out > 0){
                    this->status = Z_STREAM_END;
                }
                continue;
        }
    }
    return 0;
}
