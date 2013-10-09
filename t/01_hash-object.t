#!/usr/bin/env perl
use strict;
use warnings;
use Test::More;

our $PROGNAME = 'minigit';
diag('hash-object');

my ($cmd, $ret, $exp);

$cmd = "$PROGNAME hash-object t/hello.txt";
$ret = `$cmd`;
$exp = "ce013625030ba8dba906f756967f9e9ca394464a\n";
is $ret, $exp, $cmd;

done_testing();
