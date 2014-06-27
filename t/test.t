#!/usr/bin/env perl
use strict;
use warnings;
use autodie;
use Test::More;
use FindBin qw($Bin);
use lib "$Bin/lib";
use MyLib qw(slurp);
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

    $exp = slurp("t/objects/$sha1_short.tree.txt");
    is $ret, $exp, 'tree ' . $sha1_short;
}

diag('test commit object');

my @commits = ('0067b', '16a29');

for my $commit (@commits) {
    $ret = `./minigit cat-file -p t/objects/$commit.commit`;
    $ret =~ s/gmail/example/g;

    $exp = slurp("t/objects/$commit.commit.txt");
    is $ret, $exp, 'commit ' . $commit;
}

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




