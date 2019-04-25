/* main.cpp -- TEST "zutil"

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

#include <string.h>

#include <zutil/zfilexz.h>
#include <zutil/zfilegz.h>
#include <zutil/zfilelzo.h>


using namespace std;

int test_inflate_txt_001(ZFile *zf, const char * filename) 
{
	/* test inflate */
	zf->open(filename, std::ios_base::in);

	const int bufsize = ( 1024 );
	char * buf = new char[bufsize + 1];
	buf[bufsize] = '\0';
	size_t size;
	size_t total = 0;

	std::cout << "#### Begin ####" << std::endl ;
	while ((size = zf->read(buf, bufsize))){
		total += size;
		std::cout << buf ;
		memset(buf,0,bufsize + 1);
	}
	std::cout << "####  End  ####" << std::endl ;
	std::cout << "total: " << total << std::endl ;

	zf->close();
	delete[] buf;
}

int test_inflate_001(ZFile *zf, const char * filename)
{
	/* test inflate */
	zf->open(filename, std::ios_base::in);

	const int bufsize = ( 1024 );
	char * buf = new char[bufsize + 1];
	buf[bufsize] = '\0';
	size_t size;
	size_t total = 0;

	std::cout << "#### Begin ####" << std::endl ;
	while ((size = zf->read(buf, bufsize))){
		total += size;
		memset(buf,0,bufsize + 1);
	}
	std::cout << "####  End  ####" << std::endl ;
	std::cout << "total: " << total << std::endl ;

	zf->close();
	delete[] buf;
}

int test_deflate_001(ZFile *zf, const char * infilename, const char * outfilename)
{
	/* Test XZ deflate */
	ZFileXZ zxz;
	zf->open(outfilename, std::ios_base::out);
	std::ifstream infile (infilename, std::ifstream::binary);

	const int bufsize = ( 1024 * 1024 ); /* 1M */
	char * buf = new char[bufsize];

	bool done = false;
	do {
		infile.read(buf, bufsize);
		if (infile){
			// std::cout << "001 bufsize:"<<bufsize<< std::endl ;
			zf->write(buf, bufsize);
		}else{
			int s = infile.gcount();
			// std::cout << "001 gcount:"<<s<< std::endl ;
			zf->write(buf, s);
			done = true;
		}
	}while (!done);

	zf->close();
	infile.close();
	delete[] buf;
}


int test_compress_001_lzo() 
{
	/* Test XZ deflate */
	ZFileLZO zlzo;
	zlzo.open("test.big.txt.zutil.lzo", std::ios_base::out);
	std::ifstream infile ("test.big.txt",std::ifstream::binary);


	const int bufsize = ( 1024 * 1024 ); /* 1M */
	char * buf = new char[bufsize];

	bool done = false;
	do {
		infile.read(buf, bufsize);
		if (infile){
			// std::cout << "001 bufsize:"<<bufsize<< std::endl ;
			zlzo.write(buf, bufsize);
		}else{
			int s = infile.gcount();
			// std::cout << "001 gcount:"<<s<< std::endl ;
			zlzo.write(buf, s);
			done = true;
		}
	}while (!done);

	zlzo.close();
	infile.close();
	delete[] buf;
}

int test_compress_001_xz() 
{
	/* Test XZ deflate */
	ZFileXZ zxz;
	zxz.open("test.big.bin.zutil.xz", std::ios_base::out);
	std::ifstream infile ("test.big.bin",std::ifstream::binary);


	const int bufsize = ( 1024 * 1024 ); /* 1M */
	char * buf = new char[bufsize];

	bool done = false;
	do {
		infile.read(buf, bufsize);
		if (infile){
			// std::cout << "001 bufsize:"<<bufsize<< std::endl ;
			zxz.write(buf, bufsize);
		}else{
			int s = infile.gcount();
			// std::cout << "001 gcount:"<<s<< std::endl ;
			zxz.write(buf, s);
			done = true;
		}
	}while (!done);

	zxz.close();
	infile.close();
	delete[] buf;
}

int test_compress_002_xz(char * filename, int preset) 
{
	/* Test XZ deflate */
	ZFileXZ::options opt;
	opt.preset = preset;
	ZFileXZ zxz(opt);
	zxz.open(filename, std::ios_base::out);
	std::ifstream infile ("test.big.bin",std::ifstream::binary);


	const int bufsize = ( 1024 * 1024 * 4); /* 4M */
	char * buf = new char[bufsize];

	bool done = false;
	do {
		infile.read(buf, bufsize);
		if (infile){
			// std::cout << "001 bufsize:"<<bufsize<< std::endl ;
			zxz.write(buf, bufsize);
		}else{
			int s = infile.gcount();
			// std::cout << "001 gcount:"<<s<< std::endl ;
			zxz.write(buf, s);
			done = true;
		}
	}while (!done);

	zxz.close();
	infile.close();
	delete[] buf;
}

