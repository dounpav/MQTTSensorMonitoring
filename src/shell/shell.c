
/*
 * @author: Pavel Dounaev (dounpav)
 * @file: shell.c
 * @brief: declarations of shell functions
 * @note: descriptions for the functions in shell.h
*/


#include"shell.h"

void shell_sa_handler(int signo){

	g_signal_caught = signo;
}

// setup singal handler for shell
void shell_setup_signal_handler(struct sigaction *sa){

	sa->sa_handler = shell_sa_handler;
	sa->sa_flags = 0;
	sigfillset(&sa->sa_mask);

	if(sigaction(SIGINT, sa, NULL) == -1){

		int en = errno;
		fprintf(stderr, "ERROR cannot install SIGINT(%d) --- %s\n", en, strerror(en));
		exit(EXIT_FAILURE);
	}
}

// read option number from user
int shell_read_option(){

	int opt = 0;
	int res = scanf("%d", &opt);

	getchar();
	if(res == EOF){
		fprintf(stdout, "error: EOF caught, try again\n");
	}
	if(res == 0){
		while(fgetc(stdin) != '\n');
	}

	return opt;
}

// reads string from the user
int shell_read_string(char *prompt, char *str, size_t sz){

	int ch;
	int extra;	// excess characters
	
	if(prompt != NULL){
		printf("%s", prompt);
		fflush(stdout);
	}

	// reading input failed
	if(fgets(str, sz, stdin) == NULL){
		return INPUT_FAIL;
	}

	if(str[0] == '\n') return INPUT_SHORT;

	// if string was too long flush till the end of line
	if(str[strlen(str)-1] != '\n'){
		extra = 0;
		while( ((ch = getchar()) != '\n') && (ch != EOF)){
			extra = 1;
		}
		if(extra == 1){
			return INPUT_LONG;
		}
		return INPUT_OK;
	}

	// remove newline and give string to caller
	str[strlen(str)-1] = '\0';
	return INPUT_OK;
}

// checks if segement is a integer value
int seg_is_number(char *seg){

	int nums = 0;

	while(*seg){

		if(isdigit(*seg)){
			nums++;
			seg++;
		}
		else return 0;
	}

	if(nums > 3){
		return 0;
	}

	return 1;
}

// validates ip address
int shell_validate_address(char *ip){

	char ip_addr[IP_ADDR_LEN] = {0};
	strcpy(ip_addr, ip);

	int dots = 0;	// dots in ip address
	char *seg;	// next segment of ip address

	// check if ip address ends or starts with dot
	if( (ip_addr[strlen(ip_addr) - 1] == '.') || (ip_addr[0] == '.') ){
		return 0;
	}

	// get next segment
	seg = strtok(ip_addr, ".");

	while(1){

		if(seg_is_number(seg)){

			// check if segment is within 0-255 value range
			int seg_num = atoi(seg);
			if(seg_num >= 0 && seg_num <= 255){
				
				seg = strtok(NULL, ".");
				if(seg != NULL){ 
					dots++;
				}
				else{ 
					break;
				}
			}
			else{
				// invalid ip address
				return 0;
			}
		}
		else{
			// invalid ip address
			return 0;
		}
	}
	if(dots != 3){
		return 0;
	}
	return 1;
}


#if USE_BUILTIN

// finds most significant bit from 32bit value
int find_msb(int x){
	
	// uses gcc builtin function to count leading zeros
	// zero is not passed to function to avoid undefined behaviour
	if(x == 0) return 0;
	return 31 - __builtin_clz(x);
}

// terminates the shell and all client processes
void shell_terminate_blt(int *flag, struct client_list *clist){

	int temp = clist->slots;
	*flag = SHELL_TERMINATE;

	while(temp != 0){

		// find set slot from client list
		int n = FIND_SET_SLOT(temp);
		temp = temp ^ (1 << n);
		
		// send termination signal to the client process
		kill(clist->clients[n].pid, SIGUSR1);
		fprintf(stdout, "sending signal to client %d(%d)\n", clist->clients[n].id, clist->clients[n].pid);
	}
}

// adds client to client list and sets slot position for the client
void shell_add_client_blt(struct client_info *client, struct client_list *clist){

	int slots = clist->slots;
	int slots_inv = slots;

	// mask(reverse) the slot values
	// now all set bits in slots_inv are empty slots
	slots_inv = slots_inv ^ CLIENT_SLOTS_FULL;

	// find empty slot
	int n = FIND_EMPTY_SLOT(slots_inv);
		
	// set slot for the new client
	slots = slots | (1 << n);
	clist->slots = slots;

	// set clients position in the client list
	client->slot_pos = n;

	// add new client to client list
	clist->clients[n] = *(client);

	if(clist->slots == (clist->slots_max)){
		clist->full = true;
	}
}

