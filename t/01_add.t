#!/usr/bin/env perl
use strict;
use warnings;
use Test::More;

our $PROGNAME = 'minigit';
diag('add');

my ($cmd, $ret, $exp);

diag('create test_hello.txt');

my $content = "hello\n";
my $filename = 'test_hello.txt';

open my $fh, '>' , $filename;
print $fh $content;
close $fh;

my $exit_code = system("$PROGNAME add $filename");

is $exit_code, 0 , 'add';

ok unlink($filename), "unlink $filename";

done_testing();
