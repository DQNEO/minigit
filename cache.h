struct cache_entry {
    unsigned int ce_ctime_sec;
    unsigned int ce_ctime_nsec;
    unsigned int ce_mtime_sec;
    unsigned int ce_mtime_nsec;
    unsigned int ce_dev;
    unsigned int ce_ino;
    unsigned int ce_mode;
    unsigned int ce_uid;
    unsigned int ce_gid;
    unsigned int ce_size;
    unsigned char sha1[21];
    char namelen;
    char name[1];
};

struct index_header {
    char dirc[4];
    unsigned int version;
    unsigned int entries;
};
