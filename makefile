# Makefile for building
#    xmod
#

all: xmod

xmod: xmod.c
	gcc -o xmod.o -Wall -Werror xmod.c 

clean:
	rm -f xmod.o

