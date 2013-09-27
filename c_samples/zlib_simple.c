// Copyright 2010 Susumu Yata <syata@acm.org>
// Usage: gcc zlib_simple.c -std=c99 -lz -o zlib_simple ; ./zlib_simple hello.blob
#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <string.h>

#define ERROR(fmt, ...) \
  error_at_line(-(__LINE__), errno, __FILE__, __LINE__, fmt, ## __VA_ARGS__)

struct _TAG_OBJECT_INFO {
  char type[10];
  char size[10];
} ;

void unpack_header(const char *input_file_name,  struct _TAG_OBJECT_INFO *oi) {

  FILE *input_file = fopen(input_file_name, "rb");
  if (input_file == NULL) {
    ERROR("%s", input_file_name);
  }


  Bytef input_buf[65536];
  char output_buf[65536];
  char *cp;
  z_stream stream;
  cp = output_buf;
  // メモリの確保・解放は zlib に任せます．
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;

  // .next_in, .avail_in は inflateInit(), inflateInit2() を呼び出す前に
  // 初期化しておく必要があります．
  stream.next_in = Z_NULL;
  stream.avail_in = 0;

  int retInit = inflateInit(&stream);
  if (retInit != Z_OK) {
    // inflateInit(), inflateInit2() はエラーが起きても .msg を更新しません．
    // エラーメッセージの取得には zError() を利用することになります．
    ERROR("%s", zError(retInit));
  }

  int ret = Z_OK;
  while (ret != Z_STREAM_END) {
    stream.avail_in = fread(input_buf, 1, sizeof(input_buf), input_file);
    if (ferror(input_file) != 0) {
      ERROR("failed to read from file");
    }
    if (stream.avail_in == 0) {
      // 入力が既に尽きている状態であれば，意図していないファイルの終端に
      // 到達したことになります．
      ERROR("unexpected end of file");
    }
    stream.next_in = input_buf;
    while ((stream.avail_out == 0) && (ret != Z_STREAM_END)) {
      stream.next_out = cp;
      stream.avail_out = sizeof(output_buf);
      ret = inflate(&stream, Z_NO_FLUSH);
      if (ret == Z_MEM_ERROR) {
          ERROR("%s", stream.msg);
      }

    } ;
  };

  int i = 0;
  while (*(cp) != ' ') {
    oi->type[i++] = *(cp++);
  }
  oi->type[i] = 0;


  i = 0;
  while (*cp) {
    oi->size[i++] = *(cp++);
  }
  oi->size[i] = 0;

  int retEnd = inflateEnd(&stream);
  if (retEnd != Z_OK) {
    // deflateEnd() はエラーが起きても .msg を更新しません．
    // エラーメッセージの取得には zError() を利用することになります．
    ERROR("%s", zError(retEnd));
  }

  if (fclose(input_file) != 0) {
    ERROR("%s", input_file_name);
  }


}

int main(int argc, char *argv[]) {

  // 入力ファイルの指定がなければ，エラー
  if (argc < 2) {
    ERROR("no file name input");
  }

  struct _TAG_OBJECT_INFO *object_info;
  const char *input_file_name = argv[1];

  unpack_header(input_file_name, object_info);
  printf("type:%s\n", object_info->type);
  printf("size:%s\n", object_info->size);

  return 0;
}
