zlib: zlib.c
	gcc -g -Wall -O0 -std=c99 -lz -o zlib zlib.c

clean:
	rm zlib

test: clean zlib
	./zlib -c txts/helloworld.txt gen/helloworld && diff gen/helloworld compressed/helloworld && echo ok
	./zlib -c txts/bocchan.txt gen/bocchan && diff gen/bocchan compressed/bocchan && echo ok

git_cat_file-p: git_cat_file-p.c
	gcc -g -Wall -O0 -std=c99 -lz -o git_cat_file-p  git_cat_file-p.c
