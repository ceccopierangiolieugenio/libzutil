#!/bin/sh
cat << __END > test.txt
Eugenio - Parodi
ceccopierangiolieugenio@googlemail.com
https://github.com/ceccopierangiolieugenio
ABCDEfghij
123456789
END-
__END

cat test.txt | xz -9 -c -v > test.xz
cat test.txt | gzip -9 -c -v     > test.gz
cat test.txt | lzop -9 -c -v     > test.lzo

echo -e "#### -XZ- ####"
xzcat test.xz
echo -e "##############"
xzcat test.xz | wc

echo -e "### -GZIP- ###"
zcat test.gz
echo -e "##############"
zcat test.gz | wc

echo -e "#### -LZO- ###"
cat test.lzo | lzop -d
echo -e "##############"
cat test.lzo | lzop -d | wc

make

echo "Create binpary file to be tested"
# dd if=/dev/urandom of=test.big.txt bs=1024 count=$((1024 * 16))
dd if=/dev/urandom bs=1 count=$((1024 * 1024 + 12345)) | xxd > test.big.txt
cat test.big.txt | xz -c > test.big.txt.xz
cat test.big.txt | xz -1 -c > test.big.txt.1.xz
cat test.big.txt | xz -2 -c > test.big.txt.2.xz
cat test.big.txt | xz -3 -c > test.big.txt.3.xz
cat test.big.txt | xz -4 -c > test.big.txt.4.xz
cat test.big.txt | xz -5 -c > test.big.txt.5.xz
cat test.big.txt | xz -6 -c > test.big.txt.6.xz
cat test.big.txt | xz -7 -c > test.big.txt.7.xz
cat test.big.txt | xz -9 -c > test.big.txt.9.xz
cat test.big.txt | gzip -c > test.big.txt.gz
cat test.big.txt | gzip -9 -c > test.big.txt.9.gz
cat test.big.txt | lzop -c > test.big.txt.lzo
cat test.big.txt | lzop -9 -c > test.big.txt.9.lzo
cat test.big.txt | xz --check=crc32 --arm --lzma2=,dict=32MiB -c > test.big.txt.arm.lzma.xz

./test.out

md5sum test.big* | sort

