#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int timeTermination = 100;              //default time termination
char logfile[30];

void help(){
    printf("HELP MENU: \n\n");
}

int checkTime(int n){
	if(n > 100){
		printf("Will default to time 100s.\n");
		return 100;
	} else {
		return n;
	}
}

//deallocates and frees everything
void cleanAll(){
	//killChild();
	exit(0);
}

void ctrlC(){
	printf("Process terminate. CTRL + C caught\n");
	cleanAll();
	exit(0);
}

void timesUp(){
	printf("The time given is up. Process will terminate.\n");
	cleanAll();
	exit(0);
}

int main(int argc, char *argv[]){
    int z;          //holds the length of logfile name entered
    char c;         //used for getopt


    signal(SIGALRM, timesUp);
	signal(SIGINT, ctrlC);
	signal(SIGKILL, cleanAll);

	//referred from stackoverflow
	//struct sigaction sa;
	//memset(&sa, 0, sizeof(sa));
	//sa.sa_handler = mySigchldHandler;
	//sigaction(SIGCHLD, &sa, NULL);
    alarm(timeTermination);


    ///////////////////////////////////////////////////////////////////////////////////////////
    //GETOPT

    while((c = getopt(argc, argv, "hs:l:")) != -1){
		switch(c){
			//displays help menu
			case 'h':
				help();
				return 0;
				break;
			
            case 's':
                timeTermination = checkTime(atoi(optarg));
				printf("Process will terminate in %d sec(s)\n", timeTermination);
				alarm(timeTermination);
                break;
            
            case 'l':
                z = strlen(optarg);
				if(z < 30){
					strcpy(logfile, optarg);
                    printf("logfile name: %s\n", logfile);
		
				}else{
					strcpy(logfile, "logfile.txt");	
                    printf("logfile name: %s\n", logfile);
				}
                break;

			default:
				errno = 3;
				perror("Please refer to -h help for proper use of the porgram");
				exit(0);
		}
	}




    return 0;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    //END OF MAIN
}