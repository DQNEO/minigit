#!/usr/bin/env perl
#
# zlibによる圧縮・展開が可逆的であることを確認するテスト
#
use strict;
use warnings;
use autodie;
use Test::More;

# プログラム名
our $PROG = 'zlib';

# テスト対象テキストファイル
my @files = ('hello', 'bocchan');
my($ret, $orig_text);

for my $file (@files) {
    my $text_file = $file . '.txt';
    my $compressed_file = $file . '.zlib';
    # ゴミ掃除
    unlink $compressed_file if -e $compressed_file;

    # 圧縮
    `./$PROG -c $text_file $compressed_file`;

    # 展開して、元のテキストファイルと同一かを確認
    is `./$PROG -d $compressed_file /dev/stdout`, `cat $text_file`, 'compare';

    # ゴミ掃除
    unlink $compressed_file if -e $compressed_file;
}

done_testing;
