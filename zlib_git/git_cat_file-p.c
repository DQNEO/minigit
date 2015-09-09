/**
 * git cat-file -p 相当のことをするコマンド
 *
 * original from http://oku.edu.mie-u.ac.jp/~okumura/compression/comptest.c
 * licensed under http://creativecommons.org/licenses/by/4.0/
 */
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <string.h>

#define INBUFSIZ   1024
#define OUTBUFSIZ  1024

int _write_skipping_header(char *outbuf, size_t size, size_t n ,FILE *fout)
{
    static int is_header = 1;
    int tmp_n = n;
    if (is_header) {
        while (*outbuf) {outbuf++; tmp_n--;}
        fwrite(outbuf, size, tmp_n, fout);
        is_header = 0;
        return n;
    }
    return fwrite(outbuf, size, n, fout);
}

void _decompress(FILE *fin, FILE *fout)
{
    z_stream z;
    char inbuf[INBUFSIZ];
    char outbuf[OUTBUFSIZ];
    int count, status;

    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;

    z.next_in = Z_NULL;
    z.avail_in = 0;
    if (inflateInit(&z) != Z_OK) {
        fprintf(stderr, "inflateInit: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

    z.next_out = (Bytef *)outbuf;
    z.avail_out = OUTBUFSIZ;
    status = Z_OK;

    while (status != Z_STREAM_END) {
        if (z.avail_in == 0) {
            z.next_in = (Bytef *)inbuf;
            z.avail_in = fread(inbuf, 1, INBUFSIZ, fin);
        }
        status = inflate(&z, Z_NO_FLUSH);
        if (status == Z_STREAM_END) break;
        if (status != Z_OK) {
            fprintf(stderr, "inflate: %s\n", (z.msg) ? z.msg : "???");
            exit(1);
        }
        if (z.avail_out == 0) {
            if (_write_skipping_header(outbuf, 1, OUTBUFSIZ, fout) != OUTBUFSIZ) {
                fprintf(stderr, "Write error\n");
                exit(1);
            }
            z.next_out = (Bytef *)outbuf;
            z.avail_out = OUTBUFSIZ;
        }
    }

    if ((count = OUTBUFSIZ - z.avail_out) != 0) {
        if (_write_skipping_header(outbuf, 1, count, fout) != count) {
            fprintf(stderr, "Write error\n");
            exit(1);
        }
    }

    if (inflateEnd(&z) != Z_OK) {
        fprintf(stderr, "inflateEnd: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }
}

void usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  git_cat_file-p blob_file\n");
}

int main(int argc, char *argv[])
{
    FILE *fin;

    if (argc == 1) {
        usage();
        exit(0);
    }

    if ((fin = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "Can't open %s\n", argv[1]);
        exit(1);
    }

    _decompress(fin, stdout);
    return 0;
}
