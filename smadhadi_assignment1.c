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


#include "../include/global.h"
#include "../include/logger.h"

struct client *logged_in_client_list_head;
int server_descriptor_global = -1;
bool list_await = false;
char *logged_in_client_list;
void server(int socket_descriptor, char* port);
void client(int socket_descriptor, char* port);
void print(char *action, char *message, bool status);
char* get_host_ip_address();
void author_cmd();
void port_cmd(char *port);
void ip_cmd();
int login_cmd(fd_set *master_list, int cmax);
void send_cmd(char input_dup[]);
void receive_cmd(int i);
void receive_cmd_server(int client_descriptor, fd_set *master_list);
void logout_cmd(fd_set *master_list);
void exit_cmd(fd_set *master_list);
void list_cmd_server();
void handle_server_msg(int client_descriptor, char *received_message);
void handle_client_msg(char *action, char *msg);
void send_list(int client_descriptor);
void print_list(char *buffer);
char* remaining_msg();
void req_for_list();
void send_port(char* port);
void print_success(char *action);
void print_error(char *action);
void print_end(char *action);
void print_statistics();
int send_prevalidations(char *msg);
int check_in_logged_in_list(char *ip);
int ip_validation(char* ip);
int port_validation(char* ip);
int valid_part(char *ipv);

struct client* create_client_node(int descriptor, char *hostname, char *ip_address, char *port,  bool is_loggedin);
void insert_to_logged_in_clients(struct client* new_client);
struct client* find_client_node_by_descriptor(int descriptor);
struct client* find_client_node_by_ip(char* ip);


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
	* Refresh - Done
	* Broadcast
	* Statistics
	* Block & Unblock
	* Ip Validation - 3
	* Exception handling for all cases - 2
	* Counter in select call - done
	* Store messages of logged out clients, Max of 100 Buffered messages.
	* List - done
	* rename cmax
	* send message properly - done
	* LOGOUT - EXIT impl ******
	* send port
 **/

 struct client {
	 int descriptor;
	 char hostname[30];
	 char ip[30];
	 char port[10];
	 int num_msgs_recv;
	 int num_msgs_sent;
	 bool is_loggedin;
	 struct client *next;
 };

