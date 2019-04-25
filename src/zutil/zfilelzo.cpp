/* zfilelzo.cpp -- "zutil"

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

#include <zutil/zfilelzo.h>

#ifdef TEST_BUFFER
#define ZBUFSIZELZO_IN      ( TEST_BUFFER )
#else
#define ZBUFSIZELZO_IN      (0x1000 * 0x80) /* 512k */
#endif
#define ZBUFSIZELZO_OUT     (ZBUFSIZELZO_IN + ZBUFSIZELZO_IN / 16 + 64 + 3)

// #define DEBUG

#ifdef DEBUG
#define PD(_d) do { std::cout << _d ;}while(0)
#else
#define PD(_d) do {;}while(0)
#endif

ZFileLZO::ZFileLZO()
    : inbuf(nullptr), outbuf(nullptr)
{
    this->inbuf = new uint8_t[ZBUFSIZELZO_IN];
    this->outbuf = new uint8_t[ZBUFSIZELZO_OUT];
};

ZFileLZO::~ZFileLZO(){
    if (this->inbuf)  delete[] this->inbuf;
    if (this->outbuf) delete[] this->outbuf;
};

void ZFileLZO::open(const char* filename, std::ios_base::openmode mode){
    PD("Open File:"<<filename<<std::endl);
    ZFile::open(filename, mode);
    if (this->mode == std::ios_base::in){
        /* allocate inflate state */
        this->offsetbuf = 0;
        this->strm.next_in = nullptr;
        this->strm.avail_in = 0;
        this->strm.next_out = this->outbuf;
        this->strm.avail_out = ZBUFSIZELZO_OUT;
        if(LZOP_OK != lzop_inflateInit(&this->strm)){
            std::cerr << "Error initializing the decoder!\n";
            throw "Decoder Not initialized!";
        }
    }
    if (this->mode == std::ios_base::out){
        this->strm.next_in = nullptr;
        this->strm.avail_in = 0;
        this->strm.next_out = this->outbuf;
        this->strm.avail_out = ZBUFSIZELZO_OUT;
        lzop_deflateInit(&this->strm, 9);
    }
}

void ZFileLZO::close(){
    if (this->mode == std::ios_base::out){
        int ret = lzop_deflate(&this->strm, LZOP_FLUSH);
        // PD("D [close](out) deflate_end:"<< ret << "avail_in:"<<this->strm.avail_in<<" avail_out:"<<this->strm.avail_out<<std::endl);
        if (this->strm.avail_out != ZBUFSIZELZO_OUT) {
            size_t write_size = ZBUFSIZELZO_OUT - this->strm.avail_out;
            this->fs.write((char*)(this->outbuf), write_size);
        }
        ret = lzop_deflateEnd(&this->strm);
        PD("D [close](out) deflate_end:"<< ret << "avail_in:"<<this->strm.avail_in<<" avail_out:"<<this->strm.avail_out<<std::endl);
    }else{
        PD("D [close](in) inflate_end");
        (void)lzop_inflateEnd(&this->strm);
    }
    ZFile::close();
}

size_t ZFileLZO::write (const char* s, size_t n){
    if (this->mode != std::ios_base::out){
        // Error, Not possible to read here
        return 0;
    }
    size_t s_offset = 0;

    while (true) {
        //PD("D 001 ZBUFSIZEXZ:"<<ZBUFSIZEXZ<<" avail_in:"<<this->strm.avail_in<<" avail_out:"<<this->strm.avail_out<<std::endl);
        if (this->strm.avail_in == 0 && 0 != n) {
            size_t copy_size = ZBUFSIZELZO_IN > n ? n : ZBUFSIZELZO_IN;
            std::memcpy(this->inbuf, s + s_offset, copy_size);
            this->strm.avail_in = copy_size;
            s_offset += copy_size;
            n -= copy_size;
        }

        this->strm.next_in = this->inbuf;
        this->strm.next_out = this->outbuf;

        PD("D 002 eof:"<<this->fs.eof()<<" avail_in:"<<this->strm.avail_in<<" avail_out:"<<this->strm.avail_out<<std::endl);
        //PD("D 003 "<<" next_in:"<< &((int*)this->strm.next_in[0]) <<" next_out:"<<&((int*)this->strm.next_out[0])<<std::endl);
        int ret = lzop_deflate(&strm, LZOP_NO_FLUSH);
        PD("D 010 eof:"<<this->fs.eof()<<" lzo_ret:"<<ret<<" avail_in:"<<this->strm.avail_in<<" avail_out:"<<this->strm.avail_out<<std::endl);

        if (this->strm.avail_out != ZBUFSIZELZO_OUT) {
            size_t write_size = ZBUFSIZELZO_OUT - this->strm.avail_out;
            this->fs.write((char*)(this->outbuf), write_size);
            this->strm.avail_out = ZBUFSIZELZO_OUT;
            this->strm.next_out = this->outbuf;
        }

        if (0 == n)
            return s_offset;

        if (ret != LZOP_OK) {
        }
    }
    return 0;
}

size_t ZFileLZO::read (char* s, size_t n){
    if (this->mode != std::ios_base::in){
        // Error, Not possible to write here
        return 0;
    }

    size_t s_offset = 0;

    while (true) {
        size_t out_size = ZBUFSIZELZO_OUT - this->strm.avail_out - this->offsetbuf;
        size_t copy_size = out_size > n ? n : out_size;

        std::memcpy(s + s_offset, this->outbuf + this->offsetbuf, copy_size);

        this->offsetbuf += copy_size;
        s_offset += copy_size;
        n -= copy_size;

        PD("D 001 eof:"<<this->fs.eof()<<" out_size:"<<out_size
           <<" copy_size:"<<copy_size<<" s_offset:"<<s_offset<<std::endl);
        PD("D 002 n:"<<n<<" avail_in:"<<this->strm.avail_in<<" avail_out"<<this->strm.avail_out<<std::endl);
        if (0 == n || (this->fs.eof() && 0 == this->strm.avail_in)){
            return s_offset;
        }

        /* the outbuf is empty and we need to fetch more data */
        this->offsetbuf = 0;
        this->strm.next_out = this->outbuf;        
        this->strm.avail_out = ZBUFSIZELZO_OUT;

        PD("D 003 eof:"<<this->fs.eof()<<std::endl);

        if (this->strm.avail_in == 0 && !this->fs.eof()) {
             this->strm.next_in = this->inbuf;
            // read data as a block:
             this->fs.read((char*)(this->inbuf), ZBUFSIZELZO_IN);

             if (this->fs){
                 this->strm.avail_in = ZBUFSIZELZO_IN;
             }else{
                 this->strm.avail_in = this->fs.gcount();
             }

             PD("D 004 eof:"<<this->fs.eof()<<" in_len:"<<this->strm.avail_in<<std::endl);
        }

        int ret = lzop_inflate(&this->strm);

        PD("D 009 eof:"<<this->fs.eof()<<" in_len:"<< this->strm.avail_out <<std::endl);
        PD("D 010 eof:"<<this->fs.eof()<<" lzma_ret:"<<ret<<std::endl);

        if (ret != LZOP_OK) {
            throw "Inflate Error!";
        }
    }
    return 0;
}
