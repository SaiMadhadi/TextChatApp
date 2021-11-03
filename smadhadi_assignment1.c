/**
 * @smadhadi_assignment1
 * @author  Sai Kumar Madhadi <smadhadi@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>


//#include "../include/global.h"
//#include "../include/logger.h"

struct client *logged_in_client_list_head;
int server_descriptor_global = -1;
void server(int socket_descriptor, char* port);
void client(int socket_descriptor, char* port);
void print(char *action, char *message, bool status);
char* get_host_ip_address();
void author_cmd();
void port_cmd(char *port);
void ip_cmd();
int login_cmd();
void send_cmd();
void receive_cmd();
void logout_cmd(fd_set *master_list);
void exit_cmd();
void list_cmd_server();
char* get_host_name_by_ip(char* ip);

struct client* create_client_node(int descriptor, char *hostname, char *ip_address, int port);
void insert_to_logged_in_clients(struct client* new_client);
struct client* find_client_node_by_descriptor(int descriptor);

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */

 /**
  * TODO:
	*
	* Refresh
	* Broadcast
	* Statistics
	* Block & Unblock
	* Ip Validation - 3
	* Exception handling for all cases - 2
	* Counter in select call - 1
	* Store messages of logged out clients, Max of 100 Buffered messages.
	* List
	* rename cmax
	* send message properly
 **/

 struct client {
	 int descriptor;
	 char hostname[30];
	 char ip[30];
	 int port;
	 struct client *next;
 };

int main(int argc, char **argv)
{
	/*Init. Logger*/
	//cse4589_init_log(argv[2]);

	/*Clear LOGFILE*/
	//fclose(fopen(LOGFILE, "w"));

	/*Start Here*/
	if(argc != 3) {
			printf("Please enter c/s & port number\n");
			exit(1);
	}

	int host_socket_descriptor;
	struct sockaddr_in host_addr;

	if((host_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		printf("Socket creation failed...!!!\n");
		exit(1);
	}

  host_addr.sin_family = AF_INET;
	host_addr.sin_port = htons(atoi(argv[2]));
	host_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(host_socket_descriptor, (struct sockaddr *)&host_addr, sizeof(host_addr)) < 0) {
		printf("Bind failed...!!!\n");
		exit(1);
	}

	if(listen(host_socket_descriptor, 4) < 0) {
		printf("Listen failed...!!!\n");
		exit(1);
	}

	if(*argv[1] == 's') {
		server(host_socket_descriptor, argv[2]);
	} else if (*argv[1] == 'c') {
		client(host_socket_descriptor, argv[2]);
	} else {
		exit(1);
	}

	return 0;
}

void server(int host_socket_descriptor, char* port) {
	printf("I am server...!!! \n");
	fd_set master_list, watch_list;
	FD_ZERO(&master_list);
	FD_ZERO(&watch_list);
	FD_SET(0, &master_list);
	FD_SET(host_socket_descriptor, &master_list);
	int cmax = host_socket_descriptor;
	while(1) {
		watch_list = master_list;
		if(select(cmax+1, &watch_list, NULL, NULL, NULL) < 0) {
			printf("%s\n", "Error occured");
		} else {
				for(int i = 0; i <= cmax+1; i++) {
					if(FD_ISSET(i, &watch_list)) {
						if(i == 0) {
							char input[100];
							if(fgets(input,100, stdin) == 0) {
								printf("%s\n", "No Input");
								break;
							} else {
								printf("Given command :: %s\n", input);
								char *action = strtok(input, " ");
								if (strcmp(action, "AUTHOR\n") == 0) {
									author_cmd();
								} else if (strcmp(action, "PORT\n") == 0) {
									port_cmd(port);
								} else if (strcmp(action, "IP\n") == 0) {
									ip_cmd();
								} else if(strcmp(action, "LIST\n") == 0) {
									list_cmd_server();
								} else if(strcmp(action, "EXIT\n") == 0) {
									exit_cmd();
								}
							}
						} else if (i == host_socket_descriptor) {
							// accept connections
							printf("%s\n", "Accepting new connections...");
							struct sockaddr_in client_addr;
							int caddr_len = sizeof(client_addr);
							int client_descriptor = accept(host_socket_descriptor, (struct sockaddr *)&client_addr, (socklen_t*)&caddr_len);
							FD_SET(client_descriptor, &master_list);
							printf("%s\n", "Server accepted client connection...!!!");
							char ipv4[20];
							inet_ntop(AF_INET, &(client_addr.sin_addr), ipv4, INET_ADDRSTRLEN);
							printf("Client IP : %s, Port : %d, %d\n", ipv4, client_addr.sin_port, ntohs(client_addr.sin_port));
							if(client_descriptor > cmax)
								cmax = client_descriptor;
							struct hostent *host = gethostbyaddr(&(client_addr.sin_addr), sizeof(client_addr.sin_addr), AF_INET);
						  printf("Host name: %s\n", host->h_name);
							struct client* new_client = create_client_node(client_descriptor, host->h_name ,ipv4, client_addr.sin_port);
							insert_to_logged_in_clients(new_client);
						} else {
							// receive, parse and send.
							printf("%s\n", "Receiving messages...");
							char received_message[101];
							recv(i, received_message, 100, 0);
							printf("Received message : %s\n", received_message);
						}
					}
				}
		}
	}
}

