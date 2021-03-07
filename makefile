# Makefile for building
#    xmod
#

all: xmod log

xmod: xmod.c log
	gcc -o xmod.o -Wall -Werror xmod.c 

log: log.c
	gcc -o log.o -Wall -Werror log.c

clean:
	rm -f xmod.o log.o

