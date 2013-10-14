#!/usr/bin/env perl
use strict;
use warnings;
use autodie;
use Test::More;

our $PROGNAME = 'minigit';

diag('commit');

my ($cmd, $ret, $exp, $ret_val);

my $old_commit_sha1 = `./$PROGNAME rev-parse HEAD`;
ok $old_commit_sha1, "old sha1 " . $old_commit_sha1;

$cmd = "./$PROGNAME commit -m hoge";
$ret_val = system($cmd);
is $ret_val, 0, 'commit exit 0';

my $new_commit_sha1 = `./$PROGNAME rev-parse HEAD`;
ok $new_commit_sha1, "new sha1 " . $new_commit_sha1;

isnt $old_commit_sha1, $new_commit_sha1, 'sha1 changed';

done_testing();
