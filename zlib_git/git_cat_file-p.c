/**
 * zlib 圧縮・解凍するツール
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

void _decompress(FILE *fin, FILE *fout)
{
    z_stream z;
    char inbuf[INBUFSIZ];
    char outbuf[OUTBUFSIZ];
    int count, status;

    /* すべてのメモリ管理をライブラリに任せる */
    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;

    /* 初期化 */
    z.next_in = Z_NULL;
    z.avail_in = 0;
    if (inflateInit(&z) != Z_OK) {
        fprintf(stderr, "inflateInit: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

    z.next_out = (Bytef *)outbuf;        /* 出力ポインタ */
    z.avail_out = OUTBUFSIZ;    /* 出力バッファ残量 */
    status = Z_OK;

    while (status != Z_STREAM_END) {
        if (z.avail_in == 0) {  /* 入力残量がゼロになれば */
            z.next_in = (Bytef *)inbuf;  /* 入力ポインタを元に戻す */
            z.avail_in = fread(inbuf, 1, INBUFSIZ, fin); /* データを読む */
        }
        status = inflate(&z, Z_NO_FLUSH); /* 展開 */
        if (status == Z_STREAM_END) break; /* 完了 */
        if (status != Z_OK) {   /* エラー */
            fprintf(stderr, "inflate: %s\n", (z.msg) ? z.msg : "???");
            exit(1);
        }
        if (z.avail_out == 0) { /* 出力バッファが尽きれば */
            /* まとめて書き出す */
            if (fwrite(outbuf, 1, OUTBUFSIZ, fout) != OUTBUFSIZ) {
                fprintf(stderr, "Write error\n");
                exit(1);
            }
            z.next_out = (Bytef *)outbuf; /* 出力ポインタを元に戻す */
            z.avail_out = OUTBUFSIZ; /* 出力バッファ残量を元に戻す */
        }
    }

    /* 残りを吐き出す */
    if ((count = OUTBUFSIZ - z.avail_out) != 0) {
        if (fwrite(outbuf, 1, count, fout) != count) {
            fprintf(stderr, "Write error\n");
            exit(1);
        }
    }

    /* 後始末 */
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
    FILE *fin, *fout;

    if (argc == 1) {
        usage();
        exit(0);
    }

    if ((fin = fopen(argv[2], "r")) == NULL) {
        fprintf(stderr, "Can't open %s\n", argv[2]);
        exit(1);
    }
    if ((fout = fopen(argv[3], "w")) == NULL) {
        fprintf(stderr, "Can't open %s\n", argv[3]);
        exit(1);
    }
    _decompress(fin, fout);
    return 0;
}
