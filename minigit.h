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

void parse_object_header(char in_file_name[], object_info *oi);
void read_object_body(char in_file_name[], object_info *oi);

char *sha1_to_hex(const unsigned char *sha1);
void calc_sha1(const char *type, const void *body, unsigned long len, unsigned char *sha1);

struct tm *time_to_tm(unsigned long time, int tz);

extern const char *weekday_names[];
extern const char *month_names[];

void git_write_loose_object(const unsigned char *sha1, char *hdr, int hdrlen,
			      const void *buf, unsigned long len, time_t mtime);

void sha1_file_name(const unsigned char *sha1, char *filename);

int cmd_cat_file(int argc, char **argv);
int cmd_log(int argc, char **argv);
int cmd_rev_parse(int argc, char **argv);
int cmd_init(int argc, char **argv);
int cmd_commit(int argc, char **argv);
int cmd_hash_object(int argc, char **argv);
int cmd_ls_files(int argc, char **argv);

int _rev_parse(const char *_rev, char *sha1_string);
int cat_commit_object(const char *sha1_string, char *parent_sha1);
int find_file(const char *sha1_input, char *matched_filename);
void pretty_print_tree_object(object_info *oi);

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
