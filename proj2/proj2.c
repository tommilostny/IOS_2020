/* 
 * proj2.c
 * 
 * Řešení IOS - projekt 2
 * Datum vytvoření: 21.4.2020
 * Autor: Tomáš Milostný, xmilos02, FIT VUT
 * Překladač: gcc (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0
 */

#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>

#include "proj2.h"

int *load_arg(char **argv, int argv_index)
{
	int *number = create_shared_var(int);
	errno = 0;
	*number = strtol(argv[argv_index], NULL, 10);
	return number;
}

void fprintf_flush(FILE *file, char *fmt, ...)
{
	va_list parameters;
	va_start(parameters, fmt);

	vfprintf(file, fmt, parameters);
	fflush(file);

	va_end(parameters);
}

int main(int argc, char **argv)
{
	if (argc != 6)
	{
		fprintf(stderr, "Error: Wrong number of arguments.\nRun program as \"./proj2 PI IG JG IT JT\".\n");
		return 1;
	}
	srand(time(NULL));

	//počet procesů přistěhovalců; bude postupně vytvořeno PI immigrants (>=1)
	int *PI = load_arg(argv, 1);
	if (errno != 0 || *PI < 1)
	{
		fprintf(stderr, "Error: %d:\tWrong argument (PI must be >= 1).\n", *PI);
		return 1;
	}

	//max hodnota doby (v milisekundách), po které je generován nový proces immigrant (>= 0, <= 2000)
	int *IG = load_arg(argv, 2);
	if (errno != 0 || *IG < 0 || *IG > 2000)
	{
		fprintf(stderr, "Error: %d:\tWrong argument (IG must be >= 0 and <= 2000).\n", *IG);
		return 1;
	}

	//max hodnota doby (v milisekundách), po které soudce opět vstoupí do budovy (>= 0, <= 2000)
	int *JG = load_arg(argv, 3);
	if (errno != 0 || *JG < 0 || *JG > 2000)
	{
		fprintf(stderr, "Error: %d:\tWrong argument (JG must be >= 0 and <= 2000).\n", *JG);
		return 1;
	}

	//max hodnota doby (v milisekundách), která simuluje trvání vyzvedávání certifikátu přistěhovalcem (>= 0, <= 2000)
	int *IT = load_arg(argv, 4);
	if (errno != 0 || *IT < 0 || *IT > 2000)
	{
		fprintf(stderr, "Error: %d:\tWrong argument (IT must be >= 0 and <= 2000).\n", *IT);
		return 1;
	}

	//max hodnota doby (v milisekundách), která simuluje trvání vydávání rozhodnutí soudcem (>= 0, <= 2000)
	int *JT = load_arg(argv, 5);
	if (errno != 0 || *JT < 0 || *JT > 2000)
	{
		fprintf(stderr, "Error: %d:\tWrong argument (JT must be >= 0 and <= 2000).\n", *JT);
		return 1;
	}

	//pořadové číslo prováděné akce
	size_t *A = create_shared_var(size_t);

	//aktuální počet přistěhovalců, kteří vstoupili do budovy a dosud o nich nebylo rozhodnuto
	size_t *NE = create_shared_var(size_t);

	//aktuální počet přistěhovalců, kteří se zaregistrovali a dosud o nich nebylo rozhodnuto
	size_t *NC = create_shared_var(size_t);

	//počet přistěhovalců, kteří jsou v budově
	size_t *NB = create_shared_var(size_t);
	*A = *NE = *NC = *NB = 0;

	sem_t *semaphore = create_shared_var(sem_t);
	sem_init(semaphore, 1, 1);

	bool *judge_in_building = create_shared_var(bool);
	*judge_in_building = false;

	bool *certificate_approved = create_shared_var(bool);
	*certificate_approved = false;

	FILE *output;
	if ((output = fopen("proj2.out", "w")) == NULL)
	{
		fprintf(stderr, "Error creating output file.\n");
		return 1;
	}

	pid_t judge = fork();
	if (judge == 0) //proces soudce
	{
		int imms_judged = 0; //počet souzených přistěhovalců
		while (imms_judged < *PI)
		{
			//náhodná doba čekání před vstupem do budovy
			if (*JG > 0)
				usleep((rand() % *JG) * 1000);

			//vstup do budovy
			sem_wait(semaphore);
			fprintf_flush(output, "%lu:\tJUDGE\t: wants to enter.\n", ++(*A));
			*judge_in_building = true;
			fprintf_flush(output, "%lu:\tJUDGE\t: enters:\t\t%lu :\t%lu :\t%lu\n", ++(*A), *NE, *NC, *NB);
			sem_post(semaphore);

			//vydání rozhodnutí, pokud je někdo v budově
			if (*NB > 0)
			{
				if (*NE != *NC) //pokud nejsou všichni přistěhovalci v budově registrovaní
				{
					sem_wait(semaphore);
					fprintf_flush(output, "%lu:\tJUDGE\t: waits for imm:\t%lu :\t%lu :\t%lu\n", ++(*A), *NE, *NC, *NB);
					sem_post(semaphore);
					while (*NE != *NC);
				}
				sem_wait(semaphore);
				fprintf_flush(output, "%lu:\tJUDGE\t: starts confirmation:\t%lu :\t%lu :\t%lu\n", ++(*A), *NE, *NC, *NB);

				//náhodná doba vydávání certifikátu
				if (*JT > 0)
					usleep((rand() % *JT) * 1000);
				
				imms_judged += *NC;
				*certificate_approved = true;
				*NE = *NC = 0;
				fprintf_flush(output, "%lu:\tJUDGE\t: ends confirmation:\t%lu :\t%lu :\t%lu\n", ++(*A), *NE, *NC, *NB);
				sem_post(semaphore);
			}
			//náhodná doba čekání před odchodem z budovy
			if (*JT > 0)
				usleep((rand() % *JT) * 1000);

			//odchod z budovy
			sem_wait(semaphore);
			*judge_in_building = false;
			*certificate_approved = false;
			fprintf_flush(output, "%lu:\tJUDGE\t: leaves:\t\t%lu :\t%lu :\t%lu\n", ++(*A), *NE, *NC, *NB);
			sem_post(semaphore);
		}
		sem_wait(semaphore);
		fprintf_flush(output, "%lu:\tJUDGE\t: finishes.\n", ++(*A));
		sem_post(semaphore);
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
			int I;
			for (I = 1; I <= *PI; I++)
			{
				//náhodná doba čekání před generováním přistěhovalce
				if (*IG > 0)
					usleep((rand() % *IG) * 1000);

				pid_t immigrant = fork();
				if (immigrant == 0) //proces přistěhovalce
				{
					sem_wait(semaphore);
					fprintf_flush(output, "%lu:\tIMM %d\t: starts.\n", ++(*A), I);
					sem_post(semaphore);

					//čeká než soudce odejde z budovy
					while (*judge_in_building);

					//vstup do budovy
					sem_wait(semaphore);
					fprintf_flush(output, "%lu:\tIMM %d\t: enters:\t\t%lu :\t%lu :\t%lu\n", ++(*A), I, ++(*NE), *NC, ++(*NB));
					sem_post(semaphore);

					//registrace
					sem_wait(semaphore);
					fprintf_flush(output, "%lu:\tIMM %d\t: checks:\t\t%lu :\t%lu :\t%lu\n", ++(*A), I, *NE, ++(*NC), *NB);
					sem_post(semaphore);

					//čekání na schválení certifikátu soudcem
					while (!(*certificate_approved));

					sem_wait(semaphore);
					fprintf_flush(output, "%lu:\tIMM %d\t: wants certificate:\t%lu :\t%lu :\t%lu\n", ++(*A), I, *NE, *NC, *NB);
					sem_post(semaphore);

					if (*IT > 0)
						usleep((rand() % *IT) * 1000);

					sem_wait(semaphore);
					fprintf_flush(output, "%lu:\tIMM %d\t: got certificate:\t%lu :\t%lu :\t%lu\n", ++(*A), I, *NE, *NC, *NB);
					sem_post(semaphore);

					//odchod z budovy, čekání než odejde soudce
					while (*judge_in_building);
					sem_wait(semaphore);
					fprintf_flush(output, "%lu:\tIMM %d\t: leaves:\t\t%lu :\t%lu :\t%lu\n", ++(*A), I, *NE, *NC, --(*NB));
					sem_post(semaphore);

					return 0;
				}
				else if (immigrant == -1)
				{
					fprintf(stderr, "Error creating immigrant process #%d.\n", I);
					return 1;
				}
			}
			while (I > 0) //čekání na skončení procesů přistěhovalců
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
			waitpid(judge, NULL, 0);
			waitpid(immigrants, NULL, 0);
		}
	}
	fclose(output);
	munmap(PI, sizeof(int));
	munmap(IG, sizeof(int));
	munmap(JG, sizeof(int));
	munmap(IT, sizeof(int));
	munmap(JT, sizeof(int));
	munmap(A, sizeof(size_t));
	munmap(NE, sizeof(size_t));
	munmap(NC, sizeof(size_t));
	munmap(NB, sizeof(size_t));
	munmap(semaphore, sizeof(sem_t));
	munmap(judge_in_building, sizeof(bool));
	munmap(certificate_approved, sizeof(bool));
	return 0;
}
