// Copyright 2010 Susumu Yata <syata@acm.org>
// Usage: gcc zlib_simple.c -std=c99 -lz -o zlib_simple ; ./zlib_simple -i hello.blob
#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#define ERROR(fmt, ...) \
  error_at_line(-(__LINE__), errno, __FILE__, __LINE__, fmt, ## __VA_ARGS__)

enum RunMode {
  DEFLATE_MODE,  // 圧縮モード（デフォルト）
  INFLATE_MODE,  // 伸長モード
  HELP_MODE,     // ヘルプ表示
  DEFAULT_MODE = DEFLATE_MODE
};

enum OutputFormat {
  GZIP_FORMAT,  // gzip 形式
  ZLIB_FORMAT,  // zlib 形式（デフォルト）
  DEFAULT_FORMAT = ZLIB_FORMAT
};

enum IOBufSize {
  INPUT_BUF_SIZE = 65536,  // 入力に使うバッファのサイズ
  OUTPUT_BUF_SIZE = 65536  // 出力に使うバッファのサイズ
};

enum RunMode run_mode = DEFAULT_MODE;
enum OutputFormat output_format = DEFAULT_FORMAT;
const char *output_file_name = NULL;
int compression_level = Z_DEFAULT_COMPRESSION;

Bytef input_buf[INPUT_BUF_SIZE];
Bytef output_buf[OUTPUT_BUF_SIZE];

void ParseOptions(int argc, char *argv[]) {
  // getopt_long() で取得するオプションのリストです．
  // 2 番目のメンバが 1 のオプションは引数を取ります．
  static const struct option long_options[] = {
    { "deflate", 0, NULL, 'd' },  // 圧縮モード
    { "inflate", 0, NULL, 'i' },  // 伸長モード
    { "gzip", 0, NULL, 'g' },     // gzip 形式
    { "zlib", 0, NULL, 'z' },     // zlib 形式
    { "level", 1, NULL, 'l' },    // 圧縮レベル
    { "output", 1, NULL, 'o' },   // 出力ファイル
    { "help", 0, NULL, 'h' },     // ヘルプ表示
    { NULL, 0, NULL, '\0' }
  };

  // getopt_long() の第 3 引数はオプションのリストを受け取ります．
  // 引数を取るオプション文字には ':' が後続しています．
  int value;
  while ((value = getopt_long(argc, argv,
      "digzl:o:h", long_options, NULL)) != -1) {
    switch (value) {
      case 'd': {
        run_mode = DEFLATE_MODE;
        break;
      }
      case 'i': {
        run_mode = INFLATE_MODE;
        break;
      }
      case 'g': {
        output_format = GZIP_FORMAT;
        break;
      }
      case 'z': {
        output_format = ZLIB_FORMAT;
        break;
      }
      case 'l': {
        // 圧縮レベルは 0 以上 9 以下です．
        char *end_of_value;
        long value = strtol(optarg, &end_of_value, 10);
        if ((*end_of_value != '\0') || (value < 0) || (value > 9)) {
          ERROR("invalid compression level: %s", optarg);
        }
        compression_level = (int)value;
        break;
      }
      case 'o': {
        // 出力ファイルの名前を設定します．
        // 指定がなければ標準出力を使います．
        output_file_name = optarg;
        break;
      }
      case 'h': {
        run_mode = HELP_MODE;
        break;
      }
      default: {
        ERROR("invalid option");
      }
    }
  }
}

void DeflateInit(z_stream *stream) {
  // メモリの確保・解放は zlib に任せます．
  stream->zalloc = Z_NULL;
  stream->zfree = Z_NULL;
  stream->opaque = Z_NULL;

  // deflateInit() では zlib 形式になります．deflateInit2() の第 3 引数を
  // 24 以上 31 以下の値にすると gzip 形式になります．deflateInit2() で
  // zlib 形式の圧縮をする場合は，第 3 引数を 8 以上 15 以下にします．
  // deflateInit() の動作は 15 のときと同じです．
  int ret;
  switch (output_format) {
    case GZIP_FORMAT: {
      ret = deflateInit2(stream, compression_level,
          Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY);
      break;
    }
    case ZLIB_FORMAT: {
      ret = deflateInit(stream, compression_level);
      break;
    }
    default: {
      ERROR("invalid format: %d", output_format);
    }
  }

  if (ret != Z_OK) {
    // deflateInit(), deflateInit2() はエラーが起きても .msg を更新しません．
    // エラーメッセージの取得には zError() を利用することになります．
    ERROR("%s", zError(ret));
  }
}

void DeflateEnd(z_stream *stream) {
  int ret = deflateEnd(stream);
  if (ret != Z_OK) {
    // deflateEnd() はエラーが起きても .msg を更新しません．
    // エラーメッセージの取得には zError() を利用することになります．
    ERROR("%s", zError(ret));
  }
}

void Deflate(FILE *input_file, FILE *output_file) {
  z_stream stream;
  DeflateInit(&stream);
  int flush = Z_NO_FLUSH;
  int ret = Z_OK;
  do {
    stream.avail_in = fread(input_buf, 1, sizeof(input_buf), input_file);
    if (ferror(input_file) != 0) {
      ERROR("failed to read from file");
    } else if (feof(input_file) != 0) {
      // 入力が尽きれば deflate() の第 2 引数を Z_FINISH に切り替えます．
      flush = Z_FINISH;
    }
    stream.next_in = input_buf;
    do {
      // 出力バッファを再設定して圧縮の続きをおこないます．
      stream.next_out = output_buf;
      stream.avail_out = sizeof(output_buf);
      ret = deflate(&stream, flush);
      if (ret == Z_STREAM_ERROR) {
        ERROR("%s", stream.msg);
      }
      fwrite(output_buf, sizeof(output_buf) - stream.avail_out,
          1, output_file);
      if (ferror(output_file) != 0) {
        ERROR("failed to write into file");
      }
    } while ((stream.avail_out == 0) && (ret != Z_STREAM_END));
    if (stream.avail_in != 0) {
      ERROR("unexpected bytes in input buffer");
    }
  } while (flush != Z_FINISH);
  if (ret != Z_STREAM_END) {
    ERROR("failed to finish deflate");
  }
  if (fflush(output_file) != 0) {
    ERROR("failed to flush output file");
  }
  DeflateEnd(&stream);
}

void InflateInit(z_stream *stream) {
  // メモリの確保・解放は zlib に任せます．
  stream->zalloc = Z_NULL;
  stream->zfree = Z_NULL;
  stream->opaque = Z_NULL;

  // .next_in, .avail_in は inflateInit(), inflateInit2() を呼び出す前に
  // 初期化しておく必要があります．
  stream->next_in = Z_NULL;
  stream->avail_in = 0;

  int ret = inflateInit2(stream, 47);
  if (ret != Z_OK) {
    // inflateInit(), inflateInit2() はエラーが起きても .msg を更新しません．
    // エラーメッセージの取得には zError() を利用することになります．
    ERROR("%s", zError(ret));
  }
}

void InflateEnd(z_stream *stream) {
  int ret = inflateEnd(stream);
  if (ret != Z_OK) {
    // deflateEnd() はエラーが起きても .msg を更新しません．
    // エラーメッセージの取得には zError() を利用することになります．
    ERROR("%s", zError(ret));
  }
}

void Inflate(FILE *input_file, FILE *output_file) {
  z_stream stream;
  InflateInit(&stream);
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
  InflateEnd(&stream);
}

void Code(FILE *input_file, FILE *output_file) {
  switch (run_mode) {
    case DEFLATE_MODE: {
      Deflate(input_file, output_file);
      break;
    }
    case INFLATE_MODE: {
      Inflate(input_file, output_file);
      break;
    }
    default: {
      ERROR("invalid mode: %d", run_mode);
    }
  }
}

int main(int argc, char *argv[]) {
  ParseOptions(argc, argv);

  FILE *output_file = stdout;

  // 入力ファイルの指定がなければ，標準入力を使います．
  if (optind >= argc) {
    ERROR("no file name input");
  } else {
    for (int i = optind; i < argc; ++i) {
      const char *input_file_name = argv[i];
      FILE *input_file = fopen(input_file_name, "rb");
      if (input_file == NULL) {
        ERROR("%s", input_file_name);
      }

      Code(input_file, output_file);
      if (fclose(input_file) != 0) {
        ERROR("%s", input_file_name);
      }
    }
  }

  if (output_file != stdout) {
    if (fclose(output_file) != 0) {
      ERROR("%s", output_file_name);
    }
  }

  return 0;
}
