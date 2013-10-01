# Makefile for minigit
all : minigit.c
	gcc -g -Wall -O0  -std=c99 -lz -o minigit minigit.c

test :
	prove
clean :
	rm minigit
