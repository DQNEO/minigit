# minigit - yet another git implementation

## INSTALL

make

## How to use
```
minigit cat-file -s objectfile
minigit cat-file -t objectfile
minigit cat-file -p objectfile
```

objectfile can be a commit, tree , or blob.

## Run test

```
make test
or
prove -r t
```

## DONE
* enable to take sha1 argument
* ls-tree

## TODO
* bug in cat-file -p (for tree object)
* show
* log -1
