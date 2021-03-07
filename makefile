# Makefile for building
#    xmod
#

all: xmod log

xmod: xmod.o log.o
	gcc xmod.o log.o -o exec

xmod.o: xmod.c
	gcc -c xmod.c -Wall -Werror 

log.o: log.c
	gcc -c log.c -Wall -Werror

clean:
	rm -f xmod.o log.o

