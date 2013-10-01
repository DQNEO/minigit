# Makefile for minigit
PROGNAME = minigit

all : $(PROGNAME).c
	gcc -g -Wall -O0  -std=c99 -lz -o $(PROGNAME) $(PROGNAME).c

test :
	prove
clean :
	rm $(PROGNAME)
