
/*
 * @author: Pavel Dounaev (dounpav)
 * @file: shell_client.c
 * @brief: declarations of functions for shell client
*/

#include"shell_client.h"

// signal handler for SIGINT or SIGTERM
void client_sa_handler(int signo){

	g_signal_caught = signo;
}

// setup singal handler for client
void client_setup_signal_handler(struct sigaction *sa){
	
	sa->sa_handler = client_sa_handler;
	sa->sa_flags = 0;
	sigfillset(&sa->sa_mask);

	if(sigaction(SIGUSR1, sa, NULL) == -1){
		fprintf(stderr, "ERROR: cannot install SIGINT(%d) --- %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	// ignore the sigint signal
	sa->sa_handler = SIG_IGN;
	
	if(sigaction(SIGINT, sa, NULL) == -1){
		fprintf(stderr, "ERROR: cannot install SIGINT(%d) --- %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

// initialises client information
void client_init_info(struct client_info *info, int id, int fd, char *ip, char *topic){

	info->id = id;
	info->pid = getpid();
	info->pipefd = fd;
	info->status = CLIENT_INITIAL;
	info->slot_pos = -1;

	strcpy(info->ip, ip);
	strcpy(info->topic, topic);
	memset(info->data, '\0', CLIENT_DATA_LEN);
}

// sends client information via file descriptor
void client_send_info(struct client_info *info, struct mosquitto *mosq){

	int fd = info->pipefd;
	int ret = 0;

#if DEBUG
	
	if( (ret = write(fd, info->data, CLIENT_DATA_LEN)) == -1){

		fprintf(stderr, "DEBUG: user client unable to write filedesriptor\n");
		fprintf(stderr, "write failed(%d) --- %s\n", errno, strerror(errno));
	
		// cleanup and free rescources before terminating
		mosquitto_disconnect(mosq);
		mosquitto_destroy(mosq);
		mosquitto_lib_cleanup();

		exit(EXIT_FAILURE);
	}
	else{

		fprintf(stdout, "DEBUG: user client wrote struct/message %s to filedescriptor\n", info->data);
	}
#else

	if( (ret = write(fd, info, sizeof(struct client_info))) == -1){

		fprintf(stderr, "client %d(%d): write failed(%d) --- %s\n", info->id, info->pid, errno, strerror(errno));
		
		// cleanup and free rescources before terminating
		mosquitto_disconnect(mosq);
		mosquitto_destroy(mosq);
		mosquitto_lib_cleanup();

		exit(EXIT_FAILURE);
	}

#endif

}

// connect callback function
void mqtt_cb_connect(struct mosquitto *mosq, void *obj, int rc){

	struct client_info *info = (struct client_info*)obj;

	// connection attempt to a broker succeeds
	if(rc == 0){

		info->status = CLIENT_CONN_SUCCESS;

	#if DEBUG

		fprintf(stderr, "DEBUG: user client %d connected\n", info->id);

	#endif
		client_send_info(info, mosq);

		if(mosquitto_subscribe(mosq, NULL, info->topic ,QOS) != MOSQ_ERR_SUCCESS){

		#if DEBUG

			fprintf(stderr, "DEBUG: client subscription failed\n");

		#endif
			info->status = CLIENT_SUB_FAILURE;
			client_send_info(info, mosq);

			// free resources and clenup library before terminating
			mosquitto_destroy(mosq);
			mosquitto_lib_cleanup();
			exit(EXIT_FAILURE);
		}
	}

	// connection attemp to a broker fails
	else{

		info->status = CLIENT_CONN_FAILURE;

	#if DEBUG

		fprintf(stderr, "DEBUG: user client connection failed (%d)\n", rc);

	#endif
		client_send_info(info, mosq);
		
		// cleanup and free rescources before terminating
		mosquitto_destroy(mosq);
		mosquitto_lib_cleanup();

		exit(EXIT_FAILURE);
	}
}

// disconnect callback function
void mqtt_cb_disconnect(struct mosquitto *mosq, void *obj, int rc){

	struct client_info *info = (struct client_info*)obj;

	// client disconnected normally
	if(rc == 0){
		
		info->status = CLIENT_DISCON_SUCCESS;

	#if DEBUG

		fprintf(stderr, "DEBUG: user client disconnected normally\n");

	#endif
		client_send_info(info, mosq);

	}

	// client disconnected abnormally
	else{
		
		info->status = CLIENT_CONN_LOST;

	#if DEBUG

		fprintf(stderr, "DEBUG: user client connection lost(%d)\n", rc);
		
	#endif
		client_send_info(info, mosq);
		
		// cleanup and free rescources before terminating
		mosquitto_destroy(mosq);
		mosquitto_lib_cleanup();

		exit(EXIT_FAILURE);
	}
}

// subscribe callback function
void mqqt_cb_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos){

	struct client_info *info = (struct client_info*)obj;

#if DEBUG

	fprintf(stdout, "DEBUG: user client subsctiption success\n");

#endif

	// send client information
	info->status = CLIENT_SUB_SUCCESS;
	client_send_info(info, mosq);
}

// message callback function
void mqqt_cb_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message){

	struct client_info *info = (struct client_info*)obj;

	if(message->payloadlen){

	#if DEBUG

		fprintf(stdout, "DEBUG: client user received message %s\n", (char*)message->payload);

	#endif
		info->status = CLIENT_DATA_READY;
		strcpy(info->data, (char*)message->payload);
	}else{

	#if DEBUG

		fprintf(stderr, "DEBUG: client user didnt receive a message\n");
		info->status = CLIENT_DATA_MISSING;

	#endif

	}

	// send client information
	client_send_info(info, mosq);
}

// sets up callback functions for mosquitto instance
void mqtt_setup_callbacks(struct mosquitto *mosq){
	
	mosquitto_connect_callback_set(mosq, mqtt_cb_connect);
	mosquitto_disconnect_callback_set(mosq, mqtt_cb_disconnect);
	mosquitto_subscribe_callback_set(mosq, mqqt_cb_subscribe);
	mosquitto_message_callback_set(mosq, mqqt_cb_message);
}



