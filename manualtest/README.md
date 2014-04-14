# Manual Test of minigi

.git/objects を扱うテストを自動化しようとすると、サンプルオブジェクトを準備するのが大変なので、
その仕組みができるまでは(半)手動テストでテストする

## cat-fileのテスト

任意のGitレポジトリの `find .git/objects/[^pi]* -type f` の結果を、minigit とgit のcat-fileで比較すればテストできる。

### cat-file -pのバグ
`minigit cat-file -p manualtest/006b9f78eb6177c5284ba17158f78bf165d7383e.tree` でセグフォる。 (解決済み！)

`minigit cat-file -p manualtest/02bb82bb5e082cf9f359e6e33ae94e304991b00c.blob` でinflateエラー(解決済み！)

`./zlib_okumura/zlib -d manualtest/02bb82bb5e082cf9f359e6e33ae94e304991b00c.blob` は動く


`./minigit cat-file -p manualtest/006a74a39f29e88405d5efe16d04b88bdd315614.tree`の結果がおかしい

