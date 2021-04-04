/*
Student: Sean Dela Pena

Professor: Dr. Sanjiv Bhatia

Assignment 4: Process Scheduling, OS simulator that simulates
the process scheduling, more specifically round robin algorithm

Date: 4/4/2021

Please note that the repo might not be accurate, Please read README
github: github.com/sidkn5

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
#include <time.h>
#include "functionheader.h"

int timeTermination = 100;		//default time termination in secs
char logfile[30];				//logfile name, can't exceed 30 chars
int noOfProcesses = 18;			//max no. of processes allowed
int shmid;
int semid;
int goPids[MAX];				//ready pids
int usedProcesses[MAX];
int blockedQ[MAX];				//blocked queue pids
int readyIn = 0;
int readyOut = 0;
int blockedIn = 0;
int blockedOut = 0;
char addToLogBuffer[300];		//a buffer to log messages
int lineLimit;					//line limit of log file
FILE* fp;	
holder* shmPtr;
pid_t* pids;


//displays help menu
void help() {
	printf("HELP MENU:  \n\n");
	printf("USAGE: \t\t./oss [-h] [-s time] [-l logfile] \n\n");
	printf("This Program is an operating system simulation. \n\n");
	printf("It simulates a round robin scheduling algorithm in an operating system.  \n\n");
	printf("-----------------------------OPTIONS:--------------------  \n\n");
	printf("[-h]		\t\t -displays the help menu. \n\n");
	printf("[-s time]	\t\t -this option allows the user to specify a time in secs to when the program\n\n");
	printf("			\t\t will terminate no matter what. \n\n");
	printf("[-l logfile]\t\t -this option allows the user to specify a logfile name with a maximum  \n\n");
	printf("			\t\t of 30 chars. Please note that end the filename with .txt so it can be cleaned. \n\n");

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

//ensures that pids are killed
void killChildPids() {
	int i;
	int x;
	for (i = 0; i < MAX; i++) {
		if (goPids[i] != 0) {
			kill(goPids[i], SIGKILL);
		}
	}

	for (x = 0; x < MAX; x++) {
		if (blockedQ[x] != 0) {
			kill(blockedQ[x], SIGKILL);
		}
	}
}

//deallocates and frees everything
void cleanAll() {
	killChildPids();
	shmdt(shmPtr);
	shmctl(shmid, IPC_RMID, NULL);
	semctl(semid, 0, IPC_RMID, NULL);
	
	exit(0);
	
}

//called for ctrl+C termination
void ctrlC() {
	printf("Process terminate. CTRL + C caught.\n");
	cleanAll();
	exit(0);
}

//called for early time termination
void timesUp() {
	printf("The time given is up Process will terminate.\n");
	cleanAll();
	exit(0);
}

//logging function to the logfile, maximum 10 000 lines
void logging(char *buffer) {
	fputs(buffer, fp);
	lineLimit++;
	if (lineLimit >= 10000) {
		printf("Logfile is at maximum capacity of 10000 lines\n\n");
		cleanAll();
		exit(0);
	}
}

//
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
	while ((pid = waitpid((pid_t)(-1), 0, WNOHANG)) > 0) {
		//resetPid(pid);
	}
}

void forkCheck() {
	unsigned int nanoCheck = shmPtr->forkNanoSec;
	if (nanoCheck >= 1000000000) {
		shmPtr->forkSec += 1;
		shmPtr->forkNanoSec -= 1000000000;
	}
}

//
void nextFork() {
	//unsigned int nanoSec = shmPtr->forkNanoSec;
	
	shmPtr->forkSec = (rand() % 2) + shmPtr->seconds;
	shmPtr->forkNanoSec = (rand() % 1001) + shmPtr->nanoseconds;

	forkCheck();
	/*if (nanoSec >= 1000000000) {
		shmPtr->forkSec += 1;
		shmPtr->forkNanoSec -= 1000000000;
	}*/
}

//counts all the user processes
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

