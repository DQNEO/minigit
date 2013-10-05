#!/usr/bin/env perl
use strict;
use warnings;
use autodie;
use Test::More;

# test blob object
diag('blob object');

my ($ret, $exp, $cmd);

diag 'cat-file -s';
$ret = `./minigit cat-file -s t/objects/hello_world.blob`;
$exp = "12\n";
is $ret, $exp;

$ret = `./minigit cat-file -s t/objects/f135c.tree`;
$exp = "252\n";
is $ret, $exp;

diag 'cat-file -t';
$ret = `./minigit cat-file -t t/objects/hello_world.blob`;
$exp = "blob\n";
is $ret, $exp;

$ret = `./minigit cat-file -t t/objects/f135c.tree`;
$exp = "tree\n";
is $ret, $exp;

# cat-file-p
$ret = `./minigit cat-file -p t/objects/hello_world.blob`;
$exp = "hello world
";

is $ret, $exp;

# test tree object
diag('cat tree object');

my @sha1_list = ('1e863', '44495', 'f135c');

for my $sha1_short (@sha1_list) {
    $ret = `./minigit cat-file -p t/objects/$sha1_short.tree`;

    open my $fh, '<', "t/objects/$sha1_short.tree.txt";

    $exp = do { local $/; <$fh> };

    is $ret, $exp, 'tree ' . $sha1_short;

}

diag('test commit object');

my @commits = ('0067b', '16a29');

for my $commit (@commits) {
    $ret = `./minigit cat-file -p t/objects/$commit.commit`;
    $ret =~ s/gmail/example/g;

    open my $fh, '<', "t/objects/$commit.commit.txt";

    $exp = do { local $/; <$fh> };

    is $ret, $exp, 'commit ' . $commit;

}


diag('rev-parse');
$cmd ='rev-parse HEAD';
$ret = `./minigit $cmd`;
$exp = `git rev-parse HEAD`;
is $ret , $exp, $cmd;

$cmd = 'rev-parse';
$ret = `./minigit $cmd`;
$exp = "";
is $ret , $exp, $cmd;

$cmd = 'rev-parse br_caa88';
$ret = `./minigit $cmd`;
$exp = `git $cmd`;
is $ret , $exp, $cmd;


done_testing();

=pod
    diag('test log HEAD');
$ret = `./minigit log HEAD`;
$exp = "sha1=badcafe\n";

is $ret , $exp, 'log HEAD';

diag('test log');
$ret = `./minigit log`;
$exp = "sha1=badcafe\n";

is $ret , $exp, 'log';
=end






