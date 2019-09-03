
/*
 * @author: Pavel Dounaev (dounpav)
 * @file: sensor_client.h
 * @brief: definitions and descriptions of shell client functions
*/


#ifndef SENSOR_CLIENT_H
#define SENSOR_CLIENT_H

#include<mosquitto.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>


#define SENSOR_DATA_LEN	20

#define QOS		0
#define RETAIN		0
#define PORT		1883
#define PING		60
#define TIMEOUT 	(-1)
#define MAX_PACKETS	1


// connect callback function
void mqtt_cb_connect(struct mosquitto *mosq, void *obj, int rc);

// disconnect callback function
void mqtt_cb_disconnect(struct mosquitto *mosq, void *obj, int rc);

// sets up callback functions for mosquitto instance
void mqtt_set_callbacks(struct mosquitto *mosq);

// validates mosquitto topic
void mqtt_validate_topic(const char *topic);

// executes sensor program as a child process
void client_start_sensor(int pfd, const char *path, char *sensor_name);

// reads sensor data from pipe and publishes it to a topic
void client_read_and_pub(int pfd, const char *topic, struct mosquitto *mosq_sensor);

#endif	// SENSOR_CLIENT_H
