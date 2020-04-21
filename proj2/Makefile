# Makefile
#
# Řešení IJC-DU1
# Datum vytovření: 31.3.2020
# Autor: Tomáš Milostný, xmilos02, FIT VUT
# Překladač: gcc (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0
#            g++ (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0
# Popis: Program make pro přeložení souborů projektu

all: proj2

CC = gcc
CFLAGS = -g -std=c99 -pedantic -Wall -Wextra -Og

proj2: proj2.o
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o proj2

pack:
	zip xmilos02.zip *.c *.h Makefile

run:
	./wordcount-dynamic
	./wordcount