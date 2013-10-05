# Makefile of zlib
# -lz は libz.a をリンクする（たぶん /usr(/local)/lib にある）*/
zlib : zlib.c
	gcc -g -Wall -O0 -std=c99 -lz -o zlib zlib.c

test : zlib
	prove

clean :
	rm zlib

