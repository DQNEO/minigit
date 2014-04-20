# Makefile for minigit
PROGNAME = minigit
SUBCOMMANDS=cmd/log.c cmd/cat_file.c cmd/commit.c cmd/hash_object.c cmd/init.c cmd/rev_parse.c ls_files/cmd_ls_files.c
$(PROGNAME) : $(PROGNAME).h $(PROGNAME).c zlib.c sha1.c time.c $(SUBCOMMANDS)
	gcc -g3 -Wall -O0  -lz -lssl -o $(PROGNAME) $(PROGNAME).c zlib.c sha1.c time.c $(SUBCOMMANDS)

test :  $(PROGNAME)
	prove
clean :
	rm $(PROGNAME)
