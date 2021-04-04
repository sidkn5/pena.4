
/*
Student: Sean Dela Pena

Professor: Dr. Sanjiv Bhatia

Assignment 4: Process Scheduling, OS simulator that simulates
the process scheduling, more specifically round robin algorithm

Date: 4/4/2021

Please note that the repo might not be accurate, Please read README
github: github.com/sidkn5

*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <errno.h>
#include <time.h>
#include "functionheader.h"
extern errno;
#define IO 1
#define CPU 2


int shmid;
int semid;
int userInd;
int userPid;
int seconds;
char logfile[30];

FILE* fp;
holder* shmPtr;

//handle the clean up
void handler(){
	shmdt(shmPtr);
	
}

//semaphore signal function, increment sem
void sem_signal() {
	printf("USER: Signaling that done\n");
	struct sembuf sem;
	sem.sem_op = 1;
	sem.sem_num = 0;
	sem.sem_flg = 0;
	semop(semid, &sem, 1);
}

void clockCheck() {
	unsigned int nanoCheck = shmPtr->nanoseconds;
	if (nanoCheck >= 1000000000) {
		shmPtr->seconds += 1;
		shmPtr->nanoseconds -= 1000000000;
	}
}

//convert sec to ms
float convertSec() {
	int converted;
	converted = (float)shmPtr->pcb[userInd].secStart * 1000;
	return converted;
}

//convert ns to ms
float convertNano() {
	int converted;
	converted = (float)shmPtr->pcb[userInd].nanoStart / 1000000;
	return converted;
}

//Main Driver
int main(int argc, char* argv[]) {

	srand(time(NULL));
	int num;
	unsigned int times;
	unsigned int burstTime;
	float timeInMs;				//holds conversion
	float timeInNano;			//holds conversion
	int randomNum;
	randomNum = rand() % 10000;
	bool done = false;
	key_t shmKey = ftok("./README.md", 'a');
	key_t semKey = ftok("./Makefile", 'a');
	signal(SIGKILL, handler);
	signal(SIGINT, handler);
	signal(SIGTERM, handler);
	signal(SIGSEGV, handler);
	
	
	//sprintf(logfile, "%d.log", randomNum);
	//fp = fopen(logfile, "a");


	//attach to shared memory
	shmid = shmget(shmKey, (sizeof(processControlBlock)) + sizeof(holder), IPC_EXCL);
	if (shmid < 0) {
		perror("user: Error: shmget error, creation failure.\n");
		handler();
		exit(1);
	}

	shmPtr = (holder*)shmat(shmid, NULL, 0);
	if (shmPtr == (holder*)-1) {
		perror("user: Error: shmget error, creation failure.\n");
		handler();
		exit(1);
	}

	//attach semaphores
	
	semid = semget(semKey, 1, IPC_EXCL);
	if (semid < -1) {
		perror("user: Error: semget error, creation failure.\n");
	}

	userPid = getpid();


	while (1) {
		//TO DO
		while (shmPtr->currentPid != userPid);
		shmPtr->currentPid = 0;
		userInd = shmPtr->index;
		shmPtr->pcb[userInd].blocked = false;

		if (shmPtr->pcb[userInd].nanoStart == 0 && shmPtr->pcb[userInd].secStart == 0) {
			srand(userPid);
			num = rand() % (QUANTUM + 1);

			if (num % 2 == 0) {
				times = rand() % 11;
				burstTime = rand() % ((10000000 + 1) - 1) + 1;
				//printf("PID: %d is terminating early after working for %d ms\n", user_pid, time);
				//fprintf(file_ptr, "Process is terminating early\n");
				shmPtr->pcb[userInd].terminated = true;
				shmPtr->pcb[userInd].lastExecTime = burstTime;
				shmPtr->pcb[userInd].cpuTimeMs += times;
				shmPtr->pcb[userInd].systemTimeMs += times;
				sem_signal(shmid);
				handler();
				exit(0);
			}

			//start the time here
			shmPtr->pcb[userInd].nanoStart = shmPtr->nanoseconds;
			shmPtr->pcb[userInd].secStart = shmPtr->seconds;
			//determine the type of process whether they are an IO or CPU process
			num = rand() % (50 + 1);
			if (num % 2 == 0) {
				shmPtr->pcb[userInd].type = CPU;
			}
			else {
				shmPtr->pcb[userInd].type = IO;
			}
		
		}

		//check if there is an interrupt
		if (shmPtr->pcb[userInd].type == 1) {
			//can be interrupted
			num = rand() % 13;
			if (num % 4 == 0) {
				//fprintf(file_ptr, "Process is being blocked\n");
				shmPtr->pcb[userInd].blocked = true;
				shmPtr->pcb[userInd].blockedSeconds = shmPtr->seconds + (rand() % ((5 + 1) - 1) + 1);
				shmPtr->pcb[userInd].blockedNanoseconds = shmPtr->nanoseconds + (rand() % 1000 + 1 -1 + 1);
				if (shmPtr->pcb[userInd].blockedNanoseconds > 1000000000) {
					shmPtr->pcb[userInd].blockedNanoseconds -= 1000000000;
					shmPtr->pcb[userInd].blockedSeconds += 1;
				}

			}
			else {
				//fprintf(file_ptr, "Process is not being blocked\n");
				done = true;
			}
		}
		else {
			done = true;
		}

		burstTime = rand() % ((10000000 + 1) - 1) + 1;
		shmPtr->pcb[userInd].lastExecTime = burstTime;
		shmPtr->pcb[userInd].cpuTimeMs += (float)burstTime / 1000000;

		int timeInSec;
		int convertedSec;
		int convertedNano;

		if (done) {
			timeInSec = shmPtr->seconds - shmPtr->pcb[userInd].secStart;
			shmPtr->pcb[userInd].nanoStart += shmPtr->nanoseconds;
			clockCheck();
			shmPtr->pcb[userInd].nanoStart += burstTime;
			clockCheck();
			shmPtr->pcb[userInd].secStart = timeInSec;
			convertedNano = convertNano();
			convertedSec = convertSec();
			shmPtr->pcb[userInd].systemTimeMs = convertedNano + convertedSec;
			sem_signal(shmid);
			break;

		}

		//signal done
		sem_signal(shmid);

	}
	
	//Clean up
	handler();

	return 0;


	//////////END OF MAIN//////////////
}
