/* zfile.cpp -- "zutil"

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

#include <zutil/zfile.h>

// #define DEBUG

#ifdef DEBUG
#define PD(_d) do { std::cout << " #(zfile) " << _d ;}while(0)
#else
#define PD(_d) do {;}while(0)
#endif

//ZFile::ZFile(){}
//ZFile::~ZFile(){}

void ZFile::open(const char* filename, std::ios_base::openmode mode){
    PD("D [open]");
    this->mode = std::ios_base::app;
    switch(mode){
        case std::ios_base::in:
            break;
        case std::ios_base::out:
            break;
        default:
            std::cerr << "ERROR: Only ios_base::in or ios_base::out are supported!!!";
            // ERROR!!!
            return;
    }
    this->mode = mode;
    this->fs.open (filename, mode | std::ios_base::binary);
}

void ZFile::close(){
    PD("D [close]");
    this->fs.close();
}

bool ZFile::eof() const{
    return this->fs.eof();
}
