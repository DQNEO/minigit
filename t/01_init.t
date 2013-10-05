#!/usr/bin/env perl
use strict;
use warnings;
use Test::More;
use FindBin qw($Bin);
use lib "$Bin/lib";
use MyLib qw(slurp);
our $PROGNAME = 'minigit';

diag('cmd init');

# prepare

my $tmp_dir = "/tmp/" . $PROGNAME . "_test";
system("rm -rf $tmp_dir") if -d $tmp_dir;

mkdir $tmp_dir
      or die "cannot mkdir " . $tmp_dir;
chdir $tmp_dir
      or die "cannot chdir " . $tmp_dir;

my $ret_val = system "minigit init";

is $ret_val, 0, "exit value of init";

# assure existance of dirs
# .git/objects/
# .git/refs/
ok -d $tmp_dir, ".git";
ok -d $tmp_dir . "/.git/objects", ".git/objects";
ok -d $tmp_dir . "/.git/refs", ".git/refs";

is slurp($tmp_dir . "/.git/HEAD"), "ref: refs/heads/master\n", ".git/HEAD";


# clean
#ok system("rm -rf $tmp_dir") == 0, "clean";

done_testing();

=pod
  git init のテスト
  /tmp/下でgit initして各種ディレクトリをセットアップ
  終わったらゴミ掃除
=end

