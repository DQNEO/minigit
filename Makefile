# Makefile for minigit
minigit : minigit.c
	gcc -g -std=c99 -lz -o minigit minigit.c


