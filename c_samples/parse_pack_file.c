/**
 * parser of git pack file  
 * spec https://github.com/git/git/blob/master/Documentation/technical/pack-format.txt
 */

#include <stdio.h>

void parse_file(char *filename)
{
    FILE *file;
    file = fopen(filename, "r");

    char ptr[256];
    fread(ptr, 4,1, file);
    printf("%s\n", ptr); // 'PACK'
    fclose(file);
}

int main(int argc, char **argv)
{
    char *filename;
    printf("[parsing packe file] %s\n", argv[1]);

    filename = argv[1];
    parse_file(filename);
}
