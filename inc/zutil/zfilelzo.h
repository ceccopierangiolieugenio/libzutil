/* zfilelzo.h -- "zutil"

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

#ifndef ZFILELZO_H
#define ZFILELZO_H

#include <stdint.h>

#include <zutil/lzop.h>

#include <zutil/zfile.h>


class ZFileLZO: public ZFile
{
public:
    ZFileLZO();
    ~ZFileLZO();

    size_t write (const char* s, size_t n);
    size_t read (char* s, size_t n);
    void open(const char* filename, std::ios_base::openmode mode);
    void close();

private:
    lzop_stream strm;
    uint8_t * inbuf;
    uint8_t * outbuf;
    size_t offsetbuf;
    int status;
};

#endif // ZFILELZO_H