void client(int host_socket_descriptor, char* port) {
	fd_set master_list, watch_list;
	FD_ZERO(&master_list);
	FD_ZERO(&watch_list);
	FD_SET(0, &master_list);

	while(1) {
		watch_list = master_list;
		int cmax=0;
		if(select(cmax+1, &watch_list, NULL, NULL, NULL) < 0) {
			printf("%s\n", "Error occured");
		} else {
				for(int i = 0; i <= cmax+1; i++) {
					if(FD_ISSET(i, &watch_list)) {
						if(i == 0) {
							char input[100];
							if(fgets(input,100, stdin) == 0) {
								printf("%s\n", "No Input");
								break;
							} else {
								printf("Given command :: %s\n", input);
								char *action = strtok(input, " ");
								if (strcmp(action, "AUTHOR\n") == 0) {
									author_cmd();
								} else if (strcmp(action, "PORT\n") == 0) {
									port_cmd(port);
								} else if (strcmp(action, "IP\n") == 0) {
									ip_cmd();
								} else if (strcmp(action, "LOGIN") == 0) {
									cmax = login_cmd(&master_list, cmax);
								} else if (strcmp(action, "SEND") == 0) {
									send_cmd();
								} else if(strcmp(action, "LOGOUT\n") == 0) {
									logout_cmd(&master_list);
								} else if(strcmp(action, "EXIT\n") == 0) {
									exit_cmd();
								}
							}
						} else {
								// Handle recv returning <0, i..e. recv failed.
								receive_cmd();
						}
					}
				}
		}
	}
}

void list_cmd_server() {
	struct client *temp = logged_in_client_list_head;
	int list_id = 1;
	while(temp != NULL) {
		printf("%-5d%-35s%-20s%-8d\n", list_id++, temp->hostname, temp->ip, temp->port);
		temp=temp->next;
	}
}

void author_cmd() {
	print("AUTHOR",
	 "I, smadhadi, have read and understood the course academic integrity policy.",
	 true);
}

void port_cmd(char *port) {
	char msg[20] = "PORT:";
	strcat(msg, port);
	print("PORT", msg, true);
}

void ip_cmd() {
	char *host_ip;
	if((host_ip = get_host_ip_address()) == NULL) {
		print("IP", "", false);
	} else {
		char msg[30] = "IP:";
		strcat(msg, host_ip);
		print("IP", msg, true);
	}
	free(host_ip);
}

int login_cmd(fd_set *master_list, int cmax) {
	char *server_ip = strtok(NULL, " ");
	char *server_port = strtok(NULL, " ");
	int server_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	server_descriptor_global = server_descriptor;
	FD_SET(server_descriptor, master_list);
	if(server_descriptor > cmax)
		cmax=server_descriptor;
	printf("Logging in now... server Ip : %s & server port %s\n", server_ip, server_port);
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(server_port));
	inet_pton(AF_INET, server_ip, &(server_addr.sin_addr));
	if (connect(server_descriptor, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		printf("%s\n", "Connect Failed...!!!!");
	} else {
		printf("%s\n", "Connected successfully");
	}
	return cmax;
}

