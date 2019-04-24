/* zfile.h -- "zutil"

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

#ifndef ZFILE_H
#define ZFILE_H

#include <iostream>
#include <ostream>
#include <istream>
#include <fstream>

class ZFile
{
public:
    virtual ~ZFile(){}
    virtual void open(const char* filename, std::ios_base::openmode mode);
    virtual void close();

    virtual size_t write (const char* s, size_t n) = 0;
    virtual size_t read (char* s, size_t n) = 0;
    virtual bool eof() const;

protected:
    std::fstream fs;
    std::ios_base::openmode mode;
    std::string filename;
};

#endif // _ZFILE_H
