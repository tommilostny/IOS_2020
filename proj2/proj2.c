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
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <stdbool.h>

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
	srand(time(NULL));

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

	sem_t *semaphore = mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	sem_init(semaphore, 1, 1);

	bool *judge_in_building = mmap(NULL, sizeof(bool), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	*judge_in_building = false;

	bool *certificate_approved = mmap(NULL, sizeof(bool), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	*certificate_approved = false;

	pid_t judge = fork();
	if (judge == 0) //proces soudce
	{
		int imms_judged = 0; //počet souzených přistěhovalců
		while (imms_judged < *PI)
		{
			//náhodná doba čekání před vstupem do budovy
			usleep((rand() % *JG) * 1000);

			//vstup do budovy
			sem_wait(semaphore);
			printf("%lu:\tJUDGE\t: wants to enter.\n", ++(*A));
			*judge_in_building = true;
			*certificate_approved = false;
			printf("%lu:\tJUDGE\t: enters:\t%lu :\t%lu :\t%lu .\n", ++(*A), *NE, *NC, *NB);
			sem_post(semaphore);

			//vydání rozhodnutí
			if (*NE != *NC) //pokud nejsou všichni přistěhovalci v budově registrovaní
			{
				sem_wait(semaphore);
				printf("%lu:\tJUDGE\t: waits for imm:\t%lu :\t%lu :\t%lu .\n", ++(*A), *NE, *NC, *NB);
				sem_post(semaphore);
				while (*NE != *NC);
			}
			sem_wait(semaphore);
			printf("%lu:\tJUDGE\t: starts confirmation:\t%lu :\t%lu :\t%lu .\n", ++(*A), *NE, *NC, *NB);

			//náhodná doba vydávání certifikátu
			usleep((rand() % *JT) * 1000);
			*certificate_approved = true;

			*NE = *NC = 0;
			printf("%lu:\tJUDGE\t: ends confirmation:\t%lu :\t%lu :\t%lu .\n", ++(*A), *NE, *NC, *NB);
			sem_post(semaphore);

			//náhodná doba čekání před odchodem z budovy
			usleep((rand() % *JT) * 1000);

			//odchod z budovy
			sem_wait(semaphore);
			imms_judged += *NB;
			*judge_in_building = false;
			printf("%lu:\tJUDGE\t: leaves:\t%lu :\t%lu :\t%lu .\n", ++(*A), *NE, *NC, *NB);
			sem_post(semaphore);
		}
		sem_wait(semaphore);
		printf("%lu:\tJUDGE\t: finishes.\n", ++(*A));
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
			int I = 0;
			int *imms_queue_length = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
			*imms_queue_length = 0;

			while (I < *PI)
			{
				//náhodná doba čekání před generováním přistěhovalce
				usleep((rand() % *IG) * 1000);

				I++;
				pid_t immigrant = fork();
				if (immigrant == 0) //proces přistěhovalce
				{
					sem_wait(semaphore);
					printf("%lu:\tIMM %d\t: starts.\n", ++(*A), I);
					sem_post(semaphore);

					//čeká než soudce odejde z budovy
					while (*judge_in_building);

					sem_wait(semaphore);
					printf("%lu:\tIMM %d\t: enters:\t%lu :\t%lu :\t%lu .\n", ++(*A), I, ++(*NE), *NC, ++(*NB));
					int queue_order = *NE;
					sem_post(semaphore);

					//čekání v registrační frontě
					while (queue_order > *imms_queue_length);

					//registrace
					sem_wait(semaphore);
					printf("%lu:\tIMM %d\t: checks:\t%lu :\t%lu :\t%lu .\n", ++(*A), I, *NE, ++(*NC), *NB);
					(*imms_queue_length)--;
					sem_post(semaphore);

					//čekání na schválení certifikátu soudcem
					while (!(*certificate_approved));

					sem_wait(semaphore);
					printf("%lu:\tIMM %d\t: wants certificate:\t%lu :\t%lu :\t%lu .\n", ++(*A), I, *NE, *NC, *NB);
					sem_post(semaphore);

					usleep((rand() % *JT) * 1000);

					sem_wait(semaphore);
					printf("%lu:\tIMM %d\t: got certificate:\t%lu :\t%lu :\t%lu .\n", ++(*A), I, *NE, *NC, *NB);
					sem_post(semaphore);

					//odchod z budovy
					while (*judge_in_building);
					sem_wait(semaphore);
					printf("%lu:\tIMM %d\t: leaves:\t%lu :\t%lu :\t%lu .\n", ++(*A), I, *NE, *NC, --(*NB));
					sem_post(semaphore);

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