// finds and removes client from client list
void shell_rm_client_blt(pid_t pid, struct client_list *clist){

	int slots = clist->slots;
	int temp = slots;

	// search for a client using process id
	while(temp != 0){

		int n = FIND_SET_SLOT(temp);
		temp = temp ^ (1 << n);

		if(clist->clients[n].pid == pid){
			
			// remove the client
			// doesnt actually remove the client from the client list 
			// just sets the slot where client resides as empty

			clist->slots = slots ^ (1 << n);
		}
	}
}

// shows clients that are currently connected
void shell_show_clients_blt(struct client_list *clist){

	int temp = clist->slots;
	struct client_info *clients = clist->clients;
	
	fprintf(stdout, "connected clients:\n");
	fprintf(stdout, "OPTION	CID	PID	SENSOR\n");

	while(temp != 0){

		int n = FIND_SET_SLOT(temp);
		temp = temp ^ (1 << n);
		fprintf(stdout, "%d %d	%d	%s\n", n, clients[n].id, clients[n].pid, clients[n].topic);
	}
}

// connects to a sensor
void shell_connect_sensor_blt(char *pipefd){

	char ip[IP_ADDR_LEN];
	char topic[TOPIC_MAX_LEN];

	int ret = shell_read_string("enter ip address of the broker: ", ip, sizeof(ip));
	if(ret == INPUT_FAIL){
		fprintf(stderr, "error: reading input failed\n");
	}
	if(ret == INPUT_OK){

		if(shell_validate_address(ip)){
			
			int ret = shell_read_string("enter topic of the sensor: ", topic, sizeof(topic));
		
			if(ret == INPUT_FAIL){
				fprintf(stderr, "error: reading input failed\n");
			}
			if(ret == INPUT_LONG){
				fprintf(stderr, "error: topic is too long\n");
			}
			if(ret == INPUT_SHORT){
				fprintf(stderr, "error: topic is too short");
			}
			if(ret == INPUT_OK){
				shell_create_client(pipefd, ip, topic);
			}
		}
		else{
			fprintf(stderr, "error: invalid ip address\n");
		}
	}
}

// disconnects from a sensor
void shell_disconnect_sensor_blt(struct client_list *clist){

	int slots = clist->slots;
	struct client_info *clients = clist->clients;

	// show connected clients
	shell_show_clients_blt(clist);

	fprintf(stdout, "option: ");
	int option = shell_read_option();
	
	if( !(slots & (1 << option)) ){
		fprintf(stdout, "error: invalid option, no such option is available\n");
	}
	else{
		// send termination signal to a client process
		kill(clients[option].pid, SIGUSR1);
		fprintf(stdout, "sending signal to client %d(%d)\n", clients[option].id, clients[option].pid);
	}
}

#else // USE_BUILTIN


// terminates the shell and all client processes
void shell_terminate(int *flag, struct client_list *clist){

	int slots = clist->slots;
	struct client_info *clients = clist->clients;

	*flag = SHELL_TERMINATE;		// set shell termination flag
	
	for(int n = 0; n < CLIENTS_MAX_CNT; n++){
	
		// find set slot
		if( (slots & (1 << n)) ){

			// send termination signal to the client process
			kill(clients[n].pid, SIGUSR1);
			fprintf(stdout, "sending signal to client %d(%d)\n", clients[n].id, clients[n].pid);
		}
	}
}

// adds client to client list and sets slot position for the client
void shell_add_client(struct client_info *client, struct client_list *clist){

	int slots = clist->slots;
	
	for(int n = 0; n < CLIENTS_MAX_CNT; n++){

		// find empty slot
		if( !(slots & (1 << n)) ){
			
			// set slot for the new client
			slots = slots | (1 << n);
			clist->slots = slots;

			// set clients position in the client list
			client->slot_pos = n;

			// add new client to client list
			clist->clients[n] = *(client);
			break;
		}
	}
	if(clist->slots == clist->slots_max){
		clist->full = true;
	}
}

// finds and removes client from client list
void shell_rm_client(pid_t pid, struct client_list *clist){

	int slots = clist->slots;
	struct client_info *clients = clist->clients;

	// search for a client using process id
	for(int n = 0; n < CLIENTS_MAX_CNT; n++){
		
		// find set slot
		if( (slots & (1 << n)) ){

			if(clients[n].pid == pid){
		
				// remove the client
				// doesnt actually remove the client from the client list 
				// just sets the slot where client resides as empty

				clist->slots = slots ^ (1 << n);
			}
		}
	}
}

