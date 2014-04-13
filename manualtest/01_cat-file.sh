#!/bin/bash
#
# cat-file のテスト
#
# gitとminigitでcat-fileの出力結果が同じであるかをテストする
#
# Usage:
# [thiscommand] /path/to/.git/objetcs
#
# TODO バイナリファイルだとdiffが一致しないことがあるのが謎

set -e

obj_dir=$1
CUR_DIR=$(cd $(dirname $0); pwd)
MINIGIT=$CUR_DIR/../minigit

if [ $# -eq 0 ] ; then
    echo "Usage: $0  /path/to/.git/objetcs/"
    exit 1
fi

function list_objects () {
    find ${obj_dir}[^pi]* -type f | while read line ; do ( echo ${line#.git/objects/} | sed -e s#/##) ; done
}


echo "=== compare cat-file -t ===="
list_objects | while read hash
do
    if diff <($MINIGIT cat-file -t $hash) <(git cat-file -t $hash) ; then
        echo ok $hash
    else
        echo ng $hash
    fi
done

echo "=== compare cat-file -s ===="
list_objects | while read hash
do
    if diff <($MINIGIT cat-file -s $hash) <(git cat-file -s $hash) ; then
        echo ok $hash
    else
        echo ng $hash
    fi
done

echo "=== compare cat-file -p ===="
list_objects | while read hash
do
    if diff <($MINIGIT cat-file -p $hash) <(git cat-file -p $hash) ; then
        echo ok $hash
    else
        echo ng $hash
    fi
done



