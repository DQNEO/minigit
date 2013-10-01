# Makefile for minigit
PROGNAME = minigit

$(PROGNAME) : $(PROGNAME).c
	gcc -g -Wall -O0  -std=c99 -lz -o $(PROGNAME) $(PROGNAME).c

test :  $(PROGNAME)
	prove
clean :
	rm $(PROGNAME)
