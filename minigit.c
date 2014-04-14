#include "minigit.h"

/**
 * git:date.cのtime_to_tmおよびgm_time_tから借用
 */
struct tm *time_to_tm(unsigned long time, int tz)
{
    int minutes;

    minutes = tz < 0 ? -tz : tz;
    minutes = (minutes / 100)*60 + (minutes % 100);
    minutes = tz < 0 ? -minutes : minutes;
    time_t t = (time_t) time + minutes * 60;
    return gmtime(&t);
}

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

void pretty_print_tree_object(object_info *oi)
{

    //ヘッダー部は読み飛ばす
    char *cp = oi->buf + oi->header_length;
    //ボディのサイズはヘッダに書かれてあるのを参照する
    char *end = oi->buf + oi->header_length + oi->size;

    while (cp < end) {

	char mode[6];
	char *filename;

	int j = 0;

	char *obj_type;

	// filemode
	// 6桁または5桁。' 'で終端
	while (*cp != ' ') {
	    mode[j++] = *(cp++);
	}
	mode[j] = 0;

	if (strlen(mode) == 5) {
	    obj_type = "tree";
	} else {
	    obj_type = "blob";
	}

	cp++; // skip ' '

	// filename
	// nullで終端
	filename = cp;
	while(*cp++) {
	}

	//sha1
	//固定長で20文字
	char *sha1_string = sha1_to_hex((unsigned char *)cp);

	cp += 20;

	// pretty print
	printf("%06d %s %s\t%s\n",
	       atoi(mode),
	       obj_type,
	       sha1_string,
	       filename
	    );

	continue;
    }


}

/**
 * test if a string is a substring of another;
 * @return bool
 */
int is_substr(const char *s,const char *l)
{
  while (*s) {
    if (*(s++) != *(l++)) {
      return 0;
    }
  }

  return 1;
}

void validate_sha1(const char *sha1_input)
{
    // no problem if emtpy string
    if (*sha1_input == '\0') {
	return;
    }
    //防御的プログラミング！ってこうですか＞＜
    if ( !('0' <= *sha1_input && *sha1_input <= '9' ) &&
	 !('a' <= *sha1_input && *sha1_input <= 'f') ) {
	fprintf(stderr, "invalid sha1 :%s\n", sha1_input);
	exit(1);
    }
}

void find_file(const char *sha1_input, char *matched_filename)
{
    validate_sha1(sha1_input);

    char sha1_input_firsrt2chars[2];
    const char *sha1_input_from3rd = &sha1_input[2];

    //printf("3rd = %s\n", sha1_input_from3rd);
    sha1_input_firsrt2chars[0] = sha1_input[0];
    sha1_input_firsrt2chars[1] = sha1_input[1];
    sha1_input_firsrt2chars[2] = 0;
    //printf("sha1 first2 = %s\n", sha1_input_firsrt2chars);

    char dir[256] = ".git/objects/";
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;

    strcat(dir, sha1_input_firsrt2chars);

    if(( dp = opendir(dir) ) == NULL ){
	perror(dir);
	fprintf(stderr, "sha1_input = %s\n", sha1_input);
	exit( EXIT_FAILURE );
    }

    while((entry = readdir(dp)) != NULL){
	stat(entry->d_name, &statbuf);
	if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) {
	    continue;
	} else {
	    //printf("entry->d_name =  %s\n", entry->d_name);

	    //ユーザ入力のsha1とファイル名を比較して、
	    //前者が後者の先頭部分一致すればそれが目的のオブジェクトであるとみなす。
	      if (is_substr(sha1_input_from3rd, entry->d_name)) {
		  // match!
		  //printf("match :%s\n", entry->d_name );
		  strcpy(matched_filename, dir);
		  strcat(matched_filename, "/");
		  strcat(matched_filename, entry->d_name);
		  //printf("matched_filename = %s\n", matched_filename);
		  //filename = matched_filename;
		  break;
	      }


	  }

	}

	closedir(dp);


}

int _rev_parse(const char *_rev, char *sha1_string)
{
    char filename[256] = {};
    const char *dir = ".git/refs/heads/";
    char rev[100];
    //HEADの場合は強引にmasterに変更する
    if (strcmp(_rev, "HEAD") == 0) {
	strcpy(rev, "master");
    } else {
	strcpy(rev, _rev);
    }

    //引数が40文字なら、強引にsha1とみなす。
    if (strlen(rev) == 40) {
	strcpy(sha1_string, rev);
	return 0;
    }
    strcat(filename, dir);
    strcat(filename, rev);

    FILE *fp;

    if ((fp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "Can't open %s\n", filename);
        exit(1);
    }

    fgets(sha1_string, 41, fp);
    fclose(fp);

    return 0;
}