int main(int argc, char **argv) {
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/*Clear LOGFILE*/
	fclose(fopen(LOGFILE, "w"));

	/*Start Here*/

	if(argc != 3) {
			cse4589_print_and_log("Please enter c/s & port number\n");
			exit(1);
	}

	int host_socket_descriptor;
	struct sockaddr_in host_addr;

	if((host_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		cse4589_print_and_log("Socket creation failed...!!!\n");
		exit(1);
	}

  host_addr.sin_family = AF_INET;
	host_addr.sin_port = htons(atoi(argv[2]));
	host_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(host_socket_descriptor, (struct sockaddr *)&host_addr, sizeof(host_addr)) < 0) {
		cse4589_print_and_log("Bind failed...!!!\n");
		exit(1);
	}

	if(listen(host_socket_descriptor, 4) < 0) {
		cse4589_print_and_log("Listen failed...!!!\n");
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
	// cse4589_print_and_log("I am server...!!! \n");
	fd_set master_list, watch_list;
	FD_ZERO(&master_list);
	FD_ZERO(&watch_list);
	FD_SET(0, &master_list);
	FD_SET(host_socket_descriptor, &master_list);
	int cmax = host_socket_descriptor;
	//int client_descriptor_global;
	while(1) {
    fflush(stdin);
    fflush(stdout);
		watch_list = master_list;
		if(select(cmax+1, &watch_list, NULL, NULL, NULL) < 0) {
			// cse4589_print_and_log("%s\n", "Error occured");
		} else {
			for(int i = 0; i <= cmax+1; i++) {
				if(FD_ISSET(i, &watch_list)) {
					if(i == 0) {
						char input[256];
						if(fgets(input, 256, stdin) == 0) {
							// cse4589_print_and_log("%s\n", "No Input");
							break;
						} else {
							// cse4589_print_and_log("Given command :: %s\n", input);
							char *action = strtok(input, " ");
							if (strcmp(action, "AUTHOR\n") == 0) {
								author_cmd();
							} else if (strcmp(action, "PORT\n") == 0) {
								port_cmd(port);
							} else if (strcmp(action, "IP\n") == 0) {
								ip_cmd();
							} else if(strcmp(action, "LIST\n") == 0) {
								list_cmd_server();
							} else if(strcmp(action, "STATISTICS\n") == 0) {
								print_statistics();
							} else if(strcmp(action, "EXIT\n") == 0) {
								//exit_cmd();
								print_success("EXIT");
								print_end("EXIT");
								exit(0);
							}
						}
					} else if (i == host_socket_descriptor) {
						// accept connections
						// cse4589_print_and_log("%s\n", "Accepting new connections...");
						struct sockaddr_in client_addr;
						int caddr_len = sizeof(client_addr);
						int client_descriptor = accept(host_socket_descriptor, (struct sockaddr *)&client_addr, (socklen_t*)&caddr_len);
						//client_descriptor_global = client_descriptor;
						FD_SET(client_descriptor, &master_list);
						// cse4589_print_and_log("%s\n", "Server accepted client connection...!!!");
						// char ipv4[20];
            // cse4589_print_and_log("cl desc %d\n", client_descriptor);
            char *ipv4 = (char *)malloc(sizeof(char)*30);
						inet_ntop(AF_INET, &(client_addr.sin_addr), ipv4, INET_ADDRSTRLEN);
						// cse4589_print_and_log("Client IP : %s, Port : %d, %d\n", ipv4, client_addr.sin_port, ntohs(client_addr.sin_port));
						if(client_descriptor > cmax)
							cmax = client_descriptor;
						struct hostent *host = gethostbyaddr(&(client_addr.sin_addr), sizeof(client_addr.sin_addr), AF_INET);
					  // cse4589_print_and_log("Host name: %s\n", host->h_name);
            struct client* cl = find_client_node_by_ip(ipv4);
            if(cl== NULL) {
              // cse4589_print_and_log("%s\n", "NEW");
                struct client* new_client = create_client_node(client_descriptor, host->h_name ,ipv4, "8080", true);
						    insert_to_logged_in_clients(new_client);
            } else {
              // cse4589_print_and_log("%s\n", "OLD");
              cl->is_loggedin = true;
              cl->client_descriptor=client_descriptor;
              // cse4589_print_and_log("%s\n", "OLD DONE");
            }
            free(ipv4);
						//send_list(client_descriptor);
					} else {
						// receive, parse and send.
						receive_cmd_server(i, &master_list);
					}
				}
			}
      fflush(stdin);
      fflush(stdout);
		}
	}
}

void client(int host_socket_descriptor, char* port) {
	fd_set master_list, watch_list;
	FD_ZERO(&master_list);
	FD_ZERO(&watch_list);
	FD_SET(0, &master_list);
  FD_SET(host_socket_descriptor, &master_list);
  logged_in_client_list = (char *)malloc(sizeof(char)*1024);
	int cmax = 0;
	while(1) {
    fflush(stdin);
    fflush(stdout);
		watch_list = master_list;
		if(select(cmax+1, &watch_list, NULL, NULL, NULL) < 0) {
			// cse4589_print_and_log("%s\n", "Error occured");
		} else {
				for(int i = 0; i <= cmax+1; i++) {
					if(FD_ISSET(i, &watch_list)) {
						if(i == 0) {
							char input[256];
							if(fgets(input, 256, stdin) == 0) {
								//cse4589_print_and_log("%s\n", "No Input");
								break;
							} else {
								//cse4589_print_and_log("Given command :: %s\n", input);
								char input_dup[256];
								strcpy(input_dup, input);
								// cse4589_print_and_log("INPUT_DUP IS %s\n", input_dup);
								char *action = strtok(input, " ");
								if (strcmp(action, "AUTHOR\n") == 0) {
									author_cmd();
								} else if (strcmp(action, "PORT\n") == 0) {
									port_cmd(port);
								} else if (strcmp(action, "IP\n") == 0) {
									ip_cmd();
								} else if (strcmp(action, "LOGIN") == 0) {
                  if(server_descriptor_global == -1) {
                    int temp = login_cmd(&master_list, cmax);
                    if(temp != -1) {
                      if(temp > cmax)
                        cmax = temp;
                      send_port(port);
                    }
                  } else {
                    print_success("LOGIN");
                    print_end("LOGIN");
                  }
								} else if (strcmp(action, "SEND") == 0) {
									if(server_descriptor_global == -1) {
										print("SEND", " " ,0);
									} else {
										send_cmd(input_dup);
									}
								} else if(strcmp(action, "LIST\n") == 0) {
									if(server_descriptor_global == -1) {
										print("LIST", " " ,0);
									} else {
										char list_temp[256];
										strcpy(list_temp, logged_in_client_list);
										print_success("LIST");
										print_list(list_temp);
										print_end("LIST");
									}
								} else if(strcmp(action, "REFRESH\n") == 0) {
									if(server_descriptor_global == -1) {
										print("REFRESH", " " ,0);
									} else {
										req_for_list();
										list_await = true;
									}
								} else if(strcmp(action, "LOGOUT\n") == 0) {
									if(server_descriptor_global == -1) {
										print("LOGOUT", " " ,0);
									} else {
										logout_cmd(&master_list);
									}
								} else if(strcmp(action, "EXIT\n") == 0) {
									exit_cmd(&master_list);
								}

							}
						} else if (i == server_descriptor_global){
								// Handle recv returning <0, i..e. recv failed.
								receive_cmd(i);
						}
            fflush(stdin);
            fflush(stdout);
					}
				}
		}
	}
}

void print_statistics() {
	int list_id=1;
	struct client* node = logged_in_client_list_head;
	while(node!=NULL) {
		cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", list_id++, node->hostname, node->num_msgs_sent, node->num_msgs_recv, (node->is_loggedin)?"logged-in":"logged-out");
		node=node->next;
	}
}

void send_port(char* port) {
	char message[20];
	strcat(message, "PORT ");
	strcat(message, port);
	if(send(server_descriptor_global, message, sizeof(message), 0) == -1){
		// cse4589_print_and_log("send failed\n");
	} else {
		// cse4589_print_and_log("%s\n", "Port sent..!!!");
	}
}

void req_for_list() {
	char* action = "LIST\n";
	if(server_descriptor_global == -1) {
		//cse4589_print_and_log("%s\n", "You are not logged in...!!!");
		print_error("REFRESH");
		print_end("REFRESH");
	} else {
		if(send(server_descriptor_global, action, strlen(action), 0) == -1) {
			//cse4589_print_and_log("%s\n", "Send Failed...!!!");
			print_error("REFRESH");
			print_end("REFRESH");
		} else {
			//cse4589_print_and_log("%s\n", "Message sent...!!!");
		}
	}
}

void receive_cmd_server(int client_descriptor, fd_set *master_list) {
	// cse4589_print_and_log("%s\n", "Receiving messages...");
	char *received_message = (char *)malloc(sizeof(char)*1024);
	int recv_ret = recv(client_descriptor, received_message, 1024, 0);
  fflush(stdout);
  fflush(stdin);
	// cse4589_print_and_log("Received message : %s\n", received_message);
	if(recv_ret < 0) {
			// cse4589_print_and_log("%s\n", "receive failed");
	} else if(recv_ret == 0) {
		// Loggedout
		// change is_loggedin to 0;
		FD_CLR(client_descriptor, master_list);
		find_client_node_by_descriptor(client_descriptor)->is_loggedin = false;
		// cse4589_print_and_log("client %d logged out\n", client_descriptor);
	} else {
    received_message[recv_ret] = '\0';
		handle_server_msg(client_descriptor, received_message);
	}
	free(received_message);
}

void handle_server_msg(int client_descriptor, char *received_message) {
	//char *action = strtok(received_message, " ");
	// cse4589_print_and_log("action is : %s\n", received_message);
	if(strcmp(received_message, "LIST\n") == 0) {
		// cse4589_print_and_log("%s\n", "SENDING LIST");
		send_list(client_descriptor);
	} else if(strcmp(received_message, "REFRESH\n") == 0) {
		send_list(client_descriptor);
	//}
	// else if(strcmp(action, "BROADCAST") == 0) {
	//
	// } else if(strcmp(action, "BLOCK") == 0) {
	//
	// } else if(strcmp(action, "UNBLOCK") == 0) {
	//
	} else {
		// Send to another client
		char recv_msg_dup[256];
		strcpy(recv_msg_dup, received_message);
		char *action = strtok(received_message, " ");
		// cse4589_print_and_log("%s, %d\n", action, strcmp(action, "PORT"));
		if(strcmp(action, "PORT") == 0) {
			struct client* cl = find_client_node_by_descriptor(client_descriptor);
      // if(cl==NULL)
      // cse4589_print_and_log("%s\n", "cl is null");
			char* port_num = strtok(NULL, " ");
			strcpy(cl->port, port_num);
			send_list(client_descriptor);
		} else {
			// char *dest_ip = strtok(NULL, " ");;
			// char *msg = remaining_msg();
			char msg[256];
			int i = 0, len = strlen(action);
			while(recv_msg_dup[i+len] != '\0') {
				msg[i] = recv_msg_dup[i+len];
				i++;
			}
			msg[i]='\0';
			struct client* dest = find_client_node_by_ip(action);
      if(!dest->is_loggedin) {
        print_error("RELAYED");
				print_end("RELAYED");
        return;
      }
			int dest_cli_desc = dest->descriptor;
			struct client* cl = find_client_node_by_descriptor(client_descriptor);
			cl->num_msgs_sent++;
			if(send(dest_cli_desc, recv_msg_dup, strlen(recv_msg_dup), 0) < 0) {
				// cse4589_print_and_log("%s\n", "Delivery to client failed\n");
				print_error("RELAYED");
				print_end("RELAYED");
			} else {
				// cse4589_print_and_log("%s\n", "Delivered to client...");
				dest->num_msgs_recv++;
				print_success("RELAYED");
				cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", action, dest->ip, msg);
				print_end("RELAYED");
			}
			// free(msg);
		}
	}
}

void send_list(int client_descriptor) {
	// cse4589_print_and_log("%s\n", "Sending list to server");
	struct client* temp = logged_in_client_list_head;
	// char buffer[1024];
	char *buffer = (char *)malloc(sizeof(char)*1024);
	int i = 1;
	strcat(buffer, "LIST ");
	while(temp != NULL){
		if(temp->is_loggedin) {
			if(i == 1){
				strcat(buffer,"1");
			} else if(i == 2){
				strcat(buffer,"2");
			} else if(i == 3){
				strcat(buffer,"3");
			} else if(i == 4){
				strcat(buffer,"4");
			}
			strcat(buffer," ");
			strcat(buffer,temp->hostname);
			strcat(buffer," ");
			strcat(buffer,temp->ip);
			strcat(buffer," ");
			strcat(buffer,temp->port);
			strcat(buffer," ");
			i++;
		}
		temp=temp->next;
	}
	// cse4589_print_and_log("sending list : %s\n", buffer);
	int srv = send(client_descriptor, buffer, strlen(buffer), 0);
	free(buffer);
	if(srv == -1){
		// cse4589_print_and_log("send failed\n");
	} else if (srv == 0) {
		// cse4589_print_and_log("%s\n", "nothing sent");
	} else {
		// cse4589_print_and_log("%s\n", "List sent...!!!");
	}
}

/* To print list in client after receiving data from server*/
void print_list(char *buffer) {
	char *list_id, *hostname, *ip_addr, *port_num;
	int i = 1;
	do {
		if(i == 1) {
		   list_id = strtok(buffer, " ");
		} else {
		   list_id = strtok(NULL, " ");
		}

		if(list_id == NULL)
			break;

		hostname = strtok(NULL, " ");
		ip_addr = strtok(NULL, " ");
		port_num = strtok(NULL, " ");
		cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", atoi(list_id), hostname, ip_addr, atoi(port_num));
		i++;
	} while(buffer != NULL);
}

void list_cmd_server() {
  print_success("LIST");
	struct client *temp = logged_in_client_list_head;
	int list_id = 1;
	while(temp != NULL) {
		if(temp->is_loggedin)
		cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", list_id++, temp->hostname, temp->ip, atoi(temp->port));
		temp=temp->next;
	}
  print_end("LIST");
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
  if(server_ip == NULL) {
    print_error("LOGIN");
		print_end("LOGIN");
    return -1;
  }
	char *server_port = strtok(strtok(NULL, " "), "\n");
  if(server_port == NULL) {
    print_error("LOGIN");
		print_end("LOGIN");
    return -1;
  }
	// validations
	char server_ip_dup[256], server_port_dup[256];
	strcpy(server_ip_dup, server_ip);
	strcpy(server_port_dup, server_port);
	// cse4589_print_and_log("server port:%s\n", server_port_dup);
	if(!(ip_validation(server_ip_dup) && port_validation(server_port_dup))) {
		print_error("LOGIN");
		print_end("LOGIN");
		return -1;
	}
	int server_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	server_descriptor_global = server_descriptor;
	FD_SET(server_descriptor, master_list);
	if(server_descriptor > cmax)
		cmax = server_descriptor;
	//cse4589_print_and_log("Logging in now... server Ip : %s & server port %s\n", server_ip, server_port);
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(server_port));
	inet_pton(AF_INET, server_ip, &(server_addr.sin_addr));
	if (connect(server_descriptor, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		// cse4589_print_and_log("%s\n", "Connect Failed...!!!!");
		print_error("LOGIN");
		print_end("LOGIN");
    return -1;
	} else {
		print_success("LOGIN");
		print_end("LOGIN");
		// cse4589_print_and_log("%s\n", "Connected successfully");
	}
	return cmax;
}

void send_cmd(char input_dup[]) {
	// Send message properly
	// char *token;
	// int flag=0;
	// char *msg = (char *)malloc(sizeof(char)*1024);
	// do {
	// 	token = strtok(NULL, " ");
	// 	if(token == NULL)
	// 		break;
	// 	if(flag) {
	// 		strcat(msg, " ");
	// 	}
	// 	strcat(msg, token);
	// 	flag = 1;
	// } while(token != NULL);
	// cse4589_print_and_log("input_dup is %s, %d\n", input_dup, strlen(input_dup));
	char msg[256];
	int i = 0;
	while(input_dup[i+5] != '\0') {
		msg[i] = input_dup[i+5];
		i++;
	}
	msg[i-1] = '\0';
	// cse4589_print_and_log("Message to be sent is : %s, len : %d\n", msg, strlen(msg));
	// char *msg_dup = (char *)malloc(sizeof(char)*1024);
  char msg_dup[strlen(msg)];
  strcpy(msg_dup, msg);

	if(server_descriptor_global == -1) {
		// cse4589_print_and_log("%s\n", "You are not logged in...!!!");
		print_error("SEND");
		print_end("SEND");
	} else {
		if(send_prevalidations(msg_dup)) {
			// cse4589_print_and_log("Message to be sent is : %s, len : %lu\n", msg, strlen(msg));
			if(send(server_descriptor_global, msg, strlen(msg), 0) == -1) {
					// cse4589_print_and_log("%s\n", "Send Failed...!!!");
					print_error("SEND");
					print_end("SEND");
			} else {
				// cse4589_print_and_log("%s\n", "Message sent...!!!");
				print_success("SEND");
				print_end("SEND");
			}
		} else {
			print_error("SEND");
			print_end("SEND");
		}
	}

	// free(msg);
	// free(msg_dup);
}

int send_prevalidations(char *msg) {
	// cse4589_print_and_log("ENTERED PREVALIDATIONS OF SEND\n");
	char *ip = strtok(msg, " ");
	// char *ip_dup = (char *)malloc(sizeof(char)*256);
  char ip_dup[strlen(ip)];
	strcpy(ip_dup, ip);
	// cse4589_print_and_log("IP is : %s\n", ip);
	return ip_validation(ip) && check_in_logged_in_list(ip_dup);
}

int valid_part(char *ipv) {

	int size = strlen(ipv);
	if(size > 3) return 0;
	// digit check
	for(int i=0;i<size;i++){
		if(!(ipv[i] >= '0' && ipv[i] <= '9')){
			return 0;
		}
	}
	// value check
	if(!(atoi(ipv) >= 0 && atoi(ipv) <= 255)){
		return 0;
	}

	return 1;
}

int ip_validation(char* ip) {
	int dots_count = 0;
    if(ip == NULL) return 0;
	char *ipv = strtok(ip,".");
	while(ipv != NULL){
	  if(valid_part(ipv)){
	  	ipv = strtok(NULL,".");
	  	if(ipv != NULL){
	  	  ++dots_count;
	  	}
	  } else{
	  	return 0;
	  }
	}
	if(dots_count != 3){
		return 0;
	}
	// cse4589_print_and_log("%s\n", "IP VALIDATION PASSED");
	return 1;
}

int port_validation(char *port) {
	// cse4589_print_and_log("port is %d\n", atoi(port));
	int size = strlen(port);
	if(size > 5) return 0;
	// digit check
	for(int i=0;i<size;i++){
		if(!(port[i] >= '0' && port[i] <= '9')){
			// cse4589_print_and_log("FAILED IN DIGIT CHECK :%c\n", port[i]);
			return 0;
		}
	}
	if(!(atoi(port) >= 0 && atoi(port) <= 65535)){
		return 0;
	}
	return 1;
}

int check_in_logged_in_list(char *ip) {
	char logged_in_client_list_dup[256];
	strcpy(logged_in_client_list_dup, logged_in_client_list);
	char *list_id, *hostname, *ip_addr, *port_num;
	int i = 1;
	do {
		// cse4589_print_and_log("%s\n", "CHECK START ");
		if(i == 1) {
		   list_id = strtok(logged_in_client_list_dup, " ");
		} else {
		   list_id = strtok(NULL, " ");
		}

		if(list_id == NULL)
			break;

		hostname = strtok(NULL, " ");
		ip_addr = strtok(NULL, " ");

			// cse4589_print_and_log("%s : %s\n", ip_addr, ip);
		if(strcmp(ip_addr, ip) == 0) {
			// cse4589_print_and_log("%s : %s\n", ip_addr, ip);
			// free(ip);
			return 1;
		}
		port_num = strtok(NULL, " ");
		i++;
	} while(1);
	// cse4589_print_and_log("Not present in logged in client list");
	// free(ip);
	return 0;
	// return 1;
}



void logout_cmd(fd_set *master_list) {
	print_success("LOGOUT");
	FD_CLR(server_descriptor_global, master_list);
	close(server_descriptor_global);
  server_descriptor_global = -1;
	print_end("LOGOUT");
//	cse4589_print_and_log("%s\n", "logged out");
}

void exit_cmd(fd_set *master_list) {
	print_success("EXIT");
	FD_CLR(server_descriptor_global, master_list);
	close(server_descriptor_global);
	print_end("EXIT");
//	cse4589_print_and_log("%s\n", "Closing the application...!!!");
	exit(0);
}

void receive_cmd(int i) {
	// cse4589_print_and_log("%s\n", "Receiving messages...");
	char *received_message = (char *)malloc(sizeof(char)*1024);
  int recv_ret = recv(i, received_message, 1024, 0);
	if(recv_ret == 0) {
			server_descriptor_global = -1;
			//cse4589_print_and_log("%s\n", "Connection Terminated");
	} else {
		// cse4589_print_and_log("rec msg : %s\n", received_message);
    received_message[recv_ret]='\0';
    char received_message_dup[400];
    strcpy(received_message_dup, received_message);
		char *action = strtok(received_message, " ");
		//char* rem_msg = remaining_msg();
    char msg[400];
  	int i = 0;
  	while(received_message_dup[i+strlen(action)] != '\0') {
  		msg[i] = received_message_dup[i+strlen(action)];
  		i++;
  	}
    msg[i]='\0';
		//cse4589_print_and_log("%s\n", rem_msg);
		handle_client_msg(action, msg);
	}
}

void handle_client_msg(char *action, char *msg) {
	if(strcmp(action, "LIST") == 0) {
    free(logged_in_client_list);
    logged_in_client_list = (char *)malloc(sizeof(char)*strlen(msg));
		strcpy(logged_in_client_list, msg);
		if(list_await) {
			list_await = false;
			print_success("REFRESH");
			// print_list(msg);
			print_end("REFRESH");
		}
	} else {
		print_success("RECEIVED");
		cse4589_print_and_log("msg from:%s\n[msg]:%s\n", action, msg);
		print_end("RECEIVED");
	}
}

char* remaining_msg() {
	char *token;
	int flag=0;
	char *msg = (char *)malloc(sizeof(char)*1024);
	do {
		token = strtok(NULL, " ");
		if(token == NULL)
			break;
		if(flag) {
			strcat(msg, " ");
		}
		strcat(msg, token);
		flag = 1;
	} while(token != NULL);
	return msg;
}

void print(char *action, char *message, bool status) {
	if(status) {
		cse4589_print_and_log("[%s:SUCCESS]\n", action);
		cse4589_print_and_log("%s\n", message);
	} else {
		cse4589_print_and_log("[%s:ERROR]\n", action);
	}
	cse4589_print_and_log("[%s:END]\n", action);
}

void print_success(char *action) {
	cse4589_print_and_log("[%s:SUCCESS]\n", action);
}

void print_error(char *action) {
	cse4589_print_and_log("[%s:ERROR]\n", action);
}

void print_end(char *action) {
	cse4589_print_and_log("[%s:END]\n", action);
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
		int len = sizeof(getip_addr);
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
		while(curr != NULL && (atoi(curr->port) < atoi(new_client->port))) {
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

struct client* find_client_node_by_ip(char* ip) {
	struct client *temp = logged_in_client_list_head;
	while (temp != NULL && (strcmp(temp->ip, ip) != 0)) {
		temp = temp->next;
	}
	return temp;
}

struct client* create_client_node(int descriptor, char *hostname, char *ip_address, char *port, bool is_loggedin) {
	struct client* new_client = (struct client *)malloc(sizeof(struct client));
	new_client->descriptor = descriptor;
	strcpy(new_client->hostname, hostname);
	strcpy(new_client->ip, ip_address);
	//strcpy(new_client->port, port);
	// new_client->port = port;
	new_client->next = NULL;
	new_client->is_loggedin = is_loggedin;
	return new_client;
}
