# Manual Test of minigi

.git/objects を扱うテストを自動化しようとすると、サンプルオブジェクトを準備するのが大変なので、
その仕組みができるまでは(半)手動テストでテストする

## cat-fileのテスト

任意のGitレポジトリの `find .git/objects/[^pi]* -type f` の結果を、minigit とgit のcat-fileで比較すればテストできる。