int cmd_rev_parse(int argc, char **argv)
{
    if (argc < 2) {
	return 0;
    }

    char sha1_string[256];
    char rev[256];
    strcpy(rev, argv[1]);

    _rev_parse(rev, sha1_string);

    printf("%s\n", sha1_string);

    return 0;
}

int cmd_cat_file(int argc, char **argv)
{
    struct _TAG_OBJECT_INFO oi;
    char *filename;


    oi.header_length = 0;

    char *opt = argv[1];
    char *object_name = argv[2];

    // statでファイルの存在チェックができる。
    // http://d.hatena.ne.jp/dew_false/20070726/p1
    struct stat st;
    if (stat(object_name, &st) == 0) {
	//引数をそのままファイル名として使用(gitにはない機能)
	filename = object_name;
    } else {
	//引数をsha1(の短縮文字列)とみなす
	char found_filename[256];
	char *sha1_input = object_name;
	find_file(sha1_input, found_filename);
	// .git/objects/01/2345... を探索すする
	// なければエラー終了
	//printf("final filename = %s\n", found_filename);
	filename = found_filename;
    }

    if (strcmp(opt, "-s") == 0) {
	// show size
	parse_object_header(filename, &oi);
	printf("%d\n", oi.size);
    } else if (strcmp(opt, "-t") == 0) {
	// show type
	parse_object_header(filename, &oi);
	printf("%s\n", oi.type);
    } else if (strcmp(opt, "-p") == 0) {
	// pretty print
	parse_object_header(filename, &oi);

	if (strcmp(oi.type, "tree") == 0) {
	    read_object_body(filename, &oi); //ここがバグってるっぽい
	    pretty_print_tree_object(&oi);
	    //fwrite(oi.buf + oi.header_length , 1, oi.size, stdout);
	} else {
	    // print blob or commit
	    read_object_body(filename, &oi);
	    fwrite(oi.buf + oi.header_length , 1, oi.size, stdout);
	}

    } else {
        fprintf(stderr, "Unknown flag: %s\n", opt);
        exit(1);
    }

    return 0;
}

void pretty_print_commit_message(char *cp)
{

    int start_of_new_line = 1;
    while (*cp) {

	if (start_of_new_line || *(cp -1) == '\n') {
	    printf("    ");
	}

	printf("%c", *cp);
	cp++;
	start_of_new_line = 0;
    }

}

void parse_commit_object(object_info *oi, commit *pcmt)
{
    //ヘッダー部は読み飛ばす
    char *cp = oi->buf + oi->header_length;
    //ボディのサイズはヘッダに書かれてあるのを参照する
    //char *end = oi->buf + oi->header_length + oi->size;

    char tree_sha1[41];
    char parent_sha1[41];
    char author_name[256];
    char frmted_time[256];

    /**
     * spec of commit object body
     * -------
     * tree <sha1>
     * parent <sha1>
     * [parent <sha1>
     * author  ....
     * committer ....
     *
     * message
     * --------
     */

    // skip 'tree '
    cp += 5;
    int i = 0;

    while (*cp != '\n') {
	tree_sha1[i++] = *(cp++);
    }
    tree_sha1[40] = '\0';

    cp++;

    // skip 'parent '
    if (*cp == 'p') {
	cp += 7;
	i = 0;
	while (*cp != '\n') {
	    parent_sha1[i++] = *(cp++);
	}
	parent_sha1[40] = '\0';
	cp++; // skip '\n'
    } else {
	memset(parent_sha1, '\0', 41);
    }

    validate_sha1(parent_sha1);
    //マージコミットの場合はまたparentがある。

    cp += 7; // skip 'author '
    i = 0;
    while (*cp != '>') {
	author_name[i++] = *(cp++);
    }

    author_name[i++] = *(cp++);
    author_name[i] = '\0';

    cp++;  //skip ' '
    char str_timestamp[11];
    for (i=0;i<10;i++) {
	str_timestamp[i] = *(cp++);
    }
    str_timestamp[10] = '\0';

    char timediff[6];
    cp++; //skip ' '

    for (i=0;i<5;i++) {
	timediff[i] = *(cp++);
    }
    timediff[5] = '\0';

    while (*(++cp) != '\n') ;
    cp+=2; //skip \n and \n

    time_t t;
    t = atoi(str_timestamp);
    struct tm *tm;

    int tz = atoi(timediff);

    tm = time_to_tm(t, tz);

    // see show_date in date.c#L207
    sprintf(frmted_time ,"%.3s %.3s %d %02d:%02d:%02d %d%c%+05d",
	 weekday_names[tm->tm_wday],
	 month_names[tm->tm_mon],
	 tm->tm_mday,
	 tm->tm_hour, tm->tm_min, tm->tm_sec,
	 tm->tm_year + 1900,
	 ' ',
	 tz
	 );

    strcpy(pcmt->tree_sha1, tree_sha1);
    strcpy(pcmt->parent_sha1, parent_sha1);
    strcpy(pcmt->frmted_time, frmted_time);
    strcpy(pcmt->author_name, author_name);
    pcmt->message = cp;
}

