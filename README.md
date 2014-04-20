# minigit - yet another git implementation

## INSTALLATION
```sh
git clone git://github.com/DQNEO/minigit.git
cd minigit
make
```

## Usage

### cat-file

```
# same as git
minigit cat-file -t .git/objects/00/0298c2ba808350e498c4b4149b02b42c47cecd
minigit cat-file -s .git/objects/00/0298c2ba808350e498c4b4149b02b42c47cecd
minigit cat-file -p .git/objects/00/0298c2ba808350e498c4b4149b02b42c47cecd

# you can pass any loose object file
minigit cat-file -t 000298
minigit cat-file -s 000298
minigit cat-file -p 000298
```

objectfile can be a commit, tree , or blob.

## log

```
minigit log
```

## ls-files
```
minigit ls-files
minigit ls-files --stage
```

## Run test

```
make test
or
prove -r t
```


## How to get many loose object files

you can get loose objects as many as you like by `git unpack-objects` :)

http://qiita.com/DQNEO/items/b6cb03c60b510dd3c934