int test_compress_003_xz(char * filename, int preset) 
{
	/* Test XZ deflate */
	ZFileXZ::options opt;
	opt.preset = preset;
	opt.dict_size = (1024*1024*32); /* 32 MB */
	opt.chk = ZFileXZ::options::crc32;
	opt.filter = ZFileXZ::options::arm;
	ZFileXZ zxz(opt);
	zxz.open(filename, std::ios_base::out);
	std::ifstream infile ("test.big.bin",std::ifstream::binary);


	const int bufsize = ( 1024 * 1024 * 4); /* 4M */
	char * buf = new char[bufsize];

	bool done = false;
	do {
		infile.read(buf, bufsize);
		if (infile){
			// std::cout << "001 bufsize:"<<bufsize<< std::endl ;
			zxz.write(buf, bufsize);
		}else{
			int s = infile.gcount();
			// std::cout << "001 gcount:"<<s<< std::endl ;
			zxz.write(buf, s);
			done = true;
		}
	}while (!done);

	zxz.close();
	infile.close();
	delete[] buf;
}

int test_compress_004_xz() 
{
	char * inf = "root.cpio";
	char * outf = "root.cpio.zutil.xz";
	/* Test XZ deflate */
	ZFileXZ::options opt;
	opt.preset = 9;
	//opt.dict_size = LZMA_DICT_SIZE_DEFAULT*4; /* 32 MB */
	//opt.chk = ZFileXZ::options::crc32;
	//opt.filter = ZFileXZ::options::arm;
	ZFileXZ zxz(opt);
	zxz.open(outf, std::ios_base::out);
	std::ifstream infile (inf,std::ifstream::binary);


	const int bufsize = ( 1024 * 1024 * 4); /* 4M */
	char * buf = new char[bufsize];

	bool done = false;
	do {
		infile.read(buf, bufsize);
		if (infile){
			// std::cout << "001 bufsize:"<<bufsize<< std::endl ;
			zxz.write(buf, bufsize);
		}else{
			int s = infile.gcount();
			// std::cout << "001 gcount:"<<s<< std::endl ;
			zxz.write(buf, s);
			done = true;
		}
	}while (!done);

	zxz.close();
	infile.close();
	delete[] buf;
}

int test_compress_002_gz(char * filename, int preset) 
{
	/* Test XZ deflate */
	ZFileGZ zxz;
	zxz.open(filename, std::ios_base::out);
	std::ifstream infile ("test.big.bin",std::ifstream::binary);


	const int bufsize = ( 1024 * 1024 * 4); /* 4M */
	char * buf = new char[bufsize];

	bool done = false;
	do {
		infile.read(buf, bufsize);
		if (infile){
			//std::cout << "001 bufsize:"<<bufsize<< std::endl ;
			zxz.write(buf, bufsize);
		}else{
			int s = infile.gcount();
			//std::cout << "001 gcount:"<<s<< std::endl ;
			zxz.write(buf, s);
			done = true;
		}
	}while (!done);

	zxz.close();
	infile.close();
	delete[] buf;
}

int main(){
	//std::cout << "Test xz:" << std::endl ;
	//test_simple_001_xz();

	//std::cout << "Test gzip:" << std::endl ;
	//test_simple_001_gzip();

/*
	std::cout << "Test xz Compression 001:" << std::endl ;
	test_compress_001_xz();

	std::cout << "Test xz Compression 002:" << std::endl ;

	test_compress_002_xz("test.big.bin.1.zutil.xz", 1);
	test_compress_002_xz("test.big.bin.2.zutil.xz", 2);
	test_compress_002_xz("test.big.bin.3.zutil.xz", 3);
	test_compress_002_xz("test.big.bin.4.zutil.xz", 4);
	test_compress_002_xz("test.big.bin.5.zutil.xz", 5);
	test_compress_002_xz("test.big.bin.6.zutil.xz", 6);
	test_compress_002_xz("test.big.bin.7.zutil.xz", 7);
	test_compress_002_xz("test.big.bin.9.zutil.xz", 9);

	test_compress_003_xz("test.big.bin.9.arm.zutil.xz", 9);
	
	test_compress_002_gz("test.big.bin.9.zutil.gz", 9);
	
	test_compress_004_xz();
*/
	std::cout << "Test lzo:" << std::endl ;
	ZFileLZO *zlo = new ZFileLZO();
	test_inflate_txt_001(zlo, "test.lzo");
	delete zlo;
	std::cout << "          ---END---" << std::endl ;
/*
	std::cout << "Test big lzo:" << std::endl ;
	zlo = new ZFileLZO();
	test_inflate_txt_001(zlo, "test.big.txt.lzo");
	delete zlo;
	std::cout << "          ---END---" << std::endl ;
*/

	std::cout << "Test Deflate lzo:" << std::endl ;
	zlo = new ZFileLZO();
	test_deflate_001(zlo, "test.big.txt", "test.big.txt.zutil.lzo");
	delete zlo;
	std::cout << "          ---END---" << std::endl ;

	return 0;
}