void send_cmd() {
	// Send message properly
	char *client_ip = strtok(NULL, " ");
	char *msg = strtok(NULL, " ");
	if(server_descriptor_global == -1) {
		printf("%s\n", "You are not logged in...!!!");
	} else {
		if(send(server_descriptor_global, msg, sizeof(msg), 0) == -1) {
				printf("%s\n", "Send Failed...!!!");
		} else {
			printf("%s\n", "Message sent...!!!");
		}
	}
}

void logout_cmd(fd_set *master_list) {
	FD_CLR(server_descriptor_global, master_list);
	close(server_descriptor_global);
}

void exit_cmd() {
	printf("%s\n", "Closing the application...!!!");
	exit(0);
}

void receive_cmd(int i) {
	char *received_message;
	printf("%s\n", "Receiving messages...");
	if(recv(i, received_message, 100, 0) == 0) {
			server_descriptor_global = -1;
			printf("%s\n", "Connection Terminated");
	} else {
		char *source_ip = strtok(received_message, " ");
		printf("Message received from source : %s is: \n", source_ip);
		printf("%s\n", received_message);
	}
}


void print(char *action, char *message, bool status) {
	if(status) {
		printf("[%s:SUCCESS]\n", action);
		printf("%s\n", message);
	} else {
		printf("[%s:ERROR]\n", action);
	}
	printf("[%s:END]\n", action);
}

char* get_host_ip_address() {
	int getip_sockfd = socket(AF_INET,SOCK_DGRAM, 0);
	struct sockaddr_in getip_addr;
	char *ip = (char *)malloc(sizeof(char)*30);
	if(getip_sockfd < 0){
	  ip = NULL;
	} else {
	  getip_addr.sin_family = AF_INET;
	  getip_addr.sin_port = htons(53);
	  inet_pton(AF_INET, "8.8.8.8", &(getip_addr.sin_addr));
	  if(connect(getip_sockfd, (struct sockaddr *)&getip_addr, sizeof(getip_addr)) < 0){
	    ip = NULL;
    }
		int len=sizeof(getip_addr);
    if(getsockname(getip_sockfd, (struct sockaddr *)&getip_addr, (socklen_t *)&len) == -1) {
    	ip = NULL;
	  }
	  inet_ntop(AF_INET, &(getip_addr.sin_addr), ip, len);
  }
  return ip;
}

// List Implementation


void insert_to_logged_in_clients(struct client* new_client) {
	if(new_client == NULL) {
		return;
	}
	if(logged_in_client_list_head == NULL) {
		logged_in_client_list_head = new_client;
	} else {
		struct client *curr = logged_in_client_list_head, *prev = NULL;
		// checks from node 1
		while(curr != NULL && curr->port < new_client->port) {
			prev = curr;
			curr = curr->next;
		}

		if(curr == NULL) {
			prev->next = new_client;
		} else {
			struct client *temp;
			if(prev != NULL) {
				temp = prev->next;
				prev->next = new_client;
				new_client->next = temp;
			} else {
				prev = new_client;
				new_client->next=curr;
				logged_in_client_list_head = new_client;
			}
		}

	}
}

void remove_from_logged_in_clients(struct client* client_to_be_removed) {

	if(client_to_be_removed == NULL) {
		return;
	}
	if(logged_in_client_list_head == client_to_be_removed) {
		struct client* temp = logged_in_client_list_head;
		logged_in_client_list_head = logged_in_client_list_head->next;
		temp->next = NULL;
		free(temp);
	} else {
		struct client *prev = NULL, *curr = logged_in_client_list_head;
		while (curr != client_to_be_removed) {
			prev = curr;
			curr = curr->next;
		}
		prev->next = curr->next;
		curr->next = NULL;
		free(curr);
	}
}

struct client* find_client_node_by_descriptor(int descriptor) {
	struct client *temp = logged_in_client_list_head;
	while (temp != NULL && temp->descriptor != descriptor) {
		temp = temp->next;
	}
	return temp;
}

struct client* create_client_node(int descriptor, char *hostname, char *ip_address, int port) {
	struct client* new_client = (struct client *)malloc(sizeof(struct client *));
	new_client->descriptor = descriptor;
	strcpy(new_client->hostname, hostname);
	strcpy(new_client->ip, ip_address);
	new_client->port = port;
	new_client->next = NULL;
	return new_client;
}
