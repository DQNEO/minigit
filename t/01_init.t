#!/usr/bin/env perl
use strict;
use warnings;
use Test::More;

our $PROGNAME = 'minigit';

diag('cmd init');

# prepare
my $tmp_dir = "/tmp/" . $PROGNAME . "_test";
mkdir $tmp_dir or die "cannot mkdir " . $tmp_dir;


my $ret_val = system "minigit init";

is $ret_val, 0, "exit value of init";



# clean
rmdir $tmp_dir or die "cannot rmdir " . $tmp_dir;


done_testing();

