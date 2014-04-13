# Manual Test of minigi

.git/objects を扱うテストを自動化しようとすると、サンプルオブジェクトを準備するのが大変なので、
その仕組みができるまでは(半)手動テストでテストする

## cat-fileのテスト

任意のGitレポジトリの `find .git/objects/[^pi]* -type f` の結果を、minigit とgit のcat-fileで比較すればテストできる。

### cat-file -tのバグ
`minigit cat-file -p manualtest/006b9f78eb6177c5284ba17158f78bf165d7383e.tree` でセグフォる。
