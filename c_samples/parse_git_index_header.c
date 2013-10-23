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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <math.h>

typedef unsigned   uint32_t;

static inline uint32_t default_swab32(uint32_t val)
{
	return (((val & 0xff000000) >> 24) |
		((val & 0x00ff0000) >>  8) |
		((val & 0x0000ff00) <<  8) |
		((val & 0x000000ff) << 24));
}

static inline uint32_t bswap32(uint32_t x)
{
	uint32_t result;
	if (__builtin_constant_p(x))
		result = default_swab32(x);
	else
		__asm__("bswap %0" : "=r" (result) : "0" (x));
	return result;
}

struct cache_time {
	uint32_t nsec;
	uint32_t sec;
};

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
    uint32_t version;
    uint32_t entries;
};

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

void print_entry(struct cache_entry *ce)
{
    static int i = 1;
    /*
    printf("ctime.sec = %d\n", bswap32(ce->ce_ctime_sec));
    printf("ctime.nsec = %d\n", bswap32(ce->ce_ctime_nsec));
    printf("mtime.sec = %d\n", bswap32(ce->ce_mtime_sec));
    printf("mtime.nsec = %d\n", bswap32(ce->ce_mtime_nsec));
    printf("dev = %d\n", bswap32(ce->ce_dev));
    printf("ino = %d\n", bswap32(ce->ce_ino));
    printf("mode = %o\n", bswap32(ce->ce_mode));
    printf("uid = %d\n", bswap32(ce->ce_uid));
    printf("gid = %d\n", bswap32(ce->ce_gid));
    printf("size = %u\n", bswap32(ce->ce_size));
    printf("sha1 = %s\n", sha1_to_hex(ce->sha1));
    printf("namelen = %x\n", ce->namelen);
    */
    printf("entry[%d] = %s, len = %d\n", i, ce->name, ce->namelen);
    i++;
}

int calc_padding(int n)
{
    double fld; 
    int ret, target;

    fld = floor((n -2) / 8);
    target = (int)(fld + 1) * 8 + 2;
    ret = target - n;
    //printf("calc padding %d => %d\n", n, ret);
    return ret;
}

int main(int argc, char **argv)
{
    char *filename;
    struct stat st;
    int fd;
    void *map;
    struct index_header *hdr;
    struct cache_entry *ce;
    char *cp;

    if (argc != 2) {
	fprintf(stderr, "Usage:prog .git/index\n");
	exit(1);
    }

    filename = argv[1];
    if (stat(filename, &st) == -1) {
	fprintf(stderr, "unable to stat '%s'\n", filename);
	exit(1);
    }

    if ((fd = open(filename, O_RDONLY)) == -1) {
	fprintf(stderr, "unable to open file '%s'\n", filename);
	exit(1);
    }

    map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    hdr = map;
    ce = (struct cache_entry *)(hdr + 1);
    printf("==== header ===\n");
    printf("signature = %s\n", hdr->dirc); // => "DIRC"  44 49 52 43
    printf("version = %d\n", bswap32(hdr->version)); // => 2
    printf("entries = %d\n", bswap32(hdr->entries)); // => 41 or your number of entries

    print_entry(ce);
    cp = ce->name + ce->namelen;
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;

    print_entry(ce);
    cp = ce->name + ce->namelen;
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;

    print_entry(ce);
    cp = ce->name + ce->namelen;
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;

    print_entry(ce);
    cp = ce->name + ce->namelen;
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;

    print_entry(ce);
    cp = ce->name + ce->namelen;
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;

    print_entry(ce);
    cp = ce->name + ce->namelen; //22
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;

    print_entry(ce);
    cp = ce->name + ce->namelen; //20
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;

    print_entry(ce);
    cp = ce->name + ce->namelen; //16
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;

    print_entry(ce);
    cp = ce->name + ce->namelen; //23
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;

    print_entry(ce);
    cp = ce->name + ce->namelen; //9
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;

    print_entry(ce);
    cp = ce->name + ce->namelen; //9
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;

    print_entry(ce);
    cp = ce->name + ce->namelen; //13
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;

    print_entry(ce);
    cp = ce->name + ce->namelen; //18
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;

    print_entry(ce);
    cp = ce->name + ce->namelen; //11
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;

    print_entry(ce);
    cp = ce->name + ce->namelen; //16
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;

    print_entry(ce);
    cp = ce->name + ce->namelen; //11
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;

    print_entry(ce);
    cp = ce->name + ce->namelen; //16
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;

    print_entry(ce);
    cp = ce->name + ce->namelen; //?
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;

    print_entry(ce);
    cp = ce->name + ce->namelen; //?
    cp += calc_padding(ce->namelen);
    ce = (struct cache_entry *)cp;


    close(fd);
    return 0;
}
