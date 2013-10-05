# Makefile of zlib

zlib : comptest.c
	gcc -g -Wall -O0 -std=c99 -lz -o comptest comptest.c

test : zlib
	prove

clean :
	rm zlib

