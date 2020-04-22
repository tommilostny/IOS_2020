/* 
 * proj2.c
 * 
 * Řešení IOS - projekt 2
 * Datum vytvoření: 21.4.2020
 * Autor: Tomáš Milostný, xmilos02, FIT VUT
 * Překladač: gcc (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>

int *create_shared_int(char **argv, int argv_index)
{
	int *number = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	errno = 0;
	*number = strtol(argv[argv_index], NULL, 10);
	return number;
}

int main(int argc, char **argv)
{
	if (argc != 6)
	{
		fprintf(stderr, "Error: Wrong number of arguments.\nRun program as \"./proj2 PI IG JG IT JT\".\n");
		return 1;
	}

	//počet procesů přistěhovalců; bude postupně vytvořeno PI immigrants (>=1)
	int *PI = create_shared_int(argv, 1);
	if (errno != 0 || *PI < 1)
	{
		fprintf(stderr, "Error: %d:\tWrong argument (PI must be >= 1).\n", *PI);
		return 1;
	}

	//max hodnota doby (v milisekundách), po které je generován nový proces immigrant (>= 0, <= 2000)
	int *IG = create_shared_int(argv, 2);
	if (errno != 0 || *IG < 0 || *IG > 2000)
	{
		fprintf(stderr, "Error: %d:\tWrong argument (IG must be >= 0 and <= 2000).\n", *IG);
		return 1;
	}

	//max hodnota doby (v milisekundách), po které soudce opět vstoupí do budovy (>= 0, <= 2000)
	int *JG = create_shared_int(argv, 3);
	if (errno != 0 || *JG < 0 || *JG > 2000)
	{
		fprintf(stderr, "Error: %d:\tWrong argument (JG must be >= 0 and <= 2000).\n", *JG);
		return 1;
	}

	//max hodnota doby (v milisekundách), která simuluje trvání vyzvedávání certifikátu přistěhovalcem (>= 0, <= 2000)
	int *IT = create_shared_int(argv, 4);
	if (errno != 0 || *IT < 0 || *IT > 2000)
	{
		fprintf(stderr, "Error: %d:\tWrong argument (IT must be >= 0 and <= 2000).\n", *IT);
		return 1;
	}

	//max hodnota doby (v milisekundách), která simuluje trvání vydávání rozhodnutí soudcem (>= 0, <= 2000)
	int *JT = create_shared_int(argv, 5);
	if (errno != 0 || *JT < 0 || *JT > 2000)
	{
		fprintf(stderr, "Error: %d:\tWrong argument (JT must be >= 0 and <= 2000).\n", *JT);
		return 1;
	}

	//pořadové číslo prováděné akce
	size_t *A = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

	//aktuální počet přistěhovalců, kteří vstoupili do budovy a dosud o nich nebylo rozhodnuto
	size_t *NE = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

	//aktuální počet přistěhovalců, kteří se zaregistrovali a dosud o nich nebylo rozhodnuto
	size_t *NC = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

	//počet přistěhovalců, kteří jsou v budově
	size_t *NB = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	*A = *NE = *NC = *NB = 0;

	pid_t judge = fork();
	if (judge == 0) //proces soudce
	{
		usleep(*JG * 1000); //random
		//do souboru, semafor
		printf("%lu:\tJUDGE: wants to enter.\n", ++(*A));
		printf("%lu:\tJUDGE: enters:\t%lu :\t%lu :\t%lu .\n", ++(*A), *NE, *NC, *NB);
		return 0;
	}
	else if (judge == -1)
	{
		fprintf(stderr, "Error creating judge process.\n");
		return 1;
	}
	else
	{
		pid_t immigrants = fork();
		if (immigrants == 0) //pomocný proces pro tvorbu přistěhovalců
		{
			int I = 0;
			while (I <= *PI)
			{
				I++;
				pid_t immigrant = fork();
				if (immigrant == 0) //proces přistěhovalce
				{
					printf("%lu:\tIMM %d: starts.\n", ++(*A), I);
					return 0;
				}
				else if (immigrant == -1)
				{
					fprintf(stderr, "Error creating immigrant process #%d.\n", I);
					return 1;
				}
			}
			while (I > 0)
			{
				wait(NULL);
				I--;
			}
			return 0;
		}
		else if (immigrants == -1)
		{
			fprintf(stderr, "Error creating immigrants producing process.\n");
			return 1;
		}
		else //hlavní proces, čeká na soudce a přistěhovalce
		{
			int judge_status;
			waitpid(judge, &judge_status, 0);

			int immigrants_status;
			waitpid(immigrants, &immigrants_status, 0);

			printf("jsem rodič, variables %d, %d, %d, %d, %d\n", *PI, *IG, *JG, *IT, *JT);
		}
	}
	
	return 0;
}