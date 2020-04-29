/* 
 * proj2.h
 * 
 * Řešení IOS - projekt 2
 * Datum vytvoření: 26.4.2020
 * Autor: Tomáš Milostný, xmilos02, FIT VUT
 * Překladač: gcc (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0
 */

#pragma once

#include <stdio.h>

//Alokace ve sdílené paměti
#define create_shared_var(type) \
	mmap(NULL, sizeof(type), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0)

//Načte argument programu a vrací jeho celočíselnou hodnotu v nově alokované sdílené paměti
int *load_arg(char **argv, int argv_index);

//Tisk do souboru (jako fprintf), zajištění okamžitého zápisu do souboru pomocí fflush()
void write_step(FILE *file, char *fmt, ...);
