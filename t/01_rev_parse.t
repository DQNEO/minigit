#!/usr/bin/env perl
use strict;
use warnings;
use Test::More;

our $PROGNAME = 'minigit';
diag('rev-parse');

my ($cmd, $ret, $exp);

my @commands = (
    'rev-parse',
    'rev-parse HEAD',
    'rev-parse br_caa88',    
    );

for my $cmd (@commands) {
    $ret = `$PROGNAME $cmd`;
    $exp = `git $cmd`;
    is $ret, $exp, $cmd;
}

done_testing();
