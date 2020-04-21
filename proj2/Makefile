# Makefile
#
# Řešení IOS - projekt 2
# Datum vytovření: 21.4.2020
# Autor: Tomáš Milostný, xmilos02, FIT VUT
# Překladač: gcc (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0

all: proj2

CC = gcc
CFLAGS = -g -std=gnu99 -pedantic -Wall -Wextra -Werror

proj2: proj2.o
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o proj2

pack:
	zip xmilos02.zip *.c *.h Makefile

run:
	./wordcount-dynamic
	./wordcount