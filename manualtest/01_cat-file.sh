#!/bin/bash
#
# cat-file のテスト
#
# gitとminigitでcat-fileの出力結果が同じであるかをテストする
#
# Usage:
# [thiscommand] /path/to/.git/objetcs
obj_dir=$1
CUR_DIR=$(cd $(dirname $0); pwd)
MINIGIT=$CUR_DIR/../minigit

find .git/objects/[^pi]* -type f | while read line
do
    hash=$( echo ${line#.git/objects/} | sed -e s#/##)

    # compare cat-file -t
    if diff <($MINIGIT cat-file -t $hash) <(git cat-file -t $hash) ; then
        echo ok $hash
    else
        echo ng $hash
    fi

    # compare cat-file -s
    if diff <($MINIGIT cat-file -s $hash) <(git cat-file -s $hash) ; then
        echo ok $hash
    else
        echo ng $hash
    fi

    # compare cat-file -p
    if diff <($MINIGIT cat-file -p $hash) <(git cat-file -p $hash) ; then
        echo ok $hash
    else
        echo ng $hash
    fi

done



