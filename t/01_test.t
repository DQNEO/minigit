#!/usr/bin/env perl
use strict;
use warnings;
use Test::More;

`make >/dev/null`;

my $ret1 = `./minigit cat-file-x t/objects/hello_world.blob`;
my $exp1 = "type:blob
size: 12
header_length:8
hello world
";

is $ret1, $exp1;

done_testing();




