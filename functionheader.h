
/*
Student: Sean Dela Pena
Professor: Dr. Sanjiv Bhatia
Date: 4/4/2021

*/

#include <stdlib.h>
#include <stdbool.h>

#define MAX 18
#define QUANTUM 10


typedef struct {
	//just this for now

	int currentPid;
	int currentIndex;

	float lastExecTime;		//previous burst
	float cpuTimeMs;		//time the process used the CPU
	float systemTimeMs;		//time the process was in the system
	unsigned int nanoStart;			//time in nanoseconds that the user started		
	unsigned int secStart;			//time that the user started
	int blockedSeconds;		//time in secs when a process will be set to ready
	int blockedNanoseconds; //time in nanosec when a process will be set to ready
	bool blocked;
	bool terminated;
	bool wait;

}processControlBlock;

typedef struct {

	int noOfUsers;		//no. of users that is currently spawned
	int currentPid;
	int index;
	
	int forkSec;
	int forkNanoSec;
	unsigned int nanoseconds;
	unsigned int seconds;

	processControlBlock pcb[MAX];
}holder;
