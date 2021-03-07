# Makefile for building
#    xmod
#

all: project

project: xmod.o log.o time_ctrl.o 
	gcc xmod.o log.o time_ctrl.o -o exec

xmod.o: xmod.c log.c time_ctrl.c
	gcc -c xmod.c -Wall -Werror 

log.o: log.c time_ctrl.c
	gcc -c log.c -Wall -Werror

time_ctrl.o: time_ctrl.c
	gcc -c time_ctrl.c -Wall -Werror

clean:
	rm -f *.o

