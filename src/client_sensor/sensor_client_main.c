
/*
 * @author: Pavel Dounaev (dounpav)
 * @file: sensor_client_main.c
 * @brief: main code for sensor client
*/


#include"sensor_client.h"

int main(int argc, char* argv[]){

	struct mosquitto *mosq_sensor = NULL;	// sensors mosquitto instance
	const char *path  = argv[1];		// path to sensor program that will be excecuted
	const char *ip    = argv[2];		// ip address of the broker
	const char *topic = argv[3];		// mosquitto topic to which to publish
	int pipefd[2];				// pipe from which sensor client will recieve data from sensor

	if(argc != 4){
		fprintf(stderr, "error: incorrect amount of arguments\n");
		exit(EXIT_FAILURE);
	}
	
	if( (pipe(pipefd)) == -1 ){
		fprintf(stderr, "error creating pipe: %d --- %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	// initialize the mosquitto library
	mosquitto_lib_init();
	mqtt_validate_topic(topic);

	// create mosquitto instance
	mosq_sensor = mosquitto_new(NULL, true, (void*)topic);
	if(mosq_sensor == NULL){
		fprintf(stderr, "error: unable to create a moquitto instance\n");
		mosquitto_lib_cleanup();
		exit(EXIT_FAILURE);
	}

	// setup callbacks for client
	mqtt_set_callbacks(mosq_sensor);

	// connect to a broker
	mosquitto_connect(mosq_sensor, ip, PORT, PING);

	// parent process(sensor_client) will fork child process that will execute sensor program
	pid_t pid = fork();

	// child process
	if(pid == 0){
		close(pipefd[0]);
		client_start_sensor(pipefd[1], path, "sensor");
	}
	// parent process
	else if(pid > 0){
		close(pipefd[1]);
		client_read_and_pub(pipefd[0], topic, mosq_sensor);
	}
	else{
		fprintf(stderr, "fork failed: %d --- %s\n", errno, strerror(errno));
		mosquitto_disconnect(mosq_sensor);
		mosquitto_destroy(mosq_sensor);
		mosquitto_lib_cleanup();
		exit(EXIT_FAILURE);
	}
	
	return EXIT_SUCCESS;
}


