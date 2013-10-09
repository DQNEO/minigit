/* zlib を使う例 */
/* http://oku.edu.mie-u.ac.jp/~okumura/compression/comptest.c */
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define INBUFSIZ   1024
#define OUTBUFSIZ  1024

void _compress(FILE *fin, FILE *fout)
{
    z_stream z;
    char inbuf[INBUFSIZ];
    char outbuf[OUTBUFSIZ];
    int count, flush, status;

    /* すべてのメモリ管理をライブラリに任せる */
    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;

    /* 初期化 */
    /* 第2引数は圧縮の度合。0〜9 の範囲の整数で，0 は無圧縮 */
    /* Z_DEFAULT_COMPRESSION (= 6) が標準 */
    if (deflateInit(&z, Z_DEFAULT_COMPRESSION) != Z_OK) {
        fprintf(stderr, "deflateInit: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

    z.avail_in = 0;             /* 入力バッファ中のデータのバイト数 */
    z.next_out = (Bytef *)outbuf;        /* 出力ポインタ */
    z.avail_out = OUTBUFSIZ;    /* 出力バッファのサイズ */

    /* 通常は deflate() の第2引数は Z_NO_FLUSH にして呼び出す */
    flush = Z_NO_FLUSH;

    while (1) {
        if (z.avail_in == 0) {  /* 入力が尽きれば */
            z.next_in = (Bytef *)inbuf;  /* 入力ポインタを入力バッファの先頭に */
            z.avail_in = fread(inbuf, 1, INBUFSIZ, fin); /* データを読み込む */

            /* 入力が最後になったら deflate() の第2引数は Z_FINISH にする */
            if (z.avail_in < INBUFSIZ) flush = Z_FINISH;
        }
        status = deflate(&z, flush); /* 圧縮する */
        if (status == Z_STREAM_END) break; /* 完了 */
        if (status != Z_OK) {   /* エラー */
            fprintf(stderr, "deflate: %s\n", (z.msg) ? z.msg : "???");
            exit(1);
        }
        if (z.avail_out == 0) { /* 出力バッファが尽きれば */
            /* まとめて書き出す */
            if (fwrite(outbuf, 1, OUTBUFSIZ, fout) != OUTBUFSIZ) {
                fprintf(stderr, "Write error\n");
                exit(1);
            }
            z.next_out = (Bytef *)outbuf; /* 出力バッファ残量を元に戻す */
            z.avail_out = OUTBUFSIZ; /* 出力ポインタを元に戻す */
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
    if (deflateEnd(&z) != Z_OK) {
        fprintf(stderr, "deflateEnd: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }
}


void usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  zlib orig_file compressed_file\n");
    exit(0);
}

void do_compress(char *in_file, char *out_file)
{
   FILE *fin, *fout;

    struct stat st;
    if (stat(in_file, &st)) {
	fprintf(stderr, "unable to stat %s\n", in_file);
    }

    unsigned char *buf;
    buf = malloc(st.st_size);
    if ((fin = fopen(in_file, "r")) == NULL) {
        fprintf(stderr, "Can't open %s\n", in_file);
        exit(1);
    }

    //fread(buf, st.st_size, 1, fin);
    

    if ((fout = fopen(out_file, "w")) == NULL) {
      fprintf(stderr, "Can't open %s\n", out_file);
      exit(1);
    }

    _compress(fin, fout);

    fclose(fout);
    fclose(fin);

}

int main(int argc, char *argv[])
{
 

    if (argc == 1) {
	usage();
    }

    do_compress(argv[1], argv[2]);
    return 0;
}
