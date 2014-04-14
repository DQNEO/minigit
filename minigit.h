#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
// opendir
#include <dirent.h>
#include <time.h>

// for calculate sha1
#include <openssl/sha.h>

/* zlib sample code  */
/* http://oku.edu.mie-u.ac.jp/~okumura/compression/comptest.c */
#include <zlib.h>
#include <error.h>
#include <sys/mman.h>

typedef struct _TAG_OBJECT_INFO {
    char type[20];
    int  size;
    int  header_length; // ヘッダのバイト長
    char *buf;
} object_info;

typedef struct _TAG_COMMIT {
    char tree_sha1[41];
    char parent_sha1[41];
    char author_name[256];
    char frmted_time[256];
    char *message;
} commit;

#define INBUFSIZ   1024
#define OUTBUFSIZ  1024

#define PROGNAME "minigit"

//borrowed from git
//extern int errno;
typedef struct git_zstream {
	z_stream z;
	unsigned long avail_in;
	unsigned long avail_out;
	unsigned long total_in;
	unsigned long total_out;
	unsigned char *next_in;
	unsigned char *next_out;
} git_zstream;
#define git_SHA_CTX	SHA_CTX

struct cache_header {
	char hdr_signature[4];
	int hdr_version;
	int hdr_entries;
};

static inline int default_swab32(int val)
{
	return (((val & 0xff000000) >> 24) |
		((val & 0x00ff0000) >>  8) |
		((val & 0x0000ff00) <<  8) |
		((val & 0x000000ff) << 24));
}

static inline int bswap32(int x)
{
	int result;
	if (__builtin_constant_p(x))
		result = default_swab32(x);
	else
		__asm__("bswap %0" : "=r" (result) : "0" (x));
	return result;
}

void read_object_body(char in_file_name[], object_info *oi);
