/* zlib を使う例 */
/* http://oku.edu.mie-u.ac.jp/~okumura/compression/comptest.c */
/* コンパイル: gcc -Wall comptest.c -o comptest -lz */
/* -lz は libz.a をリンクする（たぶん /usr(/local)/lib にある）*/

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>               /* /usr(/local)/include/zlib.h */
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

// opendir
#include <dirent.h>
#include <time.h>
#define INBUFSIZ   1024         /* 入力バッファサイズ（任意） */
#define OUTBUFSIZ  1024         /* 出力バッファサイズ（任意） */
#define PROGNAME "minigit"

typedef struct _TAG_OBJECT_INFO {
    char type[20];
    int  size;
    int  header_length; // ヘッダのバイト長
    char *buf;
} object_info;

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

    int status;

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

    z.next_out = (Bytef *) outbuf;        /* 出力ポインタ */
    z.avail_out = OUTBUFSIZ;    /* 出力バッファ残量 */
    status = Z_OK;

    if (z.avail_in == 0) {  /* 入力残量がゼロになれば */
	z.next_in = (Bytef *) inbuf;  /* 入力ポインタを元に戻す */
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

void find_file(char *sha1_input, char *matched_filename)
{
    //printf("sha1_input:%s\n", sha1_input);

    char sha1_input_firsrt2chars[2];
    char *sha1_input_from3rd = &sha1_input[2];

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
	perror("opendir");
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

int _rev_parse(char *rev, char *sha1_string)
{
    char filename[256] = {};
    const char *dir = ".git/refs/heads/";

    if (strcmp(rev, "HEAD") == 0) {
	strcpy(rev, "master");
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

    char buf[OUTBUFSIZ];

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

    
    oi.buf = buf;

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
	    read_object_body(filename, &oi);
	    pretty_print_tree_object(&oi);
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

void pretty_print_commit_object(object_info *oi)
{
    //ヘッダー部は読み飛ばす
    char *cp = oi->buf + oi->header_length;
    //ボディのサイズはヘッダに書かれてあるのを参照する
    //char *end = oi->buf + oi->header_length + oi->size;

    char tree_sha1[41];

    //char **parents;
    char author_name[256];
    char *message;
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

    char parent_sha1[41];

    // skip 'parent '
    cp += 7;
    i = 0;
    while (*cp != '\n') {
	parent_sha1[i++] = *(cp++);
    }
    parent_sha1[40] = '\0';

    //マージコミットの場合はまたparentがある。

    cp += 8; // skip 'author '
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

    time_t t;
    t = atoi(str_timestamp);
    struct tm *ptm;
    ptm = localtime(&t);

    char timediff[6];
    cp++; //skip ' '

    for (i=0;i<5;i++) {
	timediff[i] = *(cp++);
    }
    timediff[5] = '\0';

    while (*(++cp) != '\n') ;
    cp+=2; //skip \n and \n

    message = cp;

    char frmted_time[256];
    strftime(frmted_time, 255, "%a %b%e %H:%M:%S %Y", ptm);

    printf("Author: %s\n", author_name);
    printf("Date:   %s %s\n", frmted_time, timediff);
    printf("\n    %s", cp);
    //printf("tree : %s\n", tree_sha1);
    //printf("parent : %s\n", parent_sha1);
    
}
 
int cat_commit_object(char *sha1_string)
{
    struct _TAG_OBJECT_INFO oi;
    char buf[OUTBUFSIZ];


    oi.header_length = 0;

    //引数をsha1(の短縮文字列)とみなす
    char found_filename[256];
    char *sha1_input = sha1_string;
    find_file(sha1_input, found_filename);

    oi.buf = buf;
    parse_object_header(found_filename, &oi);
    read_object_body(found_filename, &oi);

    // print commit
    printf("commit %s\n", sha1_string);
    pretty_print_commit_object(&oi);

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

    char sha1_string[256] = {};
    _rev_parse(rev, sha1_string);

    cat_commit_object(sha1_string);
    return 0;
}



void usage() {
    fprintf(stdout, "Usage: %s <command> [<args>]\n" , PROGNAME);
    fprintf(stdout, "  cat-file: (-s|-t|-p) <object>\n");
    fprintf(stdout, "  rev-parse (HEAD|branchanme)\n");
    fprintf(stdout, "  log: (HEAD|branchname)\n");
    exit(1);
}

int cmd_init(int argc, char *argv[])
{
    mkdir(".git", 0755);
    mkdir(".git/objects", 0755);
    mkdir(".git/refs", 0755);
    return 0;
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
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
	usage();
    }

    return 0;
}
