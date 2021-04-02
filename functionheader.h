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
	int nanoStart;			//time in nanoseconds that the user started		
	int secStart;			//time that the user started
	int blockedSeconds;		//time in secs when a process will be set to ready
	int blockedNanoseconds; //time in nanosec when a process will be set to ready
	bool blocked;
	bool terminated;
	bool wait;

}processControlBlock;

typedef struct {

	int noOfProcesses;		//no. of users that is currently spawned
	int currentPid;
	int index;
	
	int forkSec;
	int forkNanoSec;
	int nanoseconds;
	int seconds;

	processControlBlock pcb[MAX];
}holder;
