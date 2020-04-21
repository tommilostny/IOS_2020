/* 
 * wordcount.c
 * 
 * Řešení IJC-DU2, příklad 2)
 * Datum vytvoření: 31.3.2020
 * Autor: Tomáš Milostný, xmilos02, FIT VUT
 * Překladač: gcc (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <semaphore.h>

int load_arguments(char **argv, int *output[5])
{
	for (size_t i = 1; i < 6; i++)
	{
		//Načtení argumentů programu
		char *endptr;
		errno = 0;
		int *number = output[i - 1];
		*number = strtol(argv[i], &endptr, 10);

		if (errno != 0 || *endptr != '\0' //Chyba strtol
			|| (i == 1 && *number < 1) //První argument PI >= 1
			|| (i > 1 && (*number < 0 || *number > 2000))) //Ostatní argumenty >=0 && <= 2000
		{
			errno = 1;
			return *number;
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 6)
	{
		fprintf(stderr, "Error: Too few arguments.\n");
		return 1;
	}

	int PI; //počet procesů přistěhovalců; bude postupně vytvořeno PI immigrants (>=1)
	int IG; //max hodnota doby (v milisekundách), po které je generován nový proces immigrant (>= 0, <= 2000)
	int JG; //max hodnota doby (v milisekundách), po které soudce opět vstoupí do budovy (>= 0, <= 2000)
	int IT; //max hodnota doby (v milisekundách), která simuluje trvání vyzvedávání certifikátu přistěhovalcem (>= 0, <= 2000)
	int JT; //max hodnota doby (v milisekundách), která simuluje trvání vydávání rozhodnutí soudcem (>= 0, <= 2000)
	
	int *settings[] = { &PI, &IG, &JG, &IT, &JT };
	int load = load_arguments(argv, settings);

	if (errno == 1)
	{
		fprintf(stderr, "Error: %d: Wrong argument.\n", load);
		return 1;
	}
	
	return 0;
}