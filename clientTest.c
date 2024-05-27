#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "SocketUtilities.h"

static int logoutRequested = 0;

void add_new_record(int clientSocket);
void delete_record(int clientSocket);
void receive_entries(int clientSocket);
void edit_record(int clientSocket, dataEntry entryToEdit, dataEntry editedEntry);
void logout(int clientSocket);

void handle_sigint(int sig);

int main(){
    signal(SIGINT, handle_sigint);
    int clientSocket = create_client_socket(SERVER_IP, PORT);
    //no_login(clientSocket);
    login(clientSocket, "1234");

    choice_loop:
    printf("Select an option:\n"
            "1 - Search record\n"
            "2 - Add new record\n"
            "3 - Remove record\n"
            "4 - Edit record\n"
            "5 - Logout\n");

    char choiceStr[SIGNAL_LENGTH];
    fgets(choiceStr, SIGNAL_LENGTH, stdin);
    int choice;
    //sscanf(choiceStr, "%d", &choice);
    choice = (int)strtol(choiceStr, 0, 10);

    switch (choice)
    {
    case SEARCH_DB:
        send_signal(clientSocket, choiceStr);
        dataEntry testQuery = { "Mario" "" ""};
        sendDataEntry(clientSocket, &testQuery);
        receive_entries(clientSocket);
        if (logoutRequested) logout(clientSocket);
        break;
    case ADD_RECORD:
        send_signal(clientSocket, choiceStr);
        add_new_record(clientSocket);
        if (logoutRequested) logout(clientSocket);
        break;
    case REMOVE_RECORD:
        send_signal(clientSocket, choiceStr);
        delete_record(clientSocket);
        if (logoutRequested) logout(clientSocket);
        break;
    case EDIT_RECORD:
        send_signal(clientSocket, choiceStr);
        dataEntry entryToEdit = {"Mario Rossi", "Via Roma 1, 00100 Roma", "+39 06 12345678"};
        dataEntry editedEntry = {"Mario Draghi", "", ""};
        edit_record(clientSocket, entryToEdit, editedEntry);
        if (logoutRequested) logout(clientSocket);
        break;
    case LOGOUT:
        logout(clientSocket);
        if (logoutRequested) logout(clientSocket);
        break;
    default:
        printf("Invalid option selected, try again: \n");
        if (logoutRequested) logout(clientSocket);
        break;
    }

    goto choice_loop;
}

void add_new_record(int clientSocket) {
    char name[MSG_LENGHT];
    char address[MSG_LENGHT];
    char phoneNumber[MSG_LENGHT];

    printf("Name: ");
    fgets(name, MSG_LENGHT, stdin);
    printf("Address: ");
    fgets(address, MSG_LENGHT, stdin);
    printf("Number: ");
    fgets(phoneNumber, MSG_LENGHT, stdin);

    dataEntry newDataEntry;
    strcpy(newDataEntry.name, name);
    strcpy(newDataEntry.address, address);
    strcpy(newDataEntry.phoneNumber, phoneNumber);
    sendDataEntry(clientSocket, &newDataEntry);
}

void delete_record(int clientSocket) {
    dataEntry testEntry = {"Mario Rossi", "Via Roma 1, 00100 Roma", "+39 06 12345678"};
    sendDataEntry(clientSocket, &testEntry);

    //Receive validation
    int outcome;
    receive_signal(clientSocket, &outcome);
    if (outcome != 0) {
        char failureMsg[MSG_LENGHT];
        receiveMsg(clientSocket, failureMsg);
        printf("Request failed: %s\n", failureMsg);
        return;
    } 
    else printf("%s succesfully removed from database\n", testEntry.name);
}

void edit_record(int clientSocket, dataEntry entryToEdit, dataEntry editedEntry) {
    sendDataEntry(clientSocket, &entryToEdit);
    sendDataEntry(clientSocket, &editedEntry);

    int outcome;
    receive_signal(clientSocket, &outcome);
    if (outcome < 0) {
        char failureMsg[MSG_LENGHT];
        receiveMsg(clientSocket, failureMsg);
        printf("Request failed: %s\n", failureMsg);
    }

    printf("Entry succesfully edited\n");
}

void receive_entries(int clientSocket) {

    //Receive and parse the number of entries saved in the db
     int entriesCount;
    receive_signal(clientSocket, &entriesCount);
    printf("There are %d entries:\n", entriesCount);

    //Receive as many entries as present
    int i = 0;
    while (i < entriesCount) {
        dataEntry receivedDataEntry;
        int received = receiveDataEntry(clientSocket, &receivedDataEntry);
        printf("Received %d bytes - ", received);
        printf("Nome: %s, Indirizzo: %s, Numero: %s \n", 
                receivedDataEntry.name, 
                receivedDataEntry.address, 
                receivedDataEntry.phoneNumber);
        
        i++;
    }
}

void logout(int clientSocket) {
    char choiceStr[SIGNAL_LENGTH];
    sprintf(choiceStr, "%d", LOGOUT);
    send_signal(clientSocket, choiceStr);
    close(clientSocket);
    exit(EXIT_SUCCESS);
}

void handle_sigint(int sig) {
    logoutRequested = 1;
}