//init
void initializePcb(int i) {
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

//returns the total number of users ready or blocked
int countTotalUsers() {
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
		if (blockedQ[x] != 0) {
			blockedCount++;
		}
	}

	total = goCount + blockedCount;

	return total;
}

//searches the arrray for a free index
int freeIndex() {
	int i;
	for (i = 0; i < 19; i++) {
		if (usedProcesses[i] == 0) {
			return i;
		}
	}
}

//forking of the user process
void forkChild(int index) {

	printf("forking\n");
	int pid;

	if ((pid = fork()) == -1) {
		perror("oss.c: Failed forking child...");
		cleanAll();
		exit(1);
	}
	else {
		if (pid != 0) {
			shmPtr->pcb[index].currentPid = pid;
			usedProcesses[index] = 1;
			initializePcb(index);
			
			goPids[readyIn] = shmPtr->pcb[index].currentPid;
			sprintf(addToLogBuffer, "OSS: PID: %d is in the ready queue\n", shmPtr->pcb[index].currentPid);
			logging(addToLogBuffer);
			readyIn = (readyIn + 1) % MAX;
			//printf("ready in incremented\n");
		}
		else {
			execl("./user", "./user", (char*)0);
		}
	}
}

//dispatch the next process
void releaseNextProcesses() {
	int index = freeIndex();
	//printf("forking child in index: %d\n", index);
	forkChild(index);
}


//gets the index of the pid
int getIndex(int pid) {
	int i;
	for (i = 0; i < MAX; i++) {
		if (shmPtr->pcb[i].currentPid == pid) {
			return shmPtr->pcb[i].currentIndex;
		}
	}
	return -1;
}

//checks the blocked processes and see if they can be moved to the ready queue
void checkBlockedProcesses() {
	int pid = blockedQ[blockedOut];
	int i = getIndex(pid);

	if (pid != 0) {
		if (shmPtr->pcb[i].blockedSeconds == shmPtr->seconds) {
			if (shmPtr->pcb[i].blockedNanoseconds <= shmPtr->nanoseconds) {
				blockedQ[blockedOut] = 0;
				blockedOut = (blockedOut + 1) % MAX;
				goPids[readyIn] = pid;
				readyIn = (readyIn + 1) % MAX;
			}
		}
		else if (shmPtr->forkSec < shmPtr->seconds) {
			blockedQ[blockedOut] = 0;
			blockedOut = (blockedOut + 1) % MAX;
			goPids[readyIn] = pid;
			readyIn = (readyIn + 1) % MAX;
		}
	}

}

//sem_wait function for the semaphore
void sem_wait(int n) {
	struct sembuf semaphore;
	semaphore.sem_op = -1;
	semaphore.sem_num = 0;
	semaphore.sem_flg = 0;
	semop(semid, &semaphore, 1);
}

int getRandomTime() {
	int x;
	x = rand() % ((10000 + 1) - 100) + 100;
	return x;
}

void clockCheck() {
	unsigned int nanoCheck = shmPtr->nanoseconds;
	if (nanoCheck >= 1000000000) {
		shmPtr->seconds += 1;
		shmPtr->nanoseconds -= 1000000000;
	}
}


