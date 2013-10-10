/* zlib を使う例 */
/* http://oku.edu.mie-u.ac.jp/~okumura/compression/comptest.c */
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define INBUFSIZ   1024
#define OUTBUFSIZ  409600

void write_loose_object(FILE *fin, char *out_file, long body_size, char *buf, char *hdr, int hdrlen)
{
    z_stream z;
    FILE *fout;
    char *outbuf;
    int ret;

    outbuf = malloc(body_size);
    fread(buf, body_size, 1, fin);

    if ((fout = fopen(out_file, "w")) == NULL) {
      fprintf(stderr, "Can't open %s\n", out_file);
      exit(1);
    }


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

    /* First header..*/
    z.next_in = (unsigned char *)hdr;
    z.avail_in = hdrlen;

    z.next_out = (Bytef *)outbuf;        /* 出力ポインタ */
    z.avail_out = hdrlen + OUTBUFSIZ;    /* 出力バッファのサイズ */

    while (deflate(&z, 0) == Z_OK) ;
    
    z.next_in = (Bytef *)buf;
    z.avail_in = body_size;             /* 入力バッファ中のデータのバイト数 */

    do {
      ret = deflate(&z, Z_FINISH);

      /* debug
      if (ret == Z_STREAM_END) {
	printf("Z_STREAM_END\n");
      } else if (ret == Z_OK) {
	printf("Z_OK\n");
      }
      */
      fwrite(outbuf, OUTBUFSIZ, 1, fout);

    } while (ret == Z_OK);


    /* 後始末 */
    if (deflateEnd(&z) != Z_OK ) {
        fprintf(stderr, "deflateEnd error:%s\n", z.msg );
        exit(1);
    }

    free(outbuf);
    fclose(fout);
}


void usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  zlib orig_file compressed_file\n");
    exit(0);
}

void do_compress(char *in_file, char *out_file)
{
    FILE *fin;


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


    char hdr[1024];
    char *type = "blob";
    sprintf(hdr, "%s %ld", type ,(long) st.st_size);

    int hdrlen = strlen(hdr) + 1;

    write_loose_object(fin, out_file, st.st_size, buf, hdr, hdrlen);

    free(buf);
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