// deletes client from the client list(outdated function not in use)
void shell_del_client(int *client_slots, pid_t pid, int n){

	// does not actually delete the client from the list just sets the slot for that client as empty
	// so the client can be later just be overwritten
	// also sends termination signal to the client process

	*client_slots = *client_slots ^ (1 << n);
	kill(pid, SIGUSR1);
}

// shows clients that are currently connected
void shell_show_clients(struct client_list *clist){

	int slots = clist->slots;
	struct client_info *clients = clist->clients;
	
	fprintf(stdout, "connected clients:\n");
	fprintf(stdout, "CID	PID	SENSOR IP\n");
		
	for(int n = 0; n < CLIENTS_MAX_CNT; n++){
			
		// find set slot
		if( (slots & (1 << n)) ){
			fprintf(stdout, "%d	%d	%s\n", clients[n].id, clients[n].pid, clients[n].topic, clients[n].ip);
		}
	}
}

// connects to a sensor
void shell_connect_sensor(char *pipefd){

	char ip[IP_ADDR_LEN];
	char topic[TOPIC_MAX_LEN];

	int ret = shell_read_string("enter ip address of the broker", ip, sizeof(ip));
	if(ret == INPUT_FAIL){
		fprintf(stderr, "error: reading input failed\n");
	}
	if(ret == INPUT_OK){

		if(shell_validate_address(ip)){
			
			ret = shell_read_string("enter topic of the sensor: ", topic, sizeof(topic));
		
			if(ret == INPUT_FAIL){
				fprintf(stderr, "error: reading input failed\n");
			}
			if(ret == INPUT_LONG){
				fprintf(stderr, "error: topic is too long\n");
			}
			if(ret == INPUT_SHORT){
				fprintf(stderr, "error: topic is too short\n");
			}
			if(ret == INPUT_OK){
				shell_create_client(pipefd, ip, topic);
			}
		}
		else{
			fprintf(stderr, "error: invalid ip address\n");
		}
	}
}

// disconnects from a sensor
void shell_disconnect_sensor(struct client_list *clist){

	int slots = clist->slots;
	struct client_info *clients = clist->clients;

	// show connected clients
	shell_show_clients(clist);

	fprintf(stdout, "option: ");
	int option = shell_read_option();
	printf("opt %d\n", option);

	if( !(slots & (1 << option)) ){
		fprintf(stdout, "error: invalid option, no such option is available\n");
	}
	else{
		// send termination signal to a client process
		kill(clients[option].pid, SIGUSR1);
	}
}

#endif // USE_BUILTIN


// creates new client process
void shell_create_client(char *pipefd, char *ip, char *topic){

	static int cid = 0;	// client id to be assigned to a next client process
	char cid_arg[2];	// client id as an argument for client program	
	
	// convert cid intger to a string
	sprintf(cid_arg, "%d", cid);
	cid++;

	pid_t pid;
	pid_t cpid;

	pid = fork();

	if(pid > 0){
		wait(NULL);
	}
	else if(pid == 0){

		cpid = fork();

		if(cpid > 0){
			exit(EXIT_SUCCESS);
		}
		else if(pid == 0){
			
			// execute the new user client process that will handle the sensor
			if(execl("../client_shell/shell_client", "shell_client", cid_arg, pipefd, ip, topic, (char*)NULL) == -1){
				fprintf(stderr, "shell error: exec failed(%d) --- %s\n", errno, strerror(errno));
			}
			exit(EXIT_FAILURE);
		}
		else{
			fprintf(stderr, "error: first fork failed(%d) --- %s\n", errno, strerror(errno));
		}
	}
	else{
		fprintf(stderr, "error: second fork failed(%d) --- %s\n", errno, strerror(errno));
		cid--;
	}
}

