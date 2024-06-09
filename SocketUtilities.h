#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include "CommonDefines.h"

int receive_signal(int socket, int * buffer);
int send_signal(int socket, int * buffer);

int receiveMsg(int socket, char * buffer);
int sendMsg(int socket, char * buffer);

int receiveDataEntry(int socket, dataEntry * buffer);
int sendDataEntry(int socket, dataEntry * buffer);

int create_server_socket(int port, void (*f)(int errorCode, char* errorMessage));
int create_client_socket(const char *server_ip, int port, void (*f)(int errorCode, char* errorMessage));
int accept_client_connection(int server_socket, void (*f)(int errorCode, char* errorMessage));

void login(int client_socket, char password[], char response[]);
void no_login(int client_socket);

int check_socket(int socket); 

#endif 