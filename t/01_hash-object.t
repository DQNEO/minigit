#!/usr/bin/env perl
use strict;
use warnings;
use autodie;
use Test::More;

our $PROGNAME = 'minigit';

diag('hash-object');

my ($cmd, $ret, $exp);

$cmd = "./$PROGNAME hash-object t/hello.txt";
$ret = `$cmd`;
$exp = "ce013625030ba8dba906f756967f9e9ca394464a\n";
is $ret, $exp, $cmd;

diag('hash-object -w');
my $tmp_filename = "hello_" . time(). '.txt';
my $content = $tmp_filename . "\n";
open my $fh , '>', $tmp_filename;
print $fh $content;
close $fh;

$cmd = "./$PROGNAME hash-object -w $tmp_filename";
my $sha1 = `$cmd` or die "cannot " . $cmd;
chomp $sha1;
ok $sha1, "sha1 is " . $sha1;    
$ret = `./$PROGNAME cat-file -p $sha1`;
is $ret, $content, 'verify hash-object -w by cat-file -p';
unlink $tmp_filename;

done_testing();
