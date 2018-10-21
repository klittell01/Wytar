##########################
# Makefile
# Author: Kevin Littell
# Date: Oct 20, 2018
# COSC 3750, homework 6
# makes a program using the listed dependencies
##########################

CC=gcc
CFLAGS=-ggdb -Wall
RM=/bin/rm -f

.PHONEY: clean

wytar: wytar.c
	${CC} ${CFLAGS} -o wytar wytar.c -lm

clean:
	${RM} wytar
