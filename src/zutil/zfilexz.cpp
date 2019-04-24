/* zfilexz.cpp -- "zutil"

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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <iostream>
#include <cstring>

#include <zutil/zfilexz.h>


#ifdef TEST_BUFFER
#define ZBUFSIZEXZ ( TEST_BUFFER )
#else
#define ZBUFSIZEXZ (0x1000 * 0x80) /* 512k */
#endif

//#define DEBUG

#ifdef DEBUG
#define PD(_d) do { std::cout << " #(xz) " << _d ;}while(0)
constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
std::string hexStr(unsigned char *data, int len)
{
  std::string s(len * 2, ' ');
  for (int i = 0; i < len; ++i) {
    s[2 * i]     = hexmap[(data[i] & 0xF0) >> 4];
    s[2 * i + 1] = hexmap[data[i] & 0x0F];
  }
  return s;
}
#else
#define PD(_d) do {;}while(0)
#endif

ZFileXZ::ZFileXZ(const ZFileXZ::options &opt)
    :inbuf(nullptr), outbuf(nullptr), filters(nullptr), opt(opt)
{
    this->inbuf = new uint8_t[ZBUFSIZEXZ];
    this->outbuf = new uint8_t[ZBUFSIZEXZ];
#ifdef XZ_ALLOCATOR
    this->allocator.alloc  = ZFileXZ::_alloc;
    this->allocator.free   = ZFileXZ::_free;
    this->allocator.opaque = this;
    this->strm.allocator = &this->allocator;
#endif /* XZ_ALLOCATOR */
}

ZFileXZ::ZFileXZ()
    : inbuf(nullptr), outbuf(nullptr), filters(nullptr)
{
    this->inbuf = new uint8_t[ZBUFSIZEXZ];
    this->outbuf = new uint8_t[ZBUFSIZEXZ];
#ifdef XZ_ALLOCATOR
    this->allocator.alloc  = ZFileXZ::_alloc;
    this->allocator.free   = ZFileXZ::_free;
    this->allocator.opaque = this;
    this->strm.allocator = &this->allocator;
#endif /* XZ_ALLOCATOR */
}


ZFileXZ::~ZFileXZ(){
    lzma_end(&this->strm);
    if (this->inbuf)   delete[] this->inbuf;
    if (this->outbuf)  delete[] this->outbuf;
    if (this->filters) delete[] this->filters;
}

#ifdef XZ_ALLOCATOR
void *ZFileXZ::_alloc(void *opaque, size_t nmemb, size_t size){
    (void) opaque;
    PD("D [_alloc] nmemb:"<<nmemb<<" size:"<<size<<std::endl);
    void *ret = malloc(nmemb*size);
    PD("D [_alloc] ret:" << ret <<std::endl);
    return ret;
}

void ZFileXZ::_free(void *opaque, void *ptr){
    (void) opaque;
    free(ptr);
}
#endif

