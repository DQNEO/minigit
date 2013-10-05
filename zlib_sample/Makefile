# Makefile of zlib

zlib : zlib.c
	gcc -g -Wall -O0 -std=c99 -lz -o zlib zlib.c

test : zlib
	prove

clean :
	rm zlib

