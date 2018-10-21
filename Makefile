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

wytar: main.o tar_utils.o
	${CC} ${CFLAGS} -o wytar wytar.c tar_utils.o -lm

tar_utils.o: tar_utils.c
	${CC} ${CFLAGS} -c tar_utils.c

main.o: wytar.c
	${CC} ${CFLAGS} -c tar_utils.c


clean:
	${RM} wytar tar_utils.o