void ZFileXZ::open(const char* filename, std::ios_base::openmode mode){
    ZFile::open(filename, mode);
    if (this->mode == std::ios_base::in){
        lzma_ret ret = lzma_stream_decoder(
                &this->strm, UINT64_MAX, LZMA_CONCATENATED);

        if (ret != LZMA_OK){
            const char *msg;
            switch (ret) {
            case LZMA_MEM_ERROR:
                msg = "Memory allocation failed";
                break;
            case LZMA_OPTIONS_ERROR:
                msg = "Unsupported decompressor flags";
                break;
            default:
                msg = "Unknown error, possibly a bug";
                break;
            }
            std::cerr << "Error initializing the decoder: " << msg << "(error code " << ret <<")" << std::endl;
            throw "Decoder Not initialized!";
        }

        this->offsetbuf = 0;
        this->action = LZMA_RUN;
        this->status = LZMA_OK;
        this->strm.next_in = nullptr;
        this->strm.avail_in = 0;
        this->strm.next_out = this->outbuf;
        this->strm.avail_out = ZBUFSIZEXZ;
    }
    if (this->mode == std::ios_base::out){
        PD("D preset:"<<this->opt.preset<<" dict:"<<this->opt.dict_size<<std::endl);

        if (lzma_lzma_preset(&this->opt_lzma2, this->opt.preset)) {
            std::cerr << "Unsupported preset, possibly a bug" << std::endl;
            throw "Unsupported preset, possibly a bug";
        }
        if (this->opt.dict_size != LZMA_DICT_SIZE_DEFAULT){
            this->opt_lzma2.dict_size = this->opt.dict_size;
        }

        /*
         * TODO:
         * FIX this HUGE amount of CRAP!!!
         */
        lzma_filter filters[] = {
            { .id = LZMA_VLI_UNKNOWN,  .options = nullptr },
            { .id = LZMA_FILTER_LZMA2, .options = &this->opt_lzma2 },
            { .id = LZMA_VLI_UNKNOWN,  .options = nullptr }
        };

        lzma_filter * pfilters = &filters[0];

        switch( this->opt.filter){
            case options::lzma2:
                pfilters = &filters[1];
                break;
            case options::arm:
                filters[0].id = LZMA_FILTER_ARM;
                filters[0].options = nullptr;
                break;
            case options::x86:
                filters[0].id = LZMA_FILTER_X86;
                filters[0].options = nullptr;
                break;

//            default:
//                pfilters = &filters[1];
//                break;

        }

        // Initialize the encoder using the custom filter chain.
        lzma_check chk = LZMA_CHECK_NONE;
        switch (this->opt.chk){
            case options::none:
                chk = LZMA_CHECK_NONE;
                break;
            case options::crc32:
                chk = LZMA_CHECK_CRC32;
                break;
            case options::crc64:
                chk = LZMA_CHECK_CRC64;
                break;
            case options::sha256:
                chk = LZMA_CHECK_SHA256;
                break;
        }
        lzma_ret ret = lzma_stream_encoder(&this->strm, pfilters, chk);

        if (ret != LZMA_OK){
            const char *msg;
            switch (ret) {
            case LZMA_MEM_ERROR:
                msg = "Memory allocation failed";
                break;

            case LZMA_OPTIONS_ERROR:
                // We are no longer using a plain preset so this error
                // message has been edited accordingly compared to
                // 01_compress_easy.c.
                msg = "Specified filter chain is not supported";
                break;

            case LZMA_UNSUPPORTED_CHECK:
                msg = "Specified integrity check is not supported";
                break;

            default:
                msg = "Unknown error, possibly a bug";
                break;
            }

            std::cerr << "Error initializing the encoder: " << msg << "(error code " << ret <<")" << std::endl;
            throw "Encoder Not initialized!";
        }
        this->action = LZMA_RUN;
        this->offsetbuf = 0;
        this->strm.next_in = nullptr;
        this->strm.avail_in = 0;
        this->strm.next_out = this->outbuf;
        this->strm.avail_out = ZBUFSIZEXZ;
    }
}

void ZFileXZ::close(){
    if (this->mode == std::ios_base::out){
        this->strm.next_in = this->inbuf;
        this->strm.next_out = this->outbuf;
        lzma_ret ret = lzma_code(&this->strm, LZMA_FINISH);
        if (this->strm.avail_out != ZBUFSIZEXZ || ret == LZMA_STREAM_END) {
            size_t write_size = ZBUFSIZEXZ - this->strm.avail_out;
            this->fs.write((char*)(this->outbuf), write_size);
        }
        PD("D [close](out)"<<std::endl);
    }else{
        PD("D [close](in)"<<std::endl);
    }
    lzma_end(&this->strm);
    ZFile::close();
}

/*
 * Decompress Routine taken from:
 *   https://github.com/kobolabs/liblzma/blob/master/doc/examples/03_compress_custom.c
 */
size_t ZFileXZ::write (const char* s, size_t n){
    if (this->mode != std::ios_base::out){
        // Error, Not possible to read here
        return 0;
    }

    size_t s_offset = 0;

    while (true) {
        //PD("D 001 ZBUFSIZEXZ:"<<ZBUFSIZEXZ<<" avail_in:"<<this->strm.avail_in<<" avail_out:"<<this->strm.avail_out<<std::endl);
        if (this->strm.avail_in == 0 && 0 != n) {
            size_t copy_size = ZBUFSIZEXZ > n ? n : ZBUFSIZEXZ;
            std::memcpy(this->inbuf, s + s_offset, copy_size);
            this->strm.avail_in = copy_size;
            s_offset += copy_size;
            n -= copy_size;
        }

        this->strm.next_in = this->inbuf;
        this->strm.next_out = this->outbuf;

        PD("D 002 n:"<<n<<" eof:"<<this->fs.eof()<<" avail_in:"<<this->strm.avail_in<<" avail_out:"<<this->strm.avail_out<<std::endl);
        lzma_ret ret = lzma_code(&this->strm, LZMA_RUN);
        PD("D 010 n:"<<n<<" eof:"<<this->fs.eof()<<" lzma_ret:"<<ret<<" avail_in:"<<this->strm.avail_in<<" avail_out:"<<this->strm.avail_out<<std::endl);

        if (this->strm.avail_out != ZBUFSIZEXZ || ret == LZMA_STREAM_END) {
            size_t write_size = ZBUFSIZEXZ - this->strm.avail_out;
            this->fs.write((char*)(this->outbuf), write_size);
            this->strm.avail_out = ZBUFSIZEXZ;
        }

        if (0 == n)
            return s_offset;

        if (ret != LZMA_OK) {
            if (ret == LZMA_STREAM_END)
                return s_offset;

            const char *msg;
            switch (ret) {
            case LZMA_MEM_ERROR:
                msg = "Memory allocation failed";
                break;

            case LZMA_DATA_ERROR:
                msg = "File size limits exceeded";
                break;

            case LZMA_PROG_ERROR:
                msg = "Options/Input params are invalid";
                break;

            case LZMA_BUF_ERROR:
                msg = "Buffer Error, No progress is possible";
                break;

            default:
                msg = "Unknown error, possibly a bug";
                break;
            }
            std::cerr << "Deflate error: " << msg << "(error code " << ret <<")\n" << std::endl;
            throw "Deflate Error!";
        }
    }
}

