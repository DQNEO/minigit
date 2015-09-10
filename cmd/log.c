#include "../minigit.h"

int cmd_log(int argc , char **argv)
{
    char rev[256];
    if (argc > 1) {
        strcpy(rev, argv[1]);
    } else {
        // default "HEAD"
        strcpy(rev, "HEAD");
    }

    char commit_sha1[256] = {};
    _rev_parse(rev, commit_sha1);

    char parent_sha1[256] = {};

    while(cat_commit_object(commit_sha1, parent_sha1) != -1){
        strcpy(commit_sha1, parent_sha1);
    }
    return 0;
}
