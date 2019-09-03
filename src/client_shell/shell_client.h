
/*
 * @author: Pavel Dounaev (dounpav)
 * @file: shell_client.h
 * @brief: definitions and descriptons of functions for shell client
*/

#ifndef SHELL_CLIENT_H
#define SHELL_CLIENT_H

#include"client_info.h"
#include<mosquitto.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<errno.h>

#define DEBUG		 0				// turn on(1) off(0) debugging	

#define CLEAN_SESSION	 1
#define QOS		 0				// mqtt quality of service
#define PORT		 1883				// mqtt port number
#define PING		 60				// mqtt ping interval in seconds
#define TIMEOUT		 (-1)				// mqtt timeout 
#define MAX_PACKETS	 1				// mqtt parameter for future use must be set to 1

// global variable used to indicate that signal was caught
extern int g_signal_caught;

// signal handler for SIGINT or SIGTERM
void client_sa_handler(int signo);

// setup signal handler for client
void client_setup_signal_handler(struct sigaction *sa);

// initialize client information
void client_init_info(struct client_info *info, int id, int fd, char *ip, char *topic);

// sends client information using file descriptor
void client_send_info(struct client_info *info, struct mosquitto *mosq);

// mqtt connect callback function
void mqtt_cb_connect(struct mosquitto *mosq, void *obj, int rc);

// mqtt disconnect callback function
void mqtt_cb_disconnect(struct mosquitto *mosq, void *obj, int rc);

// mqtt subscribe callback function
void mqqt_cb_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos);

// mqtt message callback function
void mqqt_cb_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);

// mqtt sets up callback functions for mosquitto client
void mqtt_setup_callbacks(struct mosquitto *mosq);

#endif	// SHELL_CLIENT_H

