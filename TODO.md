## TODO
* subcommands
 * add
 * commit (not dummy commit)
 * show
 * log -1
* core
 * detect .git directory wherever current directory is.
* test
 * fix test `prove -r t` to work
* source structure
 * #include "../minigit.h" がなんかダサイ

## algorithm to find .git directory

setup.c:L643-L664 あたり

getcwdで取得した文字列を/区切りで順番にchdirして調査している。

