# Makefile for minigit
minigit : minigit.c
	gcc -g -Wall -O0  -std=c99 -lz -o minigit minigit.c