int cat_commit_object(const char *sha1_string, char *parent_sha1)
{
    struct _TAG_OBJECT_INFO oi;
    char buf[OUTBUFSIZ];


    oi.header_length = 0;

    //引数をsha1(の短縮文字列)とみなす
    char found_filename[256];
    const char *sha1_input = sha1_string;
    find_file(sha1_input, found_filename);

    oi.buf = buf;
    parse_object_header(found_filename, &oi);
    read_object_body(found_filename, &oi);

    // print commit
    printf("commit %s\n", sha1_string);

    commit cmt;
    commit *pcmt = &cmt;

    strcpy(parent_sha1 , pcmt->parent_sha1);

    printf("Author: %s\n", pcmt->author_name);
    printf("Date:   %s\n", pcmt->frmted_time);
    //printf("Parent:   %s\n", pcmt->parent_sha1);
    printf("\n");
    pretty_print_commit_message(pcmt->message);

    parse_commit_object(&oi, pcmt);
    return 0;
}


int cmd_log(int argc , char **argv)
{
    char rev[256];
    if (argc > 1) {
	strcpy(rev, argv[1]);
    } else {
	strcpy(rev, "HEAD");
    }

    char commit_sha1[256] = {};
    _rev_parse(rev, commit_sha1);

    char parent_sha1[256] = {};

    cat_commit_object(commit_sha1, parent_sha1);
    while (parent_sha1[0]) {
	printf("\n");
	cat_commit_object(parent_sha1, parent_sha1);
    }
    return 0;
}

