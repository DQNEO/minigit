/**
 * parser of git pack file  
 * spec https://github.com/git/git/blob/master/Documentation/technical/pack-format.txt
 */

#include <stdio.h>
#include <arpa/inet.h>

struct pack_header {
    uint32_t hdr_signature;
    uint32_t hdr_version;
    uint32_t hdr_entries;
};

int read_pack_header(FILE *file, struct pack_header *hdr)
{
    fread(hdr, 12,1,file);
    return 0;
}

void parse_file(char *filename)
{
    FILE *file;
    file = fopen(filename, "r");

    struct pack_header hdr;

    read_pack_header(file, &hdr);
    
    printf("signature=%d\n", hdr.hdr_signature); // 'PACK'
    printf("version=%d\n", ntohl(hdr.hdr_version));  // 2
    printf("num=%d\n", ntohl(hdr.hdr_entries)); 
    fclose(file);

    return;
    
}

int main(int argc, char **argv)
{
    char *filename;
    printf("[parsing packe file] %s\n", argv[1]);

    filename = argv[1];
    parse_file(filename);
}
