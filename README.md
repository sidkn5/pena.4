THIS IS A TEST
Student: Sean Dela Pena 

Professor: Dr. Sanjiv Bhatia

Assignment 4: Process Scheduling

Purpose: The goal of this homework is to learn about process scheduling inside an operating system. You will work on the specified
scheduling algorithm and simulate its performance. To be more specific we are using the round roubin scheduling algorithm.

USAGE: USAGE: ./oss [-h] [-s time] [-l logfile]

Problems/Notes:

	Not really a problem with regards to the program but a problem with git and github.
	I emailed Dr. Bhatia about my situation, how I didn't have internet connection over break.
	That being said I couldn't really commit every time I wanted to. Also when I tried
	to push, my connection cut out causing git hub to make a new branch on my repo. Basically
	my repo in github for this project is a mess. I apologize.

	For syschronization I decided to use semaphores for this project. I heard message queues
	were great for this project. But since I already had semaphores setup because of the last 
	project I decided to stick with semaphores. That being said, semaphores and the scheduling 
	algorithm are based on the stallings book. 

	The logifle that will be given using the -l option can be any string that is
	less than 30 chars. Please note that if you don't add the .txt extension it 
	will not be removed when performing a make clean. The default name is logfile.txt.

	In terms of logging, I don't think I missed anything but I did not copy the logging example
	given in the prompt word per word so it might look different. 

	During the report at the end of the program, blocked percentage will like be low if not 0 because
	of this. This happens especially when the program is ran only for a short amount of time.

	One problem I had was that everytime one of the user processes terminates my OSS also terminates.
	I found that the problem was in my sigchld handler function. So I was able to fix that and was not 
	able to replicate the problem as a kept on testing. 

	Also for the blocking there is only a 25% chance of it being interrupted and being sent to
	the blocked queue. It was mentioned that this number is arbitrary so I just decided 25% is
	not too bad of a chance for interrupts.
	During the report at the end of the program, blocked percentage will like be low if not 0 because
	of this. This happens especially when the program is ran only for a short amount of time.

	There are also sleeps added in the system so it is common that CPU util is low.

	That being said, I didn't find any bugs from my testing. 


Functions in oss.c:

help():
	
	-displays help menu

checkTime():

	-checks the time in sec given by user, can't exceed 100 sec (default)

report():

	-report statistics

killChildPid():

	-ensures that pids are killed 

cleanAll():

	-deallocates and frees everything

ctrlC():

	-called for ctrl+C termination

timesUp():
	
	-called for early time termination

logging():

	-logging function to the logfile, maximum 10 000 lines

resetPid():

	-resets the pid to 0

mySigchldHandeler():

	-takes care of children who dies, reffered from stackoverflow


countUsersReady():

	-count the user process that are ready

initializePcb():

	-init PCB

countTotalusers():

	-return the total number of users/processes

freeIndex():

	-searches for a free index that can be used

forkChild():

	-forking of the user process

releaseNextProcesses():

	-dispatch the next process

getIndex():

	-gets the index of the pid

checkBlockedProcess():

	-checks the blocked queue if a process can be moved to ready queue

sem_wait():

	-sem_wait used for semaphore, referred from the stallings book

getRandomTime():

	-returns a random number time



Functions in user.c:

handler():

	-handle the clean up when terminating or early termination

sem_signal():

	-sem_signal func for semaphore, increments, referred from the book

converSec():

	-converts sec to ms

converNano():

	-converts nano to ms


