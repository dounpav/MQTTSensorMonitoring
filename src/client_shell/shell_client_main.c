
/*
 * @author: Pavel Dounaev (dounpav)
 * @file: shell_client_main.c
 * @brief: main code for the program
*/


#include"shell_client.h"

// extern variable see shell_client.h
int g_signal_caught = 0;

int main(int argc, char *argv[]){

#if DEBUG

	int cid = 0;
	int fd = 0;
	char *broker_ip = "127.0.0.1";
	char *topic = "r/t";

#else

	if(argc != 5){

		fprintf(stderr, "error: incorrect amount of arguments\n");
		exit(EXIT_FAILURE);
	}

	int cid = strtod(argv[1], NULL);		// client id
	int fd = strtod(argv[2], NULL);			// filedescriptor to which send client information
	char *broker_ip = argv[3];			// broker ip address to which client will connect
	char *topic = argv[4];				// topic to which client will subscribe

#endif

	// setup signal handler
	struct sigaction sa = {0};
	client_setup_signal_handler(&sa);

	// setup client information
	struct client_info info;
	client_init_info(&info, cid, fd, broker_ip, topic);

	// initialize the mosquitto library
	mosquitto_lib_init();

	// create mosquitto instance
	struct mosquitto *mosq_client = NULL;
	mosq_client = mosquitto_new(NULL, CLEAN_SESSION, &info);

	if(mosq_client == NULL){
	
	#if DEBUG

		fprintf(stderr, "DEBUG: unable to create mosquitto instance\n");

	#endif
		info.status = CLIENT_CREAT_FAILURE;
		client_send_info(&info, mosq_client);

		// clean up library before terminating
		mosquitto_lib_cleanup();
		exit(EXIT_FAILURE);
	}
	else{

	#if DEBUG

		fprintf(stderr, "DEBUG: mosquitto instance created\n");

	#endif
	
		info.status = CLIENT_CREAT_SUCCESS;
		client_send_info(&info, mosq_client);
	}

	// set up callbacks for mosquitto client
	mqtt_setup_callbacks(mosq_client);

	// connect to a mosquitto broker
	mosquitto_connect(mosq_client, broker_ip, PORT, PING);

	// main client loop
	while(1){

		int con_loop = mosquitto_loop(mosq_client, TIMEOUT, MAX_PACKETS);
		if(con_loop != MOSQ_ERR_SUCCESS) break;
		if(g_signal_caught){
			break;
		}
	}

	// client cleanup code
	mosquitto_unsubscribe(mosq_client, NULL, topic);
	mosquitto_disconnect(mosq_client);
	mosquitto_destroy(mosq_client);
	mosquitto_lib_cleanup();

	return EXIT_SUCCESS;
}