// manages clients coming from the common pipe
void shell_manage_client(int fd, int log_fd, struct client_info *info, struct client_list *clist){

	char log_msg[LOG_MSG_LEN] = {0};

	// receive client information from pipe
	int ret = read(fd, info, sizeof(struct client_info));

	if(ret == -1){

		if(errno != EINTR){		// do nothing if read was interrupted
			fprintf(stderr, "read failed(%d) --- %s\n", errno, strerror(errno));
			fprintf(stdout, "terminating the program...\n");
			exit(EXIT_FAILURE);
		}
	}
	if(ret == 0){
		fprintf(stderr, "no data(%d) --- %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(ret > 0){

		switch(info->status){

			case CLIENT_CREAT_SUCCESS:
				sprintf(log_msg, "client %d(%d) created\n", info->id, info->pid);
				break;

			case CLIENT_CREAT_FAILURE:
				sprintf(log_msg, "client %d(%d) unable to be created\n", info->id, info->pid);
				break;

			case CLIENT_CONN_SUCCESS:
				sprintf(log_msg, "client %d(%d) connected to %s\n", info->id, info->pid, info->ip);
				break;

			case CLIENT_CONN_FAILURE:
				sprintf(log_msg, "client %d(%d) unable to connect\n", info->id, info->pid);
				break;

			case CLIENT_SUB_SUCCESS:
				sprintf(log_msg, "client %d(%d) subscribed to topic %s\n", info->id, info->pid, info->topic);

				// add client to the client list
				#if USE_BUILTIN
				shell_add_client_blt(info, clist);
				#else
				shell_add_client(info, clist);
				#endif // USE_BUILTIN
				break;

			case CLIENT_SUB_FAILURE:
				sprintf(log_msg, "client %d(%d) unable to subscribe to topic %s\n", info->id, info->pid, info->topic);
				break;

			case CLIENT_CONN_LOST:
				sprintf(log_msg, "client %d(%d) lost connection to %s\n", info->id, info->pid, info->ip);
				
				// remove client from the client list
				#if USE_BUILTIN
				shell_rm_client_blt(info->pid, clist);
				#else
				shell_rm_client(info->pid, clist);
				#endif // USE_BUILTIN
				break;

			case CLIENT_DISCON_SUCCESS:
				sprintf(log_msg, "client %d(%d) disconnected\n", info->id, info->pid);

				// remove client from the client list
				#if USE_BUILTIN
				shell_rm_client_blt(info->pid, clist);
				#else
				shell_rm_client(info->pid, clist);
				#endif // USE_BUILTIN
				break;

			case CLIENT_DATA_READY:
				sprintf(log_msg, "client %d(%d) data received: %s\n", info->id, info->pid, info->data);
				break;
			
			case CLIENT_DATA_MISSING:
				sprintf(log_msg, "client %d(%d) is not receiving any data\n", info->id, info->pid);
				break;
			
			default:
				fprintf(stderr, "error: unknown enum value\n");
				break;
		}
		// log a message
		shell_log_write(log_fd, log_msg);
		fprintf(stdout, "%s", log_msg);
	}
}

// handles request from the user: what to do
void shell_handle_request(char *pipefd, int *flag, struct client_list *clist){

	fprintf(stdout, "What to do:\n");
	fprintf(stdout, "1. Terminate the shell\n");
	fprintf(stdout, "2. Connect to sensor\n");
	fprintf(stdout, "3. Disconnect from sensor\n");
	fprintf(stdout, "4. Show clients\n");
	fprintf(stdout, "5. Close the menu\n");

	int option = shell_read_option();
	printf("option :%d\n", option);

	// terminate the shell
	if(option == 1){
	#if USE_BUILTIN
		shell_terminate_blt(flag, clist);
	#else
		shell_terminate(flag, clist);
	#endif // USE_BUILTIN
	}

	// connect to a sensor
	else if(option == 2){
		if(clist->full){
			fprintf(stdout, "no more room for clients, list is full\n");
			return;
		}
	#if USE_BUILTIN
		shell_connect_sensor_blt(pipefd);
	#else
		shell_connect_sensor(pipefd);
	#endif // USE_BUILTIN
	}

	// disconnect from the sensor
	else if(option == 3){
		if(clist->slots == 0){
			fprintf(stdout, "no connected clients to disconnect\n");
			return;
		}
	#if USE_BUILTIN
		shell_disconnect_sensor_blt(clist);
	#else
		shell_disconnect_sensor(clist);
	#endif
	}

	// show clients
	else if(option == 4){
	#if USE_BUILTIN
		shell_show_clients_blt(clist);
	#else
		shell_show_clients(clist);
	#endif // USE_BUILTIN
	
	}
	// exit from the menu
	else if(option == 5) return;

	// undefined option: do nothing
	else{
		fprintf(stdout, "error: invalid option\n");
	}
}

// opens or creates a new log file
int shell_log_open(const char *path){
	
	int fd = open(path, O_CREAT | O_APPEND | O_WRONLY, S_IRUSR | S_IWUSR);
	if(fd < 0){
		fprintf(stderr, "error: opening log file failed(%d) --- %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	return fd;
}

// logs a message to a log file
void shell_log_write(int fd, const char msg[]){

	int ret = write(fd, msg, strlen(msg));
	if(ret < 0){
		fprintf(stderr, "error: writing to a log file failed(%d) --- %s\n", errno, strerror(errno));
		// close log file only if it is actually open
		if(errno != EBADF) close(fd);

		exit(EXIT_FAILURE);
	}
}


