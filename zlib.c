#include "minigit.h"
/**
 * parsing object
 */
void read_object_body(char in_file_name[], object_info *oi)
{
    FILE *fin;

    if ((fin = fopen(in_file_name, "r")) == NULL) {
        fprintf(stderr, "Can't open %s\n", in_file_name);
        exit(1);
    }

    z_stream z;
    char inbuf[INBUFSIZ];
    int status;

    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;

    size_t outbfsiz = oi->header_length + oi->size;
    oi->buf = malloc(outbfsiz);

    z.next_in = Z_NULL;
    z.avail_in = 0;
    if (inflateInit(&z) != Z_OK) {
        fprintf(stderr, "inflateInit: %s\n", (z.msg) ? z.msg : "???");
        exit(1);
    }

    z.next_out = (Bytef *)oi->buf;        /* 出力ポインタ */
    z.avail_out = outbfsiz;    /* 出力バッファ残量 */
    status = Z_OK;

    while (status != Z_STREAM_END) {
        if (z.avail_in == 0) {  /* 入力残量がゼロになれば */
            z.next_in = (Bytef *)inbuf;  /* 入力ポインタを元に戻す */
            z.avail_in = fread(inbuf, 1, INBUFSIZ, fin); /* データを読む */
        }
        status = inflate(&z, Z_NO_FLUSH);
        if (status == Z_STREAM_END) break;
        if (status != Z_OK) {
            fprintf(stderr, "inflate: %s\n", (z.msg) ? z.msg : "???");
            exit(1);
        }
        if (z.avail_out == 0) {
            /* 出力バッファが尽きればループを抜ける */
            break;
        }
    }

    /* 後始末 */
    if (inflateEnd(&z) != Z_OK) {
        fprintf(stderr, "inflateEnd: %s\n", (z.msg) ? z.msg : "???");
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



void parse_object_header(char in_file_name[], object_info *oi)
{
    int status;

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

    z.next_out = (Bytef *) header;
    z.avail_out = sizeof(header);
    status = Z_OK;

    if (z.avail_in == 0) {  /* 入力残量がゼロになれば */
	z.next_in = (Bytef *) inbuf;  /* 入力ポインタを元に戻す */
	z.avail_in = fread(inbuf, 1, INBUFSIZ, fin); /* データを読む */
    }

    /* 展開 */
    status = inflate(&z, Z_NO_FLUSH);
    if (status != Z_STREAM_END && status != Z_OK) {
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


/**
 * writing object
 */
#define ZLIB_BUF_MAX ((uInt) 1024 * 1024 * 1024) /* 1GB */

static inline uInt zlib_buf_cap(unsigned long len)
{
	return (ZLIB_BUF_MAX < len) ? ZLIB_BUF_MAX : len;
}

static void zlib_pre_call(git_zstream *s)
{
	s->z.next_in = s->next_in;
	s->z.next_out = s->next_out;
	s->z.total_in = s->total_in;
	s->z.total_out = s->total_out;
	s->z.avail_in = zlib_buf_cap(s->avail_in);
	s->z.avail_out = zlib_buf_cap(s->avail_out);
}

static void zlib_post_call(git_zstream *s)
{
	unsigned long bytes_consumed;
	unsigned long bytes_produced;

	bytes_consumed = s->z.next_in - s->next_in;
	bytes_produced = s->z.next_out - s->next_out;
	if (s->z.total_out != s->total_out + bytes_produced) {
		printf("BUG: total_out mismatch");
		exit(1);
	}
	if (s->z.total_in != s->total_in + bytes_consumed) {
	    printf("BUG: total_in mismatch");
		exit(1);
	}
	s->total_out = s->z.total_out;
	s->total_in = s->z.total_in;
	s->next_in = s->z.next_in;
	s->next_out = s->z.next_out;
	s->avail_in -= bytes_consumed;
	s->avail_out -= bytes_produced;
}

static const char *zerr_to_string(int status)
{
	switch (status) {
	case Z_MEM_ERROR:
		return "out of memory";
	case Z_VERSION_ERROR:
		return "wrong version";
	case Z_NEED_DICT:
		return "needs dictionary";
	case Z_DATA_ERROR:
		return "data stream error";
	case Z_STREAM_ERROR:
		return "stream consistency error";
	default:
		return "unknown error";
	}
}


void git_deflate_init(git_zstream *strm, int level)
{
	int status;

	zlib_pre_call(strm);
	status = deflateInit(&strm->z, level);
	zlib_post_call(strm);
	if (status == Z_OK)
		return;
	printf("deflateInit: %s (%s)", zerr_to_string(status),
	    strm->z.msg ? strm->z.msg : "no message");
	exit(1);
}
#define MAX_IO_SIZE (8*1024*1024)

ssize_t xwrite(int fd, const void *buf, size_t len)
{
	ssize_t nr;
	if (len > MAX_IO_SIZE)
	    len = MAX_IO_SIZE;
	while (1) {
		nr = write(fd, buf, len);
		if ((nr < 0) )
			continue;
		return nr;
	}
}


ssize_t write_in_full(int fd, const void *buf, size_t count)
{
	const char *p = buf;
	ssize_t total = 0;

	while (count > 0) {
		ssize_t written = xwrite(fd, p, count);
		if (written < 0)
			return -1;
		if (!written) {
//		    errno = 777;
			return -1;
		}
		count -= written;
		p += written;
		total += written;
	}

	return total;
}

static int write_buffer(int fd, const void *buf, size_t len)
{
	if (write_in_full(fd, buf, len) < 0)
	    exit(1);
	return 0;
}

int git_deflate(git_zstream *strm, int flush)
{
	int status;

	for (;;) {
		zlib_pre_call(strm);

		/* Never say Z_FINISH unless we are feeding everything */
		status = deflate(&strm->z,
				 (strm->z.avail_in != strm->avail_in)
				 ? 0 : flush);
		if (status == Z_MEM_ERROR) {
			printf("deflate: out of memory");
			exit(1);
		}
		zlib_post_call(strm);

		/*
		 * Let zlib work another round, while we can still
		 * make progress.
		 */
		if ((strm->avail_out && !strm->z.avail_out) &&
		    (status == Z_OK || status == Z_BUF_ERROR))
			continue;
		break;
	}

	switch (status) {
	/* Z_BUF_ERROR: normal, needs more space in the output buffer */
	case Z_BUF_ERROR:
	case Z_OK:
	case Z_STREAM_END:
		return status;
	default:
		break;
	}
	printf("deflate: %s (%s)", zerr_to_string(status),
	      strm->z.msg ? strm->z.msg : "no message");
	return status;
}

int git_deflate_end_gently(git_zstream *strm)
{
	int status;

	zlib_pre_call(strm);
	status = deflateEnd(&strm->z);
	zlib_post_call(strm);
	return status;
}

int zlib_compression_level = Z_BEST_SPEED;
void git_write_loose_object(const unsigned char *sha1, char *hdr, int hdrlen,
			      const void *buf, unsigned long len, time_t mtime)
{
	int fd, ret;
	unsigned char compressed[4096];
	git_zstream stream;

	char filename[256];

	sha1_file_name(sha1, filename);
	FILE *fp;
	fp = fopen("/tmp/git_tmpfile", "wb");
	fd = fileno(fp);

	/* Set it up */
	memset(&stream, 0, sizeof(stream));
	git_deflate_init(&stream, zlib_compression_level);
	stream.next_out = compressed;
	stream.avail_out = sizeof(compressed);

	/* First header.. */
	stream.next_in = (unsigned char *)hdr;
	stream.avail_in = hdrlen;
	while (git_deflate(&stream, 0) == Z_OK)
		; /* nothing */


	/* Then the data itself.. */
	stream.next_in = (void *)buf;
	stream.avail_in = len;
	do {
		ret = git_deflate(&stream, Z_FINISH);

		if (write_buffer(fd, compressed, stream.next_out - compressed) < 0)
			printf("unable to write sha1 file");
		stream.next_out = compressed;
		stream.avail_out = sizeof(compressed);
	} while (ret == Z_OK);

	if (ret != Z_STREAM_END)
		printf("unable to deflate new object %s (%d)", sha1_to_hex(sha1), ret);
	ret = git_deflate_end_gently(&stream);
	if (ret != Z_OK)
		printf("deflateEnd on object %s failed (%d)", sha1_to_hex(sha1), ret);


	fclose(fp);

	rename("/tmp/git_tmpfile", filename);
	return ;
}
