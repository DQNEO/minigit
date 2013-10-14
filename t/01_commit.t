#!/usr/bin/env perl
use strict;
use warnings;
use autodie;
use Test::More;

our $PROGNAME = 'minigit';

diag('commit');

my ($cmd, $ret, $exp, $ret_val);

$cmd = "./$PROGNAME commit -m hoge";
$ret_val = system($cmd);

is $ret_val, 0, $cmd;

done_testing();
