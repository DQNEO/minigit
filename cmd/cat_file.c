#include "../minigit.h"

int cmd_cat_file(int argc, char **argv)
{
    struct _TAG_OBJECT_INFO oi;
    char *filename;


    oi.header_length = 0;

    char *opt = argv[1];
    char *object_name = argv[2];

    // statでファイルの存在チェックができる。
    // http://d.hatena.ne.jp/dew_false/20070726/p1
    struct stat st;
    if (stat(object_name, &st) == 0) {
        //引数をそのままファイル名として使用(gitにはない機能)
        filename = object_name;
    } else {
        //引数をsha1(の短縮文字列)とみなす
        char found_filename[256] = "\0";
        char *sha1_input = object_name;
        if (find_file(sha1_input, found_filename) == -1) {
            fprintf(stderr, "loose object file not found:%s\n", object_name);
            exit(1);
        }
        // .git/objects/01/2345... を探索すする
        // なければエラー終了
        //printf("final filename = %s\n", found_filename);
        filename = found_filename;
    }

    if (strcmp(opt, "-s") == 0) {
        // show size
        parse_object_header(filename, &oi);
        printf("%d\n", oi.size);
    } else if (strcmp(opt, "-t") == 0) {
        // show type
        parse_object_header(filename, &oi);
        printf("%s\n", oi.type);
    } else if (strcmp(opt, "-p") == 0) {
        // pretty print
        parse_object_header(filename, &oi);

        if (strcmp(oi.type, "tree") == 0) {
            read_object_body(filename, &oi); //ここがバグってるっぽい
            pretty_print_tree_object(&oi);
            //fwrite(oi.buf + oi.header_length , 1, oi.size, stdout);
        } else {
            // print blob or commit
            read_object_body(filename, &oi);
            fwrite(oi.buf + oi.header_length , 1, oi.size, stdout);
        }

    } else {
        fprintf(stderr, "Unknown flag: %s\n", opt);
        exit(1);
    }

    return 0;
}
