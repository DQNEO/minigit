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

void _compress(FILE *fin, FILE *fout, long st_size, char *buf)
{
    z_stream z;

    char outbuf[OUTBUFSIZ];
    int count, flush, status;

    fread(buf, st_size, 1, fin);
    

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

    z.next_in = (Bytef *)buf;
    z.avail_in = st_size;             /* 入力バッファ中のデータのバイト数 */
    z.next_out = (Bytef *)outbuf;        /* 出力ポインタ */
    z.avail_out = OUTBUFSIZ;    /* 出力バッファのサイズ */
    
    deflate(&z, Z_FINISH);

    fwrite(outbuf, OUTBUFSIZ, 1, fout);

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

    char *buf;
    buf = malloc(st.st_size);
    if ((fin = fopen(in_file, "r")) == NULL) {
        fprintf(stderr, "Can't open %s\n", in_file);
        exit(1);
    }


    if ((fout = fopen(out_file, "w")) == NULL) {
      fprintf(stderr, "Can't open %s\n", out_file);
      exit(1);
    }

    _compress(fin, fout, st.st_size, buf);

    free(buf);
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
