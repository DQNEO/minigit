#include "../minigit.h"

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
