# Makefile for minigit
PROGNAME = minigit

$(PROGNAME) : $(PROGNAME).c
	gcc -g -Wall -O0  -lz -lssl -o $(PROGNAME) $(PROGNAME).c

test :  $(PROGNAME)
	prove
clean :
	rm $(PROGNAME)
