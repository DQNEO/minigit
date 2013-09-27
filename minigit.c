/* zlib を使う例 */
/* http://oku.edu.mie-u.ac.jp/~okumura/compression/comptest.c */
/* コンパイル: gcc -Wall comptest.c -o comptest -lz */
/* -lz は libz.a をリンクする（たぶん /usr(/local)/lib にある）*/

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>               /* /usr(/local)/include/zlib.h */
#include <string.h>

#define INBUFSIZ   1024         /* 入力バッファサイズ（任意） */
#define OUTBUFSIZ  1024         /* 出力バッファサイズ（任意） */

struct _TAG_OBJECT_INFO {
  char type[20];
  char size[20];
  int  header_length; // ヘッダのバイト長
} ;

void do_compress(char in_file_name[])          /* 圧縮 */
{
  z_stream z;                     /* ライブラリとやりとりするための構造体 */

  char inbuf[INBUFSIZ];           /* 入力バッファ */
  char outbuf[OUTBUFSIZ];         /* 出力バッファ */
  FILE *fin;                      /* 入力・出力ファイル */

    int count, flush, status;

    if ((fin = fopen(in_file_name, "r")) == NULL) {
        fprintf(stderr, "Can't open %s\n", in_file_name);
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

    z.avail_in = 0;             /* 入力バッファ中のデータのバイト数 */
    z.next_out = outbuf;        /* 出力ポインタ */
    z.avail_out = OUTBUFSIZ;    /* 出力バッファのサイズ */

    /* 通常は deflate() の第2引数は Z_NO_FLUSH にして呼び出す */
    flush = Z_NO_FLUSH;

    while (1) {
        if (z.avail_in == 0) {  /* 入力が尽きれば */
            z.next_in = inbuf;  /* 入力ポインタを入力バッファの先頭に */
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
            if (fwrite(outbuf, 1, OUTBUFSIZ, stdout) != OUTBUFSIZ) {
                fprintf(stderr, "Write error\n");
                exit(1);
            }
            z.next_out = outbuf; /* 出力バッファ残量を元に戻す */
            z.avail_out = OUTBUFSIZ; /* 出力ポインタを元に戻す */
        }
    }

    /* 残りを吐き出す */
    if ((count = OUTBUFSIZ - z.avail_out) != 0) {
        if (fwrite(outbuf, 1, count, stdout) != count) {
            fprintf(stderr, "Write error\n");
            exit(1);
        }
    }

    /* 後始末 */
    if (deflateEnd(&z) != Z_OK) {
        fprintf(stderr, "deflateEnd: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

    fclose(fin);
}

void cat_body(char in_file_name[], struct _TAG_OBJECT_INFO *oi)        /* 展開（復元） */
{
    int count, status;

  z_stream z;                     /* ライブラリとやりとりするための構造体 */

  char inbuf[INBUFSIZ];           /* 入力バッファ */
  char outbuf[OUTBUFSIZ];         /* 出力バッファ */
  FILE *fin;                      /* 入力・出力ファイル */
  int header_skipped = 0;


    if ((fin = fopen(in_file_name, "r")) == NULL) {
        fprintf(stderr, "Can't open %s\n", in_file_name);
        exit(1);
    }


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

    z.next_out = outbuf;        /* 出力ポインタ */
    z.avail_out = OUTBUFSIZ;    /* 出力バッファ残量 */
    status = Z_OK;

    while (status != Z_STREAM_END) {
        if (z.avail_in == 0) {  /* 入力残量がゼロになれば */
            z.next_in = inbuf;  /* 入力ポインタを元に戻す */
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
            if (fwrite(outbuf, 1, OUTBUFSIZ, stdout) != OUTBUFSIZ) {
                fprintf(stderr, "Write error\n");
                exit(1);
            }
            z.next_out = outbuf; /* 出力ポインタを元に戻す */
            z.avail_out = OUTBUFSIZ; /* 出力バッファ残量を元に戻す */
        }
    }

    /* 残りを吐き出す */
    if ((count = OUTBUFSIZ - z.avail_out) != 0) {
      if (! header_skipped) {
	if (fwrite(outbuf + oi->header_length, 1, count - oi->header_length, stdout) != count - oi->header_length) {
            fprintf(stderr, "Write error\n");
            exit(1);
        }

      } else if (fwrite(outbuf, 1, count, stdout) != count) {
            fprintf(stderr, "Write error\n");
            exit(1);
        }
    }

    /* 後始末 */
    if (inflateEnd(&z) != Z_OK) {
        fprintf(stderr, "inflateEnd: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

    fclose(fin);
}

void parse_header(char in_file_name[], struct _TAG_OBJECT_INFO *oi)
{
    int count, status;

  z_stream z;                     /* ライブラリとやりとりするための構造体 */

  char inbuf[INBUFSIZ];           /* 入力バッファ */
  char outbuf[OUTBUFSIZ];         /* 出力バッファ */
  FILE *fin;                      /* 入力・出力ファイル */

    if ((fin = fopen(in_file_name, "r")) == NULL) {
        fprintf(stderr, "Can't open %s\n", in_file_name);
        exit(1);
    }


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

    z.next_out = outbuf;        /* 出力ポインタ */
    z.avail_out = OUTBUFSIZ;    /* 出力バッファ残量 */
    status = Z_OK;

    while (status != Z_STREAM_END) {
        if (z.avail_in == 0) {  /* 入力残量がゼロになれば */
            z.next_in = inbuf;  /* 入力ポインタを元に戻す */
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
            if (fwrite(outbuf, 1, OUTBUFSIZ, stdout) != OUTBUFSIZ) {
                fprintf(stderr, "Write error\n");
                exit(1);
            }
            z.next_out = outbuf; /* 出力ポインタを元に戻す */
            z.avail_out = OUTBUFSIZ; /* 出力バッファ残量を元に戻す */
        }
    }

      char *cp;
      int i = 0;


    /* 残りを吐き出す */
    if ((count = OUTBUFSIZ - z.avail_out) != 0) {

      cp = outbuf;

      while (*(cp) != ' ') {
	oi->type[i++] = *(cp++);
	oi->header_length++;
      }
      oi->type[i] = 0;
      oi->header_length++;


      i = 0;
      while (*cp) {
	oi->size[i++] = *(cp++);
	oi->header_length++;
      }
      oi->size[i] = 0;
      
    }

    /* 後始末 */
    if (inflateEnd(&z) != Z_OK) {
        fprintf(stderr, "inflateEnd: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

    fclose(fin);
}

int main(int argc, char *argv[])
{
    int c;
    char *in_file_name;

    if (argc != 3) {
        fprintf(stderr, "Usage: comptest -[flag] infile\n");
        fprintf(stderr, "  flag: -c=compress -d=decompress\n");
        exit(0);
    }

    struct _TAG_OBJECT_INFO oi;
    oi.header_length = 0;
    in_file_name = argv[2];

    if (strcmp(argv[1], "-c") == 0) {
        do_compress(in_file_name);
    } else if (strcmp(argv[1], "cat-file-x") == 0) {
      parse_header(in_file_name, &oi);
      printf("type:%s\n", oi.type);
      printf("size:%s\n", oi.size);
      printf("header_length:%d\n", oi.header_length);
      cat_body(in_file_name, &oi);
    } else if (strcmp(argv[1], "cat-file-s") == 0) {
      parse_header(in_file_name, &oi);
      printf("%s\n", oi.size);
    } else {
        fprintf(stderr, "Unknown flag: %s\n", argv[1]);
        exit(1);
    }


    return 0;
}