/*
 * Decompress Routine taken from:
 *   https://github.com/kobolabs/liblzma/blob/master/doc/examples/02_decompress.c
 */
size_t ZFileXZ::read (char* s, size_t n){
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
        size_t out_size = ZBUFSIZEXZ - this->strm.avail_out - this->offsetbuf;
        size_t copy_size = out_size > n ? n : out_size;

        std::memcpy(s + s_offset, this->outbuf + this->offsetbuf, copy_size);

        this->offsetbuf += copy_size;
        s_offset += copy_size;
        n -= copy_size;

        PD("D 001 eof:"<<this->fs.eof()<<" out_size:"<<out_size
           <<" copy_size:"<<copy_size<<" s_offset:"<<s_offset<<std::endl);

        if (0 == n || LZMA_STREAM_END == this->status ){
            return s_offset;
        }

        /* the outbuf is empty and we need to fetch more data */
        this->offsetbuf = 0;
        this->strm.next_out = this->outbuf;
        this->strm.avail_out = ZBUFSIZEXZ;

        PD("D 003 eof:"<<this->fs.eof()<<std::endl);

        if (this->strm.avail_in == 0 && !this->fs.eof()) {
            this->strm.next_in = this->inbuf;
             // read data as a block:
             this->fs.read((char*)(this->inbuf), ZBUFSIZEXZ);

             if (this->fs){
                 this->strm.avail_in = ZBUFSIZEXZ;
             }else{
                 this->strm.avail_in = this->fs.gcount();
             }

             PD("D 004 eof:"<<this->fs.eof()<<" avail_in:"<<this->strm.avail_in<<std::endl);

            if (this->fs.eof())
                this->action = LZMA_FINISH;
        }

        // PD("D 010 eof:"<<hexStr((unsigned char *)this->strm.next_in,this->strm.avail_in)<<std::endl);
        PD("D 009 eof:"<<this->fs.eof()<<" lzma_ret:X avail_in:"<<this->strm.avail_in<<" avail_out:"<<this->strm.avail_out<<std::endl);
        lzma_ret ret = lzma_code(&this->strm, this->action);
        PD("D 010 eof:"<<this->fs.eof()<<" lzma_ret:"<<ret<<" avail_in:"<<this->strm.avail_in<<" avail_out:"<<this->strm.avail_out<<std::endl);
        // PD("D 010 eof:"<<hexStr((unsigned char *)this->outbuf,ZBUFSIZEXZ-this->strm.avail_out)<<std::endl);

        if (ret != LZMA_OK) {
            if (ret == LZMA_STREAM_END || ret == LZMA_DATA_ERROR){
                if (this->strm.avail_out > 0){
                    this->status = LZMA_STREAM_END;
                }
                continue;
            }

            const char *msg;
            switch (ret) {
            case LZMA_MEM_ERROR:
                msg = "Memory allocation failed";
                break;

            case LZMA_FORMAT_ERROR:
                msg = "The input is not in the .xz format";
                break;

            case LZMA_OPTIONS_ERROR:
                msg = "Unsupported compression options";
                break;

            case LZMA_DATA_ERROR:
                msg = "Compressed file is corrupt";
                break;

            case LZMA_BUF_ERROR:
                msg = "Compressed file is truncated or "
                        "otherwise corrupt";
                break;

            default:
                msg = "Unknown error, possibly a bug";
                break;
            }

            std::cerr << "Inflate error: " << msg << "(error code " << ret <<")\n" << std::endl;
            throw "Inflate Error!";
        }
    }
}
