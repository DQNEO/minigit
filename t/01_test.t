#!/usr/bin/env perl
use strict;
use warnings;
use Test::More;

# make
`make >/dev/null`;

my ($ret, $exp);
# cat-file-x
$ret = `./minigit cat-file-x t/objects/hello_world.blob`;
$exp = "type:blob
size:12
header_length:8
hello world
";

is $ret, $exp;

# cat-file-s
$ret = `./minigit cat-file-s t/objects/hello_world.blob`;
$exp = "12\n";

is $ret, $exp;

# cat-file-t
$ret = `./minigit cat-file-t t/objects/hello_world.blob`;
$exp = "blob\n";

is $ret, $exp;

done_testing();




