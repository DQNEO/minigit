#include "minigit.h"

/**
 * get into string sha1 from binary sha1
 *
 * this function is totally borrowed from hex.c of git :)
 */
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

void calc_sha1(const char *type, const void *body, unsigned long len, unsigned char *sha1)
{
    int hdrlen;
    char hdr[256];
    SHA_CTX c;

    sprintf(hdr, "%s %ld", type, len);
    hdrlen = strlen(hdr) + 1;

    SHA1_Init(&c);
    SHA1_Update(&c, hdr, hdrlen);
    SHA1_Update(&c, body, len);
    SHA1_Final(sha1, &c);

}

void sha1_file_name(const unsigned char *sha1, char *filename)
{
    const char *objdir = ".git/objects";
    int len;
    char *str_sha1 = sha1_to_hex(sha1);
    int i;

    len = strlen(objdir);
    strcpy(filename, objdir);
    filename[len] = '/';
    filename[len+3] = '/';

    filename[len+1] = str_sha1[0];
    filename[len+2] = str_sha1[1];

    for (i=2;i<40;i++) {
	filename[len+2+i] = str_sha1[i];
    }
    filename[len + 42] = '\0';
}

void sha1_dir_name(const unsigned char *sha1, char *dirname)
{
    const char *objdir = ".git/objects";
    int len;
    char *str_sha1 = sha1_to_hex(sha1);

    len = strlen(objdir);
    strcpy(dirname, objdir);
    dirname[len] = '/';
    dirname[len+1] = str_sha1[0];
    dirname[len+2] = str_sha1[1];
    dirname[len+3] = '\0';
}
