# Makefile of write_loose_object
write_loose_object : write_loose_object.c
	gcc -g -Wall -O0 -std=c99 -lz -o write_loose_object write_loose_object.c

test : write_loose_object
	./write_loose_object hello.txt hello.zlib && diff hello.zlib hello.zlib.orig

clean :
	rm write_loose_object
