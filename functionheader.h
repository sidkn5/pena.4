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
#include <stdbool.h>

#define MAX 18			
#define QUANTUM 10		//time quantum

typedef struct {
	//just this for now

	int currentPid;
	int currentIndex;

	unsigned int lastExecTime;		//previous burst
	float cpuTimeMs;		//time the process used the CPU
	float systemTimeMs;		//time the process was in the system
	unsigned int nanoStart;			//time in nanoseconds that the user started		
	unsigned int secStart;			//time that the user started
	int blockedSeconds;		//time in secs when a process will be set to ready
	int blockedNanoseconds; //time in nanosec when a process will be set to ready
	int type;				//type of process, IO or CPU

	//flags
	bool blocked;			//process was bloked
	bool terminated;		//process terminated
	bool wait;
	

}processControlBlock;

typedef struct {

	int noOfUsers;		//no. of users that is currently spawned
	int currentPid;		//next pid that can do work
	int index;			//index of the above pid
	int forkSec;		//when it oss is allowed to fork again
	int forkNanoSec;	//when it oss is allowed to fork again
	unsigned int nanoseconds;	//time clock
	unsigned int seconds;		//time clock

	processControlBlock pcb[MAX];
}holder;
