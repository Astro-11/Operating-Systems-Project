#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include "DatabaseHandler.h"

#define SERVER_IP "127.0.0.1"
#define PORT 9890
#define SIGNAL_LENGTH 4
#define MSG_LENGHT 256

#define ADMIN 0
#define BASE 1

#define ACCESS_GRANTED "ACCESS_GRANTED"
#define ACCESS_DENIED "ACCESS_DENIED"

typedef enum{
    SEARCH_DB = 1,
    ADD_RECORD,
    REMOVE_RECORD,
    EDIT_RECORD,
    LOGIN,
} serverRequest;


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

char* login(int client_socket, char* password);
void no_login(int client_socket);

#endif // SOCKET_UTILS_H