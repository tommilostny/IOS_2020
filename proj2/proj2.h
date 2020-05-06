/* 
 * proj2.h
 * 
 * Řešení IOS - projekt 2
 * Datum vytvoření: 26.4.2020
 * Autor: Tomáš Milostný, xmilos02, FIT VUT
 * Překladač: gcc (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0
 */

#pragma once

//Alokace ve sdílené paměti
#define create_shared_var(type) \
	mmap(NULL, sizeof(type), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0)

//Načte argument programu a vrací jeho celočíselnou hodnotu
int load_arg(char **argv, int argv_index);

//Dealokuje sdílenou paměť, semafory a výstupní soubor
void free_resources();

//Proces soudce
int judge_routine();

//Pomocný proces pro tvorbu přistěhovalců
int immigrants_generator();

//Proces přistěhovalce číslo I
int immigrant_routine(const int I);
