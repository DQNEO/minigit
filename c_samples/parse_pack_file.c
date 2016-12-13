/**
 * parser of git pack file  
 * spec https://github.com/git/git/blob/master/Documentation/technical/pack-format.txt
 */

#include <stdio.h>

void parse_file(char *filename)
{
    FILE *file;
    file = fopen(filename, "r");

    char signature[5];
    char version[5];
    fread(signature, 4,1, file);
    printf("signature=%s\n", signature); // 'PACK'

    fread(version, 4,1, file);
    printf("version=%d\n", version[3]); 

    fclose(file);
}

int main(int argc, char **argv)
{
    char *filename;
    printf("[parsing packe file] %s\n", argv[1]);

    filename = argv[1];
    parse_file(filename);
}
