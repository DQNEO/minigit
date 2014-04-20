#include "../minigit.h"

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
