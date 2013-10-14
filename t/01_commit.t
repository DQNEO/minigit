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

diag('output echo-backs commit message');
$cmd = "./$PROGNAME commit -m hoge";
my $buffer = `$cmd`;
is $?, 0, 'commit exit 0';
diag($buffer);
ok $buffer =~ /hoge/, 'output has commit message';
ok $buffer =~ /badcafe/, 'output has commit sha1';

my $new_commit_sha1 = `./$PROGNAME rev-parse HEAD`;
#isnt $old_commit_sha1, $new_commit_sha1, 'sha1 changed';

done_testing();
