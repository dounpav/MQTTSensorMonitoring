
// file: client_info.h

#ifndef CLIENT_INFO_H
#define CLIENT_INFO_H

#include<sys/types.h>	// pid_t

#define CLIENT_DATA_LEN		5
#define CLIENT_TOPIC_LEN 	20
#define IP_ADDR_LEN		15

// enum holding client status
enum client_status{

	CLIENT_INITIAL,			// initial status of the client
	CLIENT_CREAT_SUCCESS,		// client created
	CLIENT_CREAT_FAILURE,		// client creation failed
	CLIENT_CONN_SUCCESS,		// client connected
	CLIENT_CONN_FAILURE,		// client unable to connect
	CLIENT_DISCON_SUCCESS,		// client disconnected normally
	CLIENT_CONN_LOST,		// client lost connection
	CLIENT_SUB_SUCCESS,		// client subscribed	
	CLIENT_SUB_FAILURE,		// client subscuption failed
	CLIENT_DATA_READY,		// client is sending data
	CLIENT_DATA_MISSING		// client did not receive data from broker
};

// structure holding information about client
struct client_info{

	int   			id;				// client id
	pid_t 			pid;				// client process id
	char  			data[CLIENT_DATA_LEN];		// data that client sends
	enum client_status	status;				// client satus
	char			ip[IP_ADDR_LEN];		// ip address to which client is connected
	char			topic[CLIENT_TOPIC_LEN];	// topic to which client is subscribed
	int			pipefd;				// pipe to which client writes
	int			slot_pos;			// clients slot position in list
};



#endif
