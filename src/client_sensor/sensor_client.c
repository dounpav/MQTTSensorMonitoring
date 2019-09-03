
/*
 * @author: Pavel Dounaev (dounpav)
 * @file: sensor_client.c
 * @brief: declarations of sensor client functions
*/


#include"sensor_client.h"

// mqtt connect callback function
void mqtt_cb_connect(struct mosquitto *mosq, void *obj, int rc){
	
	if(rc == 0){
		fprintf(stdout, "sensor connected successfully\n");
	}
	else{
		fprintf(stderr, "error: sensor unable to connect\n");
		mosquitto_destroy(mosq);
		mosquitto_lib_cleanup();
		exit(EXIT_FAILURE);
	}
}

// disconnect callback function
void mqtt_cb_disconnect(struct mosquitto *mosq, void *obj, int rc){

	if(rc == 0){
		fprintf(stdout, "sensor disconnected normally\n");
	}
	else{
		fprintf(stderr, "sensor disconnected abnormally\n");	
	}
}

// sets up callback functions for mosquitto instance
void mqtt_set_callbacks(struct mosquitto *mosq){
	
	mosquitto_connect_callback_set(mosq, mqtt_cb_connect);
	mosquitto_disconnect_callback_set(mosq, mqtt_cb_disconnect);
}

// validates mosquitto topic
void mqtt_validate_topic(const char *topic){
	
	if(mosquitto_pub_topic_check(topic) != MOSQ_ERR_SUCCESS){

		fprintf(stderr, "error: topic %s is not valid topic string\n", topic);
		mosquitto_lib_cleanup();
		exit(EXIT_FAILURE);
	}
}

// excecutes a sensor program as a child process
void client_start_sensor(int pfd, const char *path, char *sensor_name){

	char pfd_arg[10] = {0};

	// convert pipe filedescriptor into program argument(string)
	sprintf(pfd_arg, "%d", pfd);

	// execute sensor program
	if(execl(path, sensor_name, pfd_arg, (char*)NULL) == -1){
		fprintf(stderr, "error: exec failed(%d) --- %s\n", errno, strerror(errno));
	}
	exit(EXIT_FAILURE);
}

// reads sensor data from pipe and publishes it to a mosquitto topic
void client_read_and_pub(int pfd, const char *topic, struct mosquitto *mosq_sensor){

	char sensor_data[SENSOR_DATA_LEN] = {0};

	while(1){
		
		// read incoming sensor data from pipe
		if( (read(pfd, sensor_data, sizeof(sensor_data))) == -1){
		
			printf("stop\n");

			fprintf(stderr, "read failed: %d --- %s", errno, strerror(errno));
			mosquitto_destroy(mosq_sensor);
			mosquitto_lib_cleanup();
			exit(EXIT_FAILURE);
		}
		// publish sensor data to a broker
		if(mosquitto_publish(mosq_sensor, NULL, topic, sizeof(sensor_data), sensor_data, QOS, RETAIN) == MOSQ_ERR_NO_CONN){
				
			fprintf(stderr, "error: unable to publish the message, client isnt connected to a valid broker\n");
			mosquitto_destroy(mosq_sensor);
			mosquitto_lib_cleanup();
			exit(EXIT_FAILURE);
		}

		int loop = mosquitto_loop(mosq_sensor, TIMEOUT, MAX_PACKETS);
		if(loop != MOSQ_ERR_SUCCESS){

			if(loop == MOSQ_ERR_CONN_LOST){
				fprintf(stderr,"error: connection to broker was lost\n");
			}
			else if(loop == MOSQ_ERR_NO_CONN){
				fprintf(stderr, "error: no connection to broker\n");
			}
			else if(loop == MOSQ_ERR_PROTOCOL){
				fprintf(stderr, "error: protocol error\n");
			}
			else{
				fprintf(stderr, "error: system call error\n");
			}
			break;
		}
	}
	mosquitto_destroy(mosq_sensor);
	mosquitto_lib_cleanup();
	exit(EXIT_FAILURE);
}

