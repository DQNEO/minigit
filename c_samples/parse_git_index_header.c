/**
 * sample code to parse .git/index file
 *
 * the ".git/index" file is a binary file.
 * the content is like below:
 * 
 * $ hexdump -C .git/index | head
 * 00000000  44 49 52 43 00 00 00 02  00 00 00 28 52 5b a4 34  |DIRC.......(R[.4|
 * 00000010  2c 67 bf 8a 52 5b a4 34  2c 67 bf 8a 00 00 fd 00  |,g..R[.4,g......|
 * 00000020  01 ac 17 55 00 00 81 a4  00 00 01 f5 00 00 01 f5  |...U............|
 * 00000030  00 00 00 71 95 ba 71 b8  e7 c2 d7 09 65 d9 a5 08  |...q..q.....e...|
 * 00000040  6a 9f ab d5 8e 87 d2 59  00 0a 2e 67 69 74 69 67  |j......Y...gitig|
 * 00000050  6e 6f 72 65 00 00 00 00  00 00 00 00 52 65 2a 2d  |nore........Re*-|
 *
 * this c program parses only the header of the file.
 * 
 */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
	fprintf(stderr, "Usage:prog .git/index\n");
	exit(1);
    }

    
    return 0;
}
