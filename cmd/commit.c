#include "../minigit.h"

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

