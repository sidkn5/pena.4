/*
Student: Sean Dela Pena
Professor: Dr. Sanjiv Bhatia
Date: 4/4/2021

*/


#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "functionheader.h"

int timeTermination = 100;
char logfile[30];
int noOfProcesses = 18;
int shmid;
int semid;
int goPids[MAX];
int usedProcesses[MAX];
int blockedPids[MAX];
holder* shmPtr;
pid_t* pids;

void help() {
	printf("HELP MENU:  \n\n");

}

int checkTime(int n) {
	if (n > 100) {
		printf("Will default to time 100s.\n");
		return 100;
	}
	else {
		return n;
	}
}

//deallocates and frees everything
void cleanAll() {
	shmdt(shmPtr);
	shmctl(shmid, IPC_RMID, NULL);
	semctl(semid, 0, IPC_RMID, NULL);
	fclose(fp);
	exit(0);
}

void ctrlC() {
	printf("Process terminate. CTRL + C caught.\n");
	cleanAll();
	exit(0);
}

void timesUp() {
	printf("The time given is up Process will terminate.\n");
	cleanAll();
	exit(0);
}

void resetPid(pid_t p) {
	int i;
	for (i = 0; i < noOfProcesses; i++) {
		if (pids[i] == p) {
			pids[i] = 0;
			break;
		}
	}
}

//referred to stackoverflow
//takes care of children who dies
void mySigchldHandler(int sig) {
	pid_t pid;
	while ((pid = waitpid(-1, 0, WNOHANG)) != -1) {
		resetPid(pid);
	}
}

void nextFork() {
	unsigned int nanoSec = shmPtr->forkNanoSec;
	
	shmPtr->forkNanoSec = (rand() % 2) + shmPtr->seconds;
	shmPtr->forkSec = (rand() % 1001) + shmPtr->nanoseconds;

	if (nanoSec >= 1000000000) {
		shmPtr->forkSec += 1;
		shmPtr->forkNanoSec -= 1000000000;
	}
}

int countUsersReady() {
	int count = 0;
	int i;
	for (i = 0; i < noOfProcesses; i++) {
		if (goPids[i] > 0) {
			count++;
		}
	}
	return count;
}

//returns the total number of users ready or blocked
int get_user_count() {
	int total = 0;
	int goCount = 0;
	int blockedCount = 0;
	int i;
	int x;
	for (i = 0; i < noOfProcesses; i++) {
		if (goPids[i] != 0) {
			goCount;
		}
	}

	for (x = 0; x < noOfProcesses; x++) {
		if (blockedPids[x] != 0) {
			blockedCount++;
		}
	}

	total = goCount + blockedCount;

	return total;
}

int main(int argc, char* argv[]) {

	int z;			//holds the lenght of logfile name entered
	char c;			//used for getopt
	int newLogfile = 0;
	int userCount;
	int usersReady;
	key_t shmKey;
	key_t semKey;

	signal(SIGALRM, timesUp);
	signal(SIGINT, ctrlC);
	signal(SIGKILL, cleanAll);

	//referred from stackoverflow
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = mySigchldHandler;
	sigaction(SIGCHLD, &sa, NULL);

	alarm(timeTermination);

	/////////////////////////////////////////////////////////////////
	//GETOPT

	while ((c = getopt(argc, argv, "hs:l:")) != -1) {
		switch (c) {
		case 'h':
			help();
			return 0;
			break;
		case 's':
			timeTermination = checkTime(atoi(optarg));
			printf("Processes will terminate in %d sec(s)\n", timeTermination);
			alarm(timeTermination);
			break;

		case 'l':
			z = strlen(optarg);
			if (z < 30) {
				strcpy(logfile, optarg);
				printf("logfile name: %s\n", logfile);

			}
			else {
				strcpy(logfile, "logfile.txt");
				printf("logfile name: %s\n", logfile);
				newLogfile = 1;
			}
			break;

		default:
			errno = 3;
			perror("Please refer to -h help for proper use of the program.\n");
			exit(0);
		}
	}

	printf("HelloWorldNew \n");

	//defaults to logfile.txt
	if (newLogfile != 1) {
		strcpy(logfile, "logfile.txt");
	}

	fp = fopen(logfile, "a");

	shmKey = ftok("./README.md", 'a');
	shmid = shmget(shmKey, (sizeof(processControlBlock) * MAX) + sizeof(holder), IPC_CREAT | 0666);
	if (shmid < 0) {
		perror("oss: Error: shmget error, creation failure.\n");
		cleanAll();
		exit(1);
	}

	shmPtr = (holder*)shmat(shmid, NULL, 0);
	if (shmPtr == (holder*)-1) {
		perror("oss: Error: shmat error, attachment failure.\n");
		cleanAll();
		exit(1);
	}

	semKey = ftok("./Makefile", 'a');
	semid = semget(semKey, 1, IPC_CREAT | 666);

	if (semid < -1) {
		perror("oss: Error: semget error, creation failure.\n");
	}

	//init semaphore
	semctl(semid, 0, SETVAL, 0);

	//init shared memory
	
	shmPtr->noOfUsers = 0;
	shmPtr->currentPid = 0;
	shmPtr->index = -1;
	shmPtr->nanoseconds = 0;
	shmPtr->seconds = 0;
	
	//initialize the pcb
	int i;
	for (i = 0; i < noOfProcesses; i++) {
		shmPtr->pcb[i].currentIndex = i;
		shmPtr->pcb[i].lastExecTime = 0;
		shmPtr->pcb[i].cpuTimeMs = 0;
		shmPtr->pcb[i].systemTimeMs = 0;
		shmPtr->pcb[i].nanoStart = 0;
		shmPtr->pcb[i].secStart = 0;
		shmPtr->pcb[i].blocked = false;
		shmPtr->pcb[i].terminated = false;
		shmPtr->pcb[i].wait = true;
	}

	//initialize table
	for (i = 0; i < noOfProcesses; i++) {
		goPids[i] = 0;
		usedProcesses[i] = 0;
	}

	nextFork();

	/*while (1) {
		
		usersReady = countUsersReady();
		shmPtr ->noOfUsers = countTotalUsers();
		userCount = shmPtr->noOfUsers
	}*/

	sleep(10);


	cleanAll();
	return 0;


	///////////////////////////////END OF MAIN ////////////////////////////////////////////////////////	
}
