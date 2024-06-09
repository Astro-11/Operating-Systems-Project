#include "SocketUtilities.h"

#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

int receive_signal(int socket, int * buffer) {
    int signal = 0;
    int returnValue = recv(socket, &signal, SIGNAL_LENGTH, 0);
    signal = ntohl(signal);
    *buffer = signal;
    return returnValue;
}

int send_signal(int socket, int * buffer) {
    int signal = htonl(*buffer);
    return send(socket, &signal, SIGNAL_LENGTH, 0);
}

int receiveMsg(int socket, char * buffer) {
    return recv(socket, buffer, MSG_LENGHT, 0);
}

int sendMsg(int socket, char * buffer) {
    return send(socket, buffer, MSG_LENGHT, 0);
}

int receiveDataEntry(int socket, dataEntry * buffer) {
    return recv(socket, buffer, DATAENTRY_LENGHT, 0);
}

int sendDataEntry(int socket, dataEntry * buffer) {
    return send(socket, buffer, DATAENTRY_LENGHT, 0);
}

int create_server_socket(int port, void (*f)(int errorCode, char* errorMessage)) {
    int server_socket;
    struct sockaddr_in server_address;

    // Create the socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
        (*f)(0, "Socket creation failed");

    // Configure the server address structure
    memset(&server_address, 0, sizeof(server_address)); // helps to avoid weird bugs with uninitialized memory
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY

    
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
        (*f)(0, "Bind failed");

    return server_socket;
}

int create_client_socket(const char *server_ip, int port, void (*f)(int errorCode, char* errorMessage)) {
    int client_socket;
    struct sockaddr_in server_address;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
        (*f)(0, "Socket creation failed");

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0)
        (*f)(0, "Invalid address or address not supported");

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
        (*f)(0, "Connection failed");

    return client_socket;
}

int accept_client_connection(int server_socket, void (*f)(int errorCode, char* errorMessage)) {
    struct sockaddr_in client_address;
    socklen_t client_addr_len = sizeof(client_address);

    int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_addr_len);
    if (client_socket < 0)
        (*f)(0, "Accept failed");

    return client_socket;
}

void login(int client_socket, char password[], char response[]){
    int admin = ADMIN;
    send_signal(client_socket, &admin);

    sendMsg(client_socket, password);

    receiveMsg(client_socket, response);
}

void no_login(int client_socket){
    int base = BASE;
    send_signal(client_socket, &base);
}

int check_socket(int socket) {
    // int result = send(socket, NULL, 0, MSG_NOSIGNAL);
    // // Connection is closed
    // if (result == -1 && errno == EPIPE)
    //         return 1;

    // return 0;
    int result = send(socket, NULL, 0, MSG_NOSIGNAL);

    if (result == -1) {
        // An error occurred, check the errno
        if (errno == EPIPE || errno == ECONNRESET) {
            // Connection is closed
            return 1;
        }
    }
    // If result is 0 or positive, connection is open
    return 0;
}