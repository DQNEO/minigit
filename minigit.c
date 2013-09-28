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
#define PROGNAME "minigit"

typedef struct _TAG_OBJECT_INFO {
    char type[20];
    int  size;
    int  header_length; // ヘッダのバイト長
    char *buf;
} object_info;

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

void parse_header(char *header, object_info  *oi)
{
    int i = 0;
    char size[20];

    while (*(header) != ' ') {
	oi->type[i++] = *(header++);
	oi->header_length++;
    }
    oi->type[i] = 0;
    oi->header_length++;

    header++; // skip ' '
    oi->header_length++;

    i = 0;
    while (*header) {
	size[i++] = *(header++);
	oi->header_length++;
    }
    size[i] = 0;
    oi->size = atoi(size);
}

/**
 * オブジェクトを丸ごと読み込む
 */
void read_object_body(char in_file_name[], object_info *oi)
{

    int count, status;

    z_stream z;                     /* ライブラリとやりとりするための構造体 */

    char inbuf[INBUFSIZ];           /* 入力バッファ */
    char *outbuf = oi->buf;
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

    if (z.avail_in == 0) {  /* 入力残量がゼロになれば */
	z.next_in = inbuf;  /* 入力ポインタを元に戻す */
	z.avail_in = fread(inbuf, 1, INBUFSIZ, fin); /* データを読む */
    }

    /* 展開 */
    status = inflate(&z, Z_NO_FLUSH);
    if (status != Z_STREAM_END) {
	fprintf(stderr, "inflate: %s\n", (z.msg) ? z.msg : "???");
	exit(1);
    }

    /* 後始末 */
    if (inflateEnd(&z) != Z_OK) {
        fprintf(stderr, "inflateEnd: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

    fclose(fin);

    //printf("bufsize = %d\n", sizeof(outbuf));
    //printf("avail_out = %d\n", z.avail_out);

    outbuf[OUTBUFSIZ - z.avail_out] = 0;
}

void parse_object_header(char in_file_name[], object_info *oi)
{
    int count, status;

    z_stream z;                     /* ライブラリとやりとりするための構造体 */

    char inbuf[INBUFSIZ];           /* 入力バッファ */
    char header[OUTBUFSIZ];

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

    z.next_out = header;
    z.avail_out = sizeof(header);
    status = Z_OK;

    if (z.avail_in == 0) {  /* 入力残量がゼロになれば */
	z.next_in = inbuf;  /* 入力ポインタを元に戻す */
	z.avail_in = fread(inbuf, 1, INBUFSIZ, fin); /* データを読む */
    }

    /* 展開 */
    status = inflate(&z, Z_NO_FLUSH);
    if (status != Z_STREAM_END) {
	fprintf(stderr, "inflate: %s\n", (z.msg) ? z.msg : "???");
	exit(1);
    }

    /* 後始末 */
    if (inflateEnd(&z) != Z_OK) {
        fprintf(stderr, "inflateEnd: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

    fclose(fin);

    parse_header(header, oi);
}

void cat_tree_object(object_info *oi)
{

    //ヘッダー部は読み飛ばす
    char *start = oi->buf + oi->header_length;
    //ボディのサイズはヘッダに書かれてあるのを参照する
    char *end = oi->buf + oi->header_length + oi->size;
    char *cp = start;
    //

    while (cp < end) {

	char mode[6];
	char *filename;
	unsigned char sha1[20];

	int j = 0;
	int i = 0;

	int is_tree;

	// filemode
	// 6桁または5桁。' 'で終端
	while (*cp != ' ') {
	    mode[j++] = *(cp++);
	}
	mode[j] = 0;

	is_tree = (strlen(mode) == 5) ? 1 : 0;

	cp++; // skip ' '

	// filename
	// nullで終端
	filename = cp;
	while(*cp++) {
	}

	//cp++; // skip '\0'

	//sha1
	//固定長で20文字
	for (i=0;i<20;i++) {
	    sha1[i] = (unsigned char)*(cp++) ;
	}

	// pretty print
	// filemode
	printf("%06d ", atoi(mode));

	if (is_tree) {
	    printf("tree ");
	} else {
	    printf("blob ");
	}

	for (i=0;i<20;i++) {
	    printf("%02x",sha1[i]);
	}	

	printf("\t");
	printf("%s", filename);
	
	printf("\n");
	continue;

    }
  

}

int cmd_cat_file(char *argv[])
{
    struct _TAG_OBJECT_INFO oi;
    char *filename;

    char buf[OUTBUFSIZ];

    oi.header_length = 0;
    filename = argv[3];
    
    oi.buf = buf;

    if (strcmp(argv[2], "-s") == 0) {
	// show size
	parse_object_header(filename, &oi);
	printf("%d\n", oi.size);
    } else if (strcmp(argv[2], "-t") == 0) {
	// show type
	parse_object_header(filename, &oi);
	printf("%s\n", oi.type);
    } else if (strcmp(argv[2], "-p") == 0) {
	// pretty print
	parse_object_header(filename, &oi);
	if (strcmp(oi.type, "tree") == 0) {
	    read_object_body(filename, &oi);
	    cat_tree_object(&oi);
	} else {
	    // print blob or commit
	    read_object_body(filename, &oi);
	    fwrite(oi.buf + oi.header_length , 1, oi.size, stdout);
	}

    } else {
        fprintf(stderr, "Unknown flag: %s\n", argv[2]);
        exit(1);
    }

    return 0;
}


void usage() {
    fprintf(stderr, "Usage: %s option args\n" , PROGNAME);
    fprintf(stderr, "  cat-file: -[s|t|p] object\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc == 1) {
	usage();
    }

    /*
    if (strcmp(argv[1], "-c") == 0) {
        do_compress(filename);
    }
    */
    if (strcmp(argv[1], "cat-file") == 0) {
	return cmd_cat_file(argv);
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
	usage();
    }

    return 0;
}
