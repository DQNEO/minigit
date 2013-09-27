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
size: 12
header_length:8
hello world
";

is $ret1, $exp1;

done_testing();




