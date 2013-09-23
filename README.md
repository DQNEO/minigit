# C言語でzlibを使うサンプルコード

## コンパイル
( -lz オプションをつけることで、zlibがリンクされる)
```sh
gcc zlib.c -std=c99 -lz -o zlib
```

## 使いかた
```sh
./zlib --help
./zlib -i hello.blob
blob 12hello world
```
