#!/usr/bin/env perl
#
# zlibによる圧縮・展開が可逆的であることを確認するテスト
#
use strict;
use warnings;
use autodie;
use Test::More;

# プログラム名
our $PROGNAME = 'zlib';

# テスト対象テキストファイル
my @files = ('hello', 'bocchan');
my($ret, $orig_text);

for my $file (@files) {
    my $text_file = $file . '.txt';
    my $compressed_file = $file . '.zlib';
    unlink $compressed_file if -e $compressed_file;
    `./$PROGNAME -c $text_file $compressed_file`;
    my $cmd = "./$PROGNAME -d $compressed_file /dev/stdout";
    $ret = `$cmd`;
    is $ret, `cat $text_file`, "$cmd";

    # ゴミ掃除
    unlink $compressed_file if -e $compressed_file;
}

done_testing;
