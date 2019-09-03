
/*
 * @author: Pavel Dounaev (dounpav)
 * @file: shell.h
 * @brief: definitions and descriptons of functions for shell
*/


#ifndef SHELL_H
#define SHELL_H

#include"client_info.h"
#include<mosquitto.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<math.h>
#include<unistd.h>
#include<sys/wait.h>
#include<signal.h>
#include<errno.h>
#include<fcntl.h>


#define USE_BUILTIN		1	// use gcc builtin function

#define SHELL_TERMINATE		1		
#define CLIENTS_MAX_CNT		5
#define CLIENT_SLOTS_FULL 	(int)(pow(2, CLIENTS_MAX_CNT) -1)

#if USE_BUILTIN

#define FIND_EMPTY_SLOT(x)	find_msb(x)
#define FIND_SET_SLOT(x)	find_msb(x)

#endif // USE_BUILTIN

#define TOPIC_MAX_LEN		100
#define INPUT_OK		0
#define INPUT_FAIL		1
#define INPUT_LONG		2
#define INPUT_SHORT		3

#define VALID_ADDR		0
#define INVALID_ADDR		1

#define LOG_MSG_LEN		80

// client list structure
struct client_list{

	struct 	  	client_info *clients;	// pointer to client array
	int 	  	slots;			// used slots in array
	int 		slots_max;		// maximum value of used slots
	bool		full;			// list is full
};


extern int g_signal_caught;

// signal handler for the shell
void shell_sa_handler(int signo);

// sets up singal handler for the shell
void shell_setup_signal_handler(struct sigaction *sa);

// reads option number from the user
int shell_read_option();

// reads string from the user
int shell_read_string(char *prompt, char *str, size_t sz);

// checks if segment is a number
int seg_is_number(char *seg);

// validates a ip address
int shell_validate_address(char *ip);


#if USE_BUILTIN 	// use functions with gcc builtin functions

// finds most significant bit from 32bit value
int find_msb(int x);

// terminates the shell and all client processes uses gcc builtin function
void shell_terminate_blt(int *flag, struct client_list *clist);

// adds client to client list and sets slot position/number for the client uses gcc builtin function
void shell_add_client_blt(struct client_info *client, struct client_list *clist);

// finds and removes client from client list uses gcc builtin function
void shell_rm_client_blt(pid_t pid, struct client_list *clist);

// shows clients that are currently connected
void shell_show_clients_blt(struct client_list *clist);

// connects client to a sensor
void shell_connect_sensor_blt(char *pipefd);

// disconnects the client from the sensor
void shell_disconnect_sensor_blt(struct client_list *clist);

#else // USE_BUILTIN

// terminates the shell and all client processes
void shell_terminate(int *flag, struct client_list *clist);

// adds client to client list and sets slot position/number for the client
void shell_add_client(struct client_info *client, struct client_list *clist);

// finds and removes client from client list
void shell_rm_client(pid_t pid, struct client_list *clist);

// deletes client from the client list
void shell_del_client(int *client_slots, pid_t pid, int n);

// shows clients that are currently connected
void shell_show_clients(struct client_list *clist);

// connects client to a sensor
void shell_connect_sensor(char *pipefd);

// disconnects the client from the sensor
void shell_disconnect_sensor(struct client_list *clist);

#endif // USE_BUILTIN


// creates new client process
void shell_create_client(char *pipefd, char *ip, char *topic);

// manages client coming from the common pipe
void shell_manage_client(int fd, int log_fd, struct client_info *info, struct client_list *clist);

// handles request from the user
void shell_handle_request(char *pipefd, int *flag, struct client_list *clist);

// opens or creates a new log file
int shell_log_open(const char *path);

// logs a message to a log file
void shell_log_write(int fd, const char msg[]);


#endif // SHELL_H