void usage() {
    fprintf(stdout, "minigit v0.0.2 @DQNEO\n");
    fprintf(stdout, "Usage: %s <command> [<args>]\n" , PROGNAME);
    fprintf(stdout, "  cat-file: (-s|-t|-p) <object>\n");
    fprintf(stdout, "  rev-parse (HEAD|branchanme)\n");
    fprintf(stdout, "  log: (HEAD|branchname)\n");
    fprintf(stdout, "  hash-object [-w] <file>\n");
    exit(1);
}


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
static void git_write_loose_object(const unsigned char *sha1, char *hdr, int hdrlen,
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


int cmd_hash_object(int argc, char *argv[])
{
    char *filename;
    int do_write;
    if (strcmp(argv[1], "-w") == 0) {
	do_write = 1;
	filename = argv[2];
    } else {
	do_write = 0;
	filename = argv[1];
    }

    unsigned char sha1[41];
    struct stat st;

    if (lstat(filename, &st)) {
	fprintf(stderr, "unable to lstat %s\n", filename);
    }

    unsigned char *buf;
    buf = malloc(st.st_size);

    FILE *fp;
    fp = fopen(filename, "rb");
    fread(buf, st.st_size, 1, fp);
    fclose(fp);

    calc_sha1("blob", buf, st.st_size, sha1);

    if (do_write) {
	char hdr[1024];
	char *obj_type = "blob";
	sprintf(hdr, "%s %ld", obj_type ,(long) st.st_size);

	int hdrlen = strlen(hdr) + 1;
	git_write_loose_object(sha1, hdr, hdrlen, buf, st.st_size, 0);
	//write_loose_object(sha1, hdr, hdrlen, buf, st.st_size);
    }

    free(buf);
    printf("%s\n", sha1_to_hex(sha1));
    return 0;
}
int update_ref(const char *new_sha1_string)
{
    char *filename = ".git/refs/heads/master";
    FILE *fp;
    if ((fp = fopen(filename, "w")) == NULL) {
	fprintf(stderr, "unable to open %s\n", filename);
	exit(1);
    }

    fprintf(fp,"%s\n", new_sha1_string);
    fclose(fp);

    return 1;
}

int cmd_commit(int argc, char *argv[])
{
    /**
     * - 対象となるtreeハッシュ値を取得
     * - commitオブジェクトを作成
     * - commitオブジェクトを保存
     * - refs/heads/{$branch} に新コミットハッシュ値を書き込み
     */

    char tree_sha1[41] = "decd3339b94705aefe6229c1b54150dc7f04c389";
    char *message = argv[2];

    unsigned char new_sha1[21];
    char parent[41] = "";
    char buf[4096]; // FIXME: this may cause buffer overflow
    char *author = "DQNEO <dqneoo@example.com> 1381754277 +0900";
    char *commiter = "DQNEO <dqneoo@example.com> 1381754277 +0900";

    _rev_parse("HEAD", parent);

    sprintf(buf, "tree %s\nparent %s\nauthor %s\ncommiter %s\n\n%s\n",
	   tree_sha1,
	   parent,
	   author,
	   commiter,
	   message);
    size_t obj_size = strlen(buf) + 1;
    calc_sha1("commit", buf, obj_size, new_sha1);

    char hdr[1024];
    char *obj_type = "commit";
    sprintf(hdr, "%s %ld", obj_type ,(long) obj_size);

    int hdrlen = strlen(hdr) + 1;
    git_write_loose_object(new_sha1, hdr, hdrlen, buf, obj_size,0);

    /*
    if (! update_ref(sha1_to_hex(new_sha1))) {
	fprintf(stderr, "unable to update_ref by %s\n", new_sha1_string);
	exit(1);
    }
    */
    printf("[master %s] %s\n", sha1_to_hex(new_sha1), message);
    return 0;
}

int cmd_init(int argc, char *argv[])
{
    mkdir(".git", 0755);
    mkdir(".git/objects", 0755);
    mkdir(".git/refs", 0755);

    FILE *fp;
    if ((fp = fopen(".git/HEAD","w")) == NULL) {
	fprintf(stderr, "unable to create .git/HEAD");
	exit(1);
    }

    fprintf(fp, "ref: refs/heads/master\n");
    fclose(fp);
    return 0;
}

/**
 * see the document of .git/index:
 * https://github.com/gitster/git/blob/master/Documentation/technical/index-format.txt
 */
int cmd_ls_files(int argc, char *argv[])
{
    int fd;
    char *mm;
    struct stat st;
    struct cache_header *hdr;

    fd = open(".git/index", O_RDONLY);
    if (fd < 0) {
	fprintf(stderr, "unable to create .git/HEAD");
	exit(1);
    }

    if (fstat(fd, &st)) {
	fprintf(stderr, "unable to fstat index");
	exit(1);
    }

    mm = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    hdr = (struct cache_header *)mm;
    printf("hdr_signature=%s\n", hdr->hdr_signature);
    printf("hdr_version=%x\n", bswap32(hdr->hdr_version));
    printf("hdr_entries=%x\n", bswap32(hdr->hdr_entries));
    return 0;
}

void read_object_body(char in_file_name[], object_info *oi)
{
    FILE *fin;

    if ((fin = fopen(in_file_name, "r")) == NULL) {
        fprintf(stderr, "Can't open %s\n", in_file_name);
        exit(1);
    }

    z_stream z;
    char inbuf[INBUFSIZ];
    int count, status;

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
}



int main(int argc, char *argv[])
{
    if (argc == 1) {
	usage();
    }

    if (strcmp(argv[1], "cat-file") == 0) {
	return cmd_cat_file(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "log") == 0) {
	return cmd_log(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "rev-parse") == 0) {
	return cmd_rev_parse(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "init") == 0) {
	return cmd_init(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "hash-object") == 0) {
	return cmd_hash_object(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "commit") == 0) {
	return cmd_commit(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "ls-files") == 0) {
	return cmd_ls_files(argc - 1, argv + 1);
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
	usage();
    }

    return 0;
}
