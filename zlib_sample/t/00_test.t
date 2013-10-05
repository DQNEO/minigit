#!/usr/bin/env perl
use strict;
use warnings;
use autodie;
use Test::More;

our $PROGNAME = 'zlib';
my @files = ('hello', 'bocchan');
my($ret, $orig_text);

for my $file (@files) {
    my $orig = $file . '.txt';
    my $zlib = $file . '.zlib';
    unlink $zlib if -e $zlib;
    `./$PROGNAME c $orig $zlib`;
    $orig_text = `cat $orig`;
    $ret = `./$PROGNAME d $zlib /dev/stdout`;
    is $ret, $orig_text, "file $file";

    unlink $zlib if -e $zlib;
}

done_testing;
