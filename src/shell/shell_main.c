
/*
 *@author: Pavel Dounaev (dounpav)
 *@file: shell_main.c
 *@brief: main code of the program
*/

#include"shell.h"
#include<time.h>

int g_signal_caught = -1;

int main(void){

	struct sigaction sa;
	shell_setup_signal_handler(&sa);

	int pipefd[2];		// common pipe
	char pipefd_w[3];	// write part of the pipe as string

	struct client_info client;
	struct client_info clients[CLIENTS_MAX_CNT];
	memset(clients, 0, sizeof(clients));

	struct client_list clist;
	clist.clients = clients;
	clist.slots = 0;
	clist.slots_max = CLIENT_SLOTS_FULL;
	clist.full = false;

	int flag = 0;

	time_t raw_time;
	struct tm *timeinfo;
	char log_msg[LOG_MSG_LEN];

	// create common pipe
	if(pipe(pipefd) == -1){

		fprintf(stderr, "error: pipe creation failed(%d) --- %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	// convert filedescriptor number into string
	sprintf(pipefd_w, "%d", pipefd[1]);

	int log_fd = shell_log_open("log.txt");
	time(&raw_time);
	timeinfo = localtime(&raw_time);
	strftime(log_msg, LOG_MSG_LEN, "\nshell session started %F %T\n", timeinfo);
	shell_log_write(log_fd, log_msg);

	while(1){

		if(g_signal_caught == SIGINT){
			
			g_signal_caught = -1;

			// if termination flag is set skip request handling
			if(flag != SHELL_TERMINATE){
				shell_handle_request(pipefd_w, &flag, &clist);
			}
		}
		// if termination flag is set and there are no clients left terminate the shell
		if( (flag == SHELL_TERMINATE) && (clist.slots == 0) ){
			
			// log to a file and close it
			time(&raw_time);
			timeinfo = localtime(&raw_time);
			strftime(log_msg, LOG_MSG_LEN, "shell session ended %F %T\n", timeinfo);
			shell_log_write(log_fd, log_msg);
			close(log_fd);
			break;
		}

		shell_manage_client(pipefd[0], log_fd, &client, &clist);
	}

	return EXIT_SUCCESS;
}

