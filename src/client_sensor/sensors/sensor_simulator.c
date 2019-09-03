
// file: sensor_simulator.c

#include<stdio.h>
#include<stdlib.h>
#include<time.h>	// time_t
#include<unistd.h>	// write
#include<string.h>	// strtod
#include<errno.h>

#define DELAY		2
#define UPPER		30
#define LOWER		10
#define SENSOR_DATA_LEN	20

int main(int argc, char *argv[]){

	if(argc != 2){
		fprintf(stderr, "error: filedescriptor not specified\n");
		exit(EXIT_FAILURE);
	}
	
	if(UPPER < LOWER){
		fprintf(stderr, "error: upper limit is lower than lower limit\n");
		exit(EXIT_FAILURE);
	}

	// get file descriptor of the pipe
	int pipefd = strtod(argv[1], NULL);
	if(pipefd < 0){
		fprintf(stderr, "invalid filedescriptor\n");
		exit(EXIT_FAILURE);
	}

	char sensor_data[SENSOR_DATA_LEN] = {0};	// simulated sensor data

	// initialize seed for random number generator
	srand(time(NULL));

	while(1){
		
		// random number between upper and lower limits
		int rval = (rand() % (UPPER - LOWER + 1)) + LOWER;	
		sprintf(sensor_data, "%d", rval);
		
		if((write(pipefd, sensor_data, sizeof(sensor_data))) == -1){
			
			fprintf(stderr, "write failed: %d --- %s\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
		else{
			sleep(DELAY);
		}
	}

	return EXIT_SUCCESS;
}
