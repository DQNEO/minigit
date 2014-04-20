#include "../minigit.h"

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
