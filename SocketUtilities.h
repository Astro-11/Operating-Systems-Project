#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include "DatabaseHandler.h"

#define SERVER_IP "127.0.0.1"
#define PORT 9090
#define SIGNAL_LENGTH 4
#define MSG_LENGHT 256

int receive_signal(int socket, int * buffer);
int send_signal(int socket, char * buffer);

int receiveMsg(int socket, char * buffer);
int sendMsg(int socket, char * buffer);

int receiveDataEntry(int socket, dataEntry * buffer);
int sendDataEntry(int socket, dataEntry * buffer);

int create_server_socket(int port);
int create_client_socket(const char *server_ip, int port);
int accept_client_connection(int server_socket);
void handle_error(const char *msg);
void login(int client_socket, char* user, char* password);

#endif // SOCKET_UTILS_H