/* zlib ��Ȥ��� */
/* http://oku.edu.mie-u.ac.jp/~okumura/compression/comptest.c */
/* ����ѥ���: gcc -Wall comptest.c -o comptest -lz */
/* -lz �� libz.a ���󥯤���ʤ��֤� /usr(/local)/lib �ˤ����*/

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>               /* /usr(/local)/include/zlib.h */
#include <string.h>

#define INBUFSIZ   1024         /* ���ϥХåե���������Ǥ�ա� */
#define OUTBUFSIZ  1024         /* ���ϥХåե���������Ǥ�ա� */

z_stream z;                     /* �饤�֥��Ȥ��Ȥꤹ�뤿��ι�¤�� */

char inbuf[INBUFSIZ];           /* ���ϥХåե� */
char outbuf[OUTBUFSIZ];         /* ���ϥХåե� */
FILE *fin, *fout;               /* ���ϡ����ϥե����� */

void do_compress(void)          /* ���� */
{
    int count, flush, status;

    /* ���٤ƤΥ��������饤�֥���Ǥ���� */
    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;

    /* ����� */
    /* ��2�����ϰ��̤��ٹ硣0��9 ���ϰϤ������ǡ�0 ��̵���� */
    /* Z_DEFAULT_COMPRESSION (= 6) ��ɸ�� */
    if (deflateInit(&z, Z_DEFAULT_COMPRESSION) != Z_OK) {
        fprintf(stderr, "deflateInit: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

    z.avail_in = 0;             /* ���ϥХåե���Υǡ����ΥХ��ȿ� */
    z.next_out = outbuf;        /* ���ϥݥ��� */
    z.avail_out = OUTBUFSIZ;    /* ���ϥХåե��Υ����� */

    /* �̾�� deflate() ����2������ Z_NO_FLUSH �ˤ��ƸƤӽФ� */
    flush = Z_NO_FLUSH;

    while (1) {
        if (z.avail_in == 0) {  /* ���Ϥ��Ԥ���� */
            z.next_in = inbuf;  /* ���ϥݥ��󥿤����ϥХåե�����Ƭ�� */
            z.avail_in = fread(inbuf, 1, INBUFSIZ, fin); /* �ǡ������ɤ߹��� */

            /* ���Ϥ��Ǹ�ˤʤä��� deflate() ����2������ Z_FINISH �ˤ��� */
            if (z.avail_in < INBUFSIZ) flush = Z_FINISH;
        }
        status = deflate(&z, flush); /* ���̤��� */
        if (status == Z_STREAM_END) break; /* ��λ */
        if (status != Z_OK) {   /* ���顼 */
            fprintf(stderr, "deflate: %s\n", (z.msg) ? z.msg : "???");
            exit(1);
        }
        if (z.avail_out == 0) { /* ���ϥХåե����Ԥ���� */
            /* �ޤȤ�ƽ񤭽Ф� */
            if (fwrite(outbuf, 1, OUTBUFSIZ, fout) != OUTBUFSIZ) {
                fprintf(stderr, "Write error\n");
                exit(1);
            }
            z.next_out = outbuf; /* ���ϥХåե����̤򸵤��᤹ */
            z.avail_out = OUTBUFSIZ; /* ���ϥݥ��󥿤򸵤��᤹ */
        }
    }

    /* �Ĥ���Ǥ��Ф� */
    if ((count = OUTBUFSIZ - z.avail_out) != 0) {
        if (fwrite(outbuf, 1, count, fout) != count) {
            fprintf(stderr, "Write error\n");
            exit(1);
        }
    }

    /* ����� */
    if (deflateEnd(&z) != Z_OK) {
        fprintf(stderr, "deflateEnd: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }
}

void do_decompress(void)        /* Ÿ���������� */
{
    int count, status;

    /* ���٤ƤΥ��������饤�֥���Ǥ���� */
    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;

    /* ����� */
    z.next_in = Z_NULL;
    z.avail_in = 0;
    if (inflateInit(&z) != Z_OK) {
        fprintf(stderr, "inflateInit: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

    z.next_out = outbuf;        /* ���ϥݥ��� */
    z.avail_out = OUTBUFSIZ;    /* ���ϥХåե����� */
    status = Z_OK;

    while (status != Z_STREAM_END) {
        if (z.avail_in == 0) {  /* ���ϻ��̤�����ˤʤ�� */
            z.next_in = inbuf;  /* ���ϥݥ��󥿤򸵤��᤹ */
            z.avail_in = fread(inbuf, 1, INBUFSIZ, fin); /* �ǡ������ɤ� */
        }
        status = inflate(&z, Z_NO_FLUSH); /* Ÿ�� */
        if (status == Z_STREAM_END) break; /* ��λ */
        if (status != Z_OK) {   /* ���顼 */
            fprintf(stderr, "inflate: %s\n", (z.msg) ? z.msg : "???");
            exit(1);
        }
        if (z.avail_out == 0) { /* ���ϥХåե����Ԥ���� */
            /* �ޤȤ�ƽ񤭽Ф� */
            if (fwrite(outbuf, 1, OUTBUFSIZ, fout) != OUTBUFSIZ) {
                fprintf(stderr, "Write error\n");
                exit(1);
            }
            z.next_out = outbuf; /* ���ϥݥ��󥿤򸵤��᤹ */
            z.avail_out = OUTBUFSIZ; /* ���ϥХåե����̤򸵤��᤹ */
        }
    }

    /* �Ĥ���Ǥ��Ф� */
    if ((count = OUTBUFSIZ - z.avail_out) != 0) {
        if (fwrite(outbuf, 1, count, fout) != count) {
            fprintf(stderr, "Write error\n");
            exit(1);
        }
    }

    /* ����� */
    if (inflateEnd(&z) != Z_OK) {
        fprintf(stderr, "inflateEnd: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    int c;

    if (argc != 4) {
        fprintf(stderr, "Usage: comptest -[flag] infile outfile\n");
        fprintf(stderr, "  flag: -c=compress -dd=decompress\n");
        exit(0);
    }

    if (strcmp(argv[1], "-c") == 0) {
        c = 1;
    } else if (strcmp(argv[1], "-d") == 0) {
        c = 0;
    } else {
        fprintf(stderr, "Unknown flag: %s\n", argv[1]);
        exit(1);
    }
    if ((fin = fopen(argv[2], "r")) == NULL) {
        fprintf(stderr, "Can't open %s\n", argv[2]);
        exit(1);
    }

    if ((fout = fopen(argv[3], "w")) == NULL) {
        fprintf(stderr, "Can't open %s\n", argv[3]);
        exit(1);
    }
    if (c) do_compress(); else do_decompress();
    fclose(fin);
    fclose(fout);
    return 0;
}
