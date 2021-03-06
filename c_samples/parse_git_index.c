/**
 * Parser of .git/index file
 *
 * This C program parses any .git/index file.
 * It workds just the same as "git ls-files --stage" command.
 * 
 * Usage:
 *   ./parse_git_index  path/to/.git/index
 * 
 * ".git/index" is a binary file.
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
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

struct cache_entry {
    unsigned int ce_ctime_sec;
    unsigned int ce_ctime_nsec;
    unsigned int ce_mtime_sec;
    unsigned int ce_mtime_nsec;
    unsigned int ce_dev;
    unsigned int ce_ino;
    unsigned int ce_mode;
    unsigned int ce_uid;
    unsigned int ce_gid;
    unsigned int ce_size;
    unsigned char sha1[21];
    char namelen;
    char name[1];
};

struct index_header {
    char dirc[4];
    unsigned int version;
    unsigned int entries;
};

static inline unsigned int default_swab32(unsigned int val)
{
	return (((val & 0xff000000) >> 24) |
		((val & 0x00ff0000) >>  8) |
		((val & 0x0000ff00) <<  8) |
		((val & 0x000000ff) << 24));
}

static inline unsigned int bswap32(unsigned int x)
{
	unsigned int result;
	if (__builtin_constant_p(x))
		result = default_swab32(x);
	else
		__asm__("bswap %0" : "=r" (result) : "0" (x));
	return result;
}

char *sha1_to_hex(const unsigned char *sha1)
{
    static int bufno;
    static char hexbuffer[4][50];
    static const char hex[] = "0123456789abcdef";
    char *buffer = hexbuffer[3 & ++bufno], *buf = buffer;
    int i;

    for (i = 0; i < 20; i++) {
	unsigned int val = *sha1++;
	*buf++ = hex[val >> 4];
	*buf++ = hex[val & 0xf];
    }
    *buf = '\0';

    return buffer;
}

int calc_padding(int n)
{
    int floor;
    int ret, target;

    floor = (int)((n -2) / 8);
    target = (floor + 1) * 8 + 2;
    ret = target - n;

    return ret;
}

int main(int argc, char **argv)
{
    char *index_file;
    struct stat st;
    int fd;
    void *map;
    struct index_header *hdr;
    struct cache_entry *ce;
    char *p_next_entry;
    int count_entries;
    int i;

    if (argc != 2) {
	fprintf(stderr, "Usage:prog .git/index\n");
	exit(1);
    }

    index_file = argv[1];
    if (stat(index_file, &st) == -1) {
	fprintf(stderr, "unable to stat '%s'\n", index_file);
	exit(1);
    }

    if ((fd = open(index_file, O_RDONLY)) == -1) {
	fprintf(stderr, "unable to open file '%s'\n", index_file);
	exit(1);
    }

    // git original code is here:
    // https://github.com/git/git/blob/v1.9.1/read-cache.c#L1455
    map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    hdr = map;
    ce = (struct cache_entry *)(hdr + 1);

    /* for debug  */
    /* 
    printf("==== header ===\n");
    printf("signature = %s\n", hdr->dirc); // => "DIRC"  44 49 52 43
    printf("version = %d\n", bswap32(hdr->version)); // => 2
    printf("entries = %d\n", bswap32(hdr->entries)); // => number of entries
    */
    count_entries = bswap32(hdr->entries);

    for (i=0; i < count_entries; i++) {
	printf("%o %s 0\t%s\n",
	       bswap32(ce->ce_mode),
	       sha1_to_hex(ce->sha1),
	       ce->name
	    );

	p_next_entry = ce->name + ce->namelen + calc_padding(ce->namelen);
	ce = (struct cache_entry *)p_next_entry;
    }

    close(fd);
    return 0;
}
