#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

// opendir
#include <dirent.h>
#include <time.h>

// for calculate sha1
#include <openssl/sha.h>

/* zlib sample code  */
/* http://oku.edu.mie-u.ac.jp/~okumura/compression/comptest.c */
#include <zlib.h>
#include <error.h>

const char *weekday_names[] = {
  "Sunday",  "Mondays",  "Tuesdays",  "Wednesdays",  "Thursdays",  "Fridays",  "Saturdays"
};

const char *month_names[] = {
  "January",  "February",  "March",  "April",  "May",  "June",  "July",  "August",  "September",  "October",  "November",  "December"
};

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

