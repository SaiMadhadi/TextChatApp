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


//#include "../include/global.h"
//#include "../include/logger.h"

int server_descriptor_global = -1;
void server(int socket_descriptor, int port);
void client(int socket_descriptor, int port);

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
	* List
 **/
int main(int argc, char **argv)
{
	/*Init. Logger*/
	//cse4589_init_log(argv[2]);

	/*Clear LOGFILE*/
	//fclose(fopen(LOGFILE, "w"));

	/*Start Here*/
	if(argc != 3) {
			printf("Please enter c/s & port number\n");
	} else {
	//	printf("Given port is : %d", htons(atoi(argv[2])));
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
		server(host_socket_descriptor, atoi(argv[2]));
	} else if (*argv[1] == 'c') {
		client(host_socket_descriptor, atoi(argv[2]));
	} else {
		exit(1);
	}

	return 0;
}

void server(int host_socket_descriptor, int port) {
	printf("I am in server %d , %d\n", host_socket_descriptor, port);
	fd_set master_list, watch_list;
	FD_ZERO(&master_list);
	FD_ZERO(&watch_list);
	FD_SET(0, &master_list);
	FD_SET(host_socket_descriptor, &master_list);

	while(1) {
		watch_list = master_list;
		int cmax = host_socket_descriptor;
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
								puts(action);
								if (strcmp(action, "AUTHOR\n") == 0) {
									printf("%s\n", "Hello I am author!!!");
								} else if (strcmp(action, "PORT\n") == 0) {
									printf("PORT IS : %d\n", port);
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
							if(client_descriptor > cmax)
								cmax=client_descriptor;
							printf("current client desc : %d, cmax : %d\n", client_descriptor, cmax);
						} else {
							// receive, parse and send.
							printf("%s\n", "Receiving messages...");
							char *received_message;
							recv(i, received_message, 100, 0);
							printf("Received message : %s\n", received_message);
						}
					}
				}
		}
	}
}

void client(int host_socket_descriptor, int port) {
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
								puts(action);
								if (strcmp(action, "AUTHOR\n") == 0) {
									printf("%s\n", "Hello I am author!!!");
								} else if (strcmp(action, "PORT\n") == 0) {
									printf("PORT IS : %d\n", port);
								} else if (strcmp(action, "LOGIN") == 0) {
									char *server_ip = strtok(NULL, " ");
									char *server_port = strtok(NULL, " ");
									int server_descriptor = socket(AF_INET, SOCK_STREAM, 0);
									server_descriptor_global = server_descriptor;
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
								} else if (strcmp(action, "SEND") == 0) {
									printf("%s\n", "Sending messages...!!!");
									if(server_descriptor_global == -1) {
										printf("%s\n", "You are not logged in...!!!");
									} else {
										if(send(server_descriptor_global, input, sizeof(input), 0) == -1) {
												printf("%s\n", "Send Failed");
										} else {
											printf("%s\n", "Message sent");
										}
									}
								} else if(strcmp(action, "LOGOUT\n") == 0) {
									printf("%s\n", "Closing connection...!!!");
									FD_CLR(server_descriptor_global, &master_list);
									close(server_descriptor_global);
								} else if(strcmp(action, "EXIT\n") == 0) {
									printf("%s\n", "Closing the application...!!!");
									exit(0);
								}
							}
						} else {
								// Handle recv returning <0, i..e. recv failed.
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
					}
				}
		}
	}
}