//MAIN driver
int main(int argc, char* argv[]) {
	srand(time(NULL));

	int z;			//holds the lenght of logfile name entered
	char c;			//used for getopt
	int newLogfile = 0;
	int userCount;
	int usersReady;
	int pidHolder;	//temp pid holder
	int temp;
	int randomTime;
	unsigned int nanoAdd;
	key_t shmKey;
	key_t semKey;

	signal(SIGALRM, timesUp);
	signal(SIGINT, ctrlC);
	signal(SIGKILL, cleanAll);
	signal(SIGSEGV, cleanAll);

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

	//printf("HelloWorldNew \n");

	//defaults to logfile.txt
	if (newLogfile != 1) {
		strcpy(logfile, "logfile.txt");
	}

	fp = fopen(logfile, "a");

	//create and attach shared memory
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


	//create and attach semaphore array
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

	//
	while (1) {
		sleep(1);
		usersReady = countUsersReady();
		shmPtr ->noOfUsers = countTotalUsers();
		userCount = shmPtr->noOfUsers;
		//printf("User Count: %d \n", userCount);

		if(shmPtr->noOfUsers < MAX){
			//printf("got here\n");
			//printf("sec %d minus seconds %d", shmPtr->forkSec, shmPtr->seconds);
			//fork a user if allowed, base on the time clock
			if (shmPtr->forkSec == shmPtr->seconds) {
				printf("got here1\n");
				printf("nanosec %d minus nanoseconds %d\n", shmPtr->forkNanoSec, shmPtr->nanoseconds);
				printf("sec2 %d minus seconds2 %d\n", shmPtr->forkSec, shmPtr->seconds);
				if (shmPtr->forkNanoSec <= shmPtr->nanoseconds) {
					printf("got here2\n");
					releaseNextProcesses();
					nextFork();
				}
				
			}
			else if (shmPtr->forkSec < shmPtr->seconds) {
				printf("got here3\n");
				releaseNextProcesses();
				nextFork();
			}
		}


		//printf("Hello there \n");
		//always checks the blocked queue so that it can move it to ready if possible
		checkBlockedProcesses();

		sprintf(addToLogBuffer, "OSS: PID is ready to go \n");
		logging(addToLogBuffer);

		//users are ready proceed
		if (usersReady > 0) {
			//printf("inside if\n");
			pidHolder = goPids[readyOut];
			temp = getIndex(pidHolder);
			shmPtr->index = temp;

			randomTime = getRandomTime();
			shmPtr->nanoseconds += randomTime;
			clockCheck();

			//log
			sprintf(addToLogBuffer, "OSS: PID %d is ready to go \n", pidHolder);
			logging(addToLogBuffer);

			shmPtr->currentPid = pidHolder;

			goPids[readyOut] = 0;
			readyOut = (readyOut + 1) % MAX;

			sem_wait(semid);
			//printf("semwait done.\n");

			//more logging, depending on the status of the process
			//terminated earlier than scheduled
			if (shmPtr->pcb[temp].terminated) {
				sprintf(addToLogBuffer, "OSS: PID %d has terminated. TimeSpent with the CPU %d ns \n", pidHolder, shmPtr->pcb[temp].lastExecTime);
				logging(addToLogBuffer);
				usedProcesses[temp] = 0;
			}
			//process got blocked
			else if (shmPtr->pcb[temp].blocked) {
				blockedQ[blockedIn] = pidHolder;
				blockedIn = (blockedIn + 1) % MAX;
				sprintf(addToLogBuffer, "OSS: PID %d is going to the blocked queue. Last Exec Time / Burst: %d ns\n", pidHolder, shmPtr->pcb[temp].lastExecTime);
				logging(addToLogBuffer);
				sprintf(addToLogBuffer, "OSS: Time slice was not used all the way because the process got blocked. \n");
				logging(addToLogBuffer);
			}
			//finished all the way, no interrupt
			else {
				usedProcesses[temp] = 0;
				sprintf(addToLogBuffer, "OSS: PID %d is finish and terminated. Time used in CPU: %.2fms Time in system: %.2fms Last exec time/burst: %dns\n", pidHolder, shmPtr->pcb[temp].cpuTimeMs, shmPtr->pcb[temp].systemTimeMs, shmPtr->pcb[temp].lastExecTime);
				logging(addToLogBuffer);
			}

		}

		//time increment
		shmPtr->seconds += 1;
		nanoAdd = rand() % 1001;
		shmPtr->nanoseconds += nanoAdd;
		clockCheck();

	}

	printf("Program ending....");

	fclose(fp);
	cleanAll();
	return 0;


	///////////////////////////////END OF MAIN ////////////////////////////////////////////////////////	
}
