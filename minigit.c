#include "minigit.h"

/**
 * @return bool
 */
int is_git_directory(const char *suspect)
{
  char path_to_gitdir[PATH_MAX + 1];
  strcpy(path_to_gitdir, suspect);
  strcpy(path_to_gitdir + strlen(suspect), "/.git");
  if (access(path_to_gitdir, X_OK)) {
    return 0; // false
  }

  return 1; // true
}


int find_git_root_directory(char* cwd, size_t cwd_size)
{
  int offset;

  if (!getcwd(cwd, cwd_size - 1)) {
    fprintf(stderr, "Unable to read cwd");
    return 1;
  }
  offset = strlen(cwd);

  while (offset > 1) {
    if (is_git_directory(cwd)) {
      printf(".git found: %s/.git\n", cwd);
      return 0;
    }

    while (offset-- && cwd[offset] != '/') ;
    cwd[offset] = '\0';
  }

  return 0;
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

/**
 * @return 0:OK -1:not found
 */
int find_file(const char *sha1_input, char *matched_filename)
{
    int ret = -1;
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
                  ret = 0;
		  break;
	      }


	  }

	}

	closedir(dp);

        return ret;
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
        fprintf(stderr, "_rev_parse:Can't open %s\n", filename);
        exit(1);
    }

    fgets(sha1_string, 41, fp);
    fclose(fp);

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
    if (find_file(sha1_input, found_filename) == -1) {
        fprintf(stderr, "file not found: %s\n", found_filename);
        exit(1);
    }

    oi.buf = buf;
    parse_object_header(found_filename, &oi);
    read_object_body(found_filename, &oi);

    commit cmt;
    commit *pcmt = &cmt;

    parse_commit_object(&oi, pcmt);

    // print commit
    printf("commit %s\n", sha1_string);


    strcpy(parent_sha1 , pcmt->parent_sha1);

    printf("Author: %s\n", pcmt->author_name);
    printf("Date:   %s\n", pcmt->frmted_time);
    //printf("Parent:   %s\n", pcmt->parent_sha1);
    printf("\n");
    pretty_print_commit_message(pcmt->message);
    printf("\n");

    return 0;
}



void usage() {
    fprintf(stdout, "minigit v0.0.2 @DQNEO\n");
    fprintf(stdout, "Usage: %s <command> [<args>]\n" , PROGNAME);
    fprintf(stdout, "  cat-file: (-s|-t|-p) <object>\n");
    fprintf(stdout, "  rev-parse (HEAD|branchanme)\n");
    fprintf(stdout, "  log: (HEAD|branchname)\n");
    fprintf(stdout, "  ls-files [--stage]\n");
    fprintf(stdout, "  hash-object [-w] <file>\n");
    exit(1);
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


int main(int argc, char *argv[])
{
    if (argc == 1) {
	usage();
    }

    char *sub_cmd = argv[1];
    argc--;
    argv++;

    if (strcmp(sub_cmd, "cat-file") == 0) {
	return cmd_cat_file(argc, argv);
    } else if (strcmp(sub_cmd, "log") == 0) {
	return cmd_log(argc, argv);
    } else if (strcmp(sub_cmd, "rev-parse") == 0) {
	return cmd_rev_parse(argc, argv);
    } else if (strcmp(sub_cmd, "init") == 0) {
	return cmd_init(argc, argv);
    } else if (strcmp(sub_cmd, "hash-object") == 0) {
	return cmd_hash_object(argc, argv);
    } else if (strcmp(sub_cmd, "commit") == 0) {
	return cmd_commit(argc, argv);
    } else if (strcmp(sub_cmd, "ls-files") == 0) {
	return cmd_ls_files(argc, argv);
    } else {
        fprintf(stderr, "Unknown command: %s\n", sub_cmd);
	usage();
    }

    return 0;
}
