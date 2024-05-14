#include <sys/socket.h>
#include "DatabaseHandler.h"

#include "SocketUtilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int getMsgLenght() {
    return 256;
}

int getDataEntryLenght() {
    int size = sizeof(dataEntry);
    return size;
}

int receiveMsg(int socket, char * buffer) {
    return recv(socket, buffer, getMsgLenght(), 0);
}

int sendMsg(int socket, char * buffer) {
    return send(socket, buffer, getMsgLenght(), 0);
}

int receiveDataEntry(int socket, dataEntry * buffer) {
    return recv(socket, buffer, getDataEntryLenght(), 0);
}

int sendDataEntry(int socket, dataEntry * buffer) {
    return send(socket, buffer, getDataEntryLenght(), 0);
}

void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int create_server_socket(int port) {
    int server_socket;
    struct sockaddr_in server_address;

    // Create the socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        handle_error("socket creation failed");
    }

    // Configure the server address structure
    memset(&server_address, 0, sizeof(server_address)); // helps to avoid weird bugs with uninitialized memory
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY

    
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        handle_error("bind failed");
    }

    return server_socket;
}

int create_client_socket(const char *server_ip, int port) {
    int client_socket;
    struct sockaddr_in server_address;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        handle_error("socket creation failed");
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        handle_error("invalid address or address not supported");
    }

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        handle_error("connection failed");
    }

    return client_socket;
}

int accept_client_connection(int server_socket) {
    struct sockaddr_in client_address;
    socklen_t client_addr_len = sizeof(client_address);

    int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_addr_len);
    if (client_socket < 0) {
        handle_error("accept failed");
    }

    return client_socket;
}

int login(char* user, char* password){
    // sendMsg(client_socket, user);
    // sendMsg(client_socket, password);
    return 1;
}

