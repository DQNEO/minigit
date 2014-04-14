# Makefile for minigit
PROGNAME = minigit

$(PROGNAME) : $(PROGNAME).c zlib.c
	gcc -g3 -Wall -O0  -lz -lssl -o $(PROGNAME) $(PROGNAME).c zlib.c

test :  $(PROGNAME)
	prove
clean :
	rm $(PROGNAME)
