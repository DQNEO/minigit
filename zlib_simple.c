// Copyright 2010 Susumu Yata <syata@acm.org>
// Usage: gcc zlib_simple.c -std=c99 -lz -o zlib_simple ; ./zlib_simple hello.blob
#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#define ERROR(fmt, ...) \
  error_at_line(-(__LINE__), errno, __FILE__, __LINE__, fmt, ## __VA_ARGS__)

enum IOBufSize {
  INPUT_BUF_SIZE = 65536,  // 入力に使うバッファのサイズ
  OUTPUT_BUF_SIZE = 65536  // 出力に使うバッファのサイズ
};

Bytef input_buf[INPUT_BUF_SIZE];
Bytef output_buf[OUTPUT_BUF_SIZE];

void InflateEnd(z_stream *stream) {
}

void Inflate(FILE *input_file, FILE *output_file) {
  z_stream stream;

  // メモリの確保・解放は zlib に任せます．
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;

  // .next_in, .avail_in は inflateInit(), inflateInit2() を呼び出す前に
  // 初期化しておく必要があります．
  stream.next_in = Z_NULL;
  stream.avail_in = 0;

  int retInit = inflateInit2(&stream, 47);
  if (retInit != Z_OK) {
    // inflateInit(), inflateInit2() はエラーが起きても .msg を更新しません．
    // エラーメッセージの取得には zError() を利用することになります．
    ERROR("%s", zError(retInit));
  }

  int ret = Z_OK;
  do {
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
    do {
      // 出力バッファを再設定して伸長の続きをおこないます．
      stream.next_out = output_buf;
      stream.avail_out = sizeof(output_buf);
      ret = inflate(&stream, Z_NO_FLUSH);
      switch (ret) {
        case Z_NEED_DICT: {
          // 辞書がなければ伸長できないのでエラーにします．
          ERROR("%s", zError(Z_DATA_ERROR));
        }
        case Z_STREAM_ERROR:
        case Z_DATA_ERROR:
        case Z_MEM_ERROR: {
          ERROR("%s", stream.msg);
        }
      }
      fwrite(output_buf, sizeof(output_buf) - stream.avail_out,
          1, output_file);
      if (ferror(output_file) != 0) {
        ERROR("failed to write into file");
      }
      // 一つのファイルに複数のデータが格納されているかもしれないので，
      // 返り値が Z_STREAM_END であっても，入力が残っている状態であれば，
      // 内部状態をリセットして伸長を継続します．
      if ((ret == Z_STREAM_END) &&
          ((stream.avail_in != 0) || (feof(input_file) == 0))) {
        ret = inflateReset(&stream);
        if (ret != Z_OK) {
          ERROR("%s", zError(ret));
        }
        stream.avail_out = 0;
      }
    } while ((stream.avail_out == 0) && (ret != Z_STREAM_END));
  } while (ret != Z_STREAM_END);

  int retEnd = inflateEnd(&stream);
  if (retEnd != Z_OK) {
    // deflateEnd() はエラーが起きても .msg を更新しません．
    // エラーメッセージの取得には zError() を利用することになります．
    ERROR("%s", zError(retEnd));
  }

}

int main(int argc, char *argv[]) {

  // 入力ファイルの指定がなければ，エラー
  if (argc < 2) {
    ERROR("no file name input");
  }

  FILE *output_file = stdout;

  const char *input_file_name = argv[1];
  FILE *input_file = fopen(input_file_name, "rb");
  if (input_file == NULL) {
    ERROR("%s", input_file_name);
  }

  Inflate(input_file, output_file);
  if (fclose(input_file) != 0) {
    ERROR("%s", input_file_name);
  }

  return 0;
}
