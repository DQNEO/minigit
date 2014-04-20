#include "../minigit.h"

/**
 * see the document of .git/index:
 * https://github.com/gitster/git/blob/master/Documentation/technical/index-format.txt
 */
int cmd_ls_files_old(int argc, char *argv[])
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
    return 0;
}


