#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "SocketUtilities.h"

int main(){

    int msgLenght = getMsgLenght();

    char user[msgLenght];
    char password[msgLenght];

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;
    int check = connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    //Authentication mock
    printf("Insert your username: ");
    fgets(user, sizeof(user), stdin);
    sendMsg(client_socket, user);
    printf("Insert your password: ");
    fgets(password, sizeof(password), stdin);
    sendMsg(client_socket, password);
    //TODO: add authentication failed signal reception

    //Receive and parse the number of entries saved in the db
    char entriesCountMsg[msgLenght];
    receiveMsg(client_socket, entriesCountMsg);
    int entriesCount;
    sscanf(entriesCountMsg, "%d", &entriesCount);
    printf("There are %d entries:\n", entriesCount);

    //Receive as many entries as present
    int i = 0;
    while (i < entriesCount) {
        dataEntry receivedDataEntry;
        int received = receiveDataEntry(client_socket, &receivedDataEntry);
        printf("Received %d bytes - ", received);
         printf("Nome: %s, Indirizzo: %s, Numero: %s \n", receivedDataEntry.name, receivedDataEntry.address, receivedDataEntry.phoneNumber);
        i++;
    }

    close(client_socket);
    
    return 0;
}