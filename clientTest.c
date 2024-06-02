#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "DatabaseHandler.h"
#include "SocketUtilities.h"

#define DEBUG 1

static int clientSocket;
static int busy = 0;
static int logoutRequested = 0;

int init(char password[]);
int search_record(int clientSocket, dataEntry searchedEntry, dataEntry results[]);
int add_new_record(int clientSocket, dataEntry newEntry, char errorMessage[MSG_LENGHT]);
int delete_record(int clientSocket, dataEntry entryToDelete, char errorMessage[MSG_LENGHT]);
int receive_entries(int clientSocket);
int edit_record(int clientSocket, dataEntry entryToEdit, dataEntry editedEntry, char errorMessage[MSG_LENGHT]);
void logout(int clientSocket);

void handle_sigint(int sig);

int main(){
    clientSocket = init("0");

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
    busy = 1;

    int outcome = 0;
    char errorMessage[MSG_LENGHT];

    switch (choice)
    {
    case SEARCH_DB:
        send_signal(clientSocket, &choice);
        dataEntry testQuery = { "Mario" "" ""};
        
        dataEntry results[10];
        search_record(clientSocket, testQuery, results);

        if (logoutRequested) logout(clientSocket);
        else busy = 0;
        break;
    case ADD_RECORD:
        send_signal(clientSocket, &choice);

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
        strcpy(newDataEntry.name, rtrim(name));
        strcpy(newDataEntry.address, rtrim(address));
        strcpy(newDataEntry.phoneNumber, rtrim(phoneNumber));

        outcome = add_new_record(clientSocket, newDataEntry, errorMessage);
        if (outcome < 0) printf("Request failed: %s\n", errorMessage);
        else (printf("Entry succesfully added\n"));

        if (logoutRequested) logout(clientSocket);
        else busy = 0;
        break;
    case REMOVE_RECORD:
        send_signal(clientSocket, &choice);
        dataEntry testEntry = {"Mario Rossi", "Via Roma 1, 00100 Roma", "+39 06 12345678"};

        outcome = delete_record(clientSocket, testEntry, errorMessage);
        if (outcome < 0) printf("Request failed: %s\n", errorMessage);
        else printf("%s succesfully removed from database\n", testEntry.name);

        if (logoutRequested) logout(clientSocket);
        else busy = 0;
        break;
    case EDIT_RECORD:
        send_signal(clientSocket, &choice);
        dataEntry entryToEdit = {"Mario Rossi", "Via Roma 1, 00100 Roma", "06 12345678"};
        dataEntry editedEntry = {"  Mario Draghi", "", ""};

        outcome = edit_record(clientSocket, entryToEdit, editedEntry, errorMessage);
        if (outcome = 1) printf("Request failed: %s\n", errorMessage);
        else (printf("Entry succesfully edited\n"));

        if (logoutRequested) logout(clientSocket);
        else busy = 0;
        break;
    case LOGOUT:
        logout(clientSocket);
        if (logoutRequested) logout(clientSocket);
        else busy = 0;
        break;
    default:
        printf("Invalid option selected, try again: \n");
        if (logoutRequested) logout(clientSocket);
        else busy = 0;
        break;
    }

    goto choice_loop;
}

//Set password as 0 to authenticate as BASE
//Returns client socket if succesful, -1 if failed 
int init(char password[]) {
    signal(SIGINT, handle_sigint);
    clientSocket = create_client_socket(SERVER_IP, PORT);

    if (password == 0) {
        no_login(clientSocket);
    } else {
        char response[MSG_LENGHT];
        login(clientSocket, password, response);
        if(strcmp(response, ACCESS_GRANTED) != 0 )
            return -1;
    }

    return clientSocket;
}

//Returns the number of found records 
int search_record(int clientSocket, dataEntry searchedEntry, dataEntry results[]) {
    sendDataEntry(clientSocket, &searchedEntry);

    //Receive and parse the number of entries saved in the db
    int entriesCount;
    receive_signal(clientSocket, &entriesCount);

    #if DEBUG
    printf("There are %d entries:\n", entriesCount);
    #endif

    //Receive as many entries as present
    int i = 0;
    while (i < entriesCount) {
        dataEntry receivedDataEntry;
        receiveDataEntry(clientSocket, &receivedDataEntry);
        results[i] = receivedDataEntry;
        i++;

        #if DEBUG
        printf("Received:\n");
        printf("Nome: %s, Indirizzo: %s, Numero: %s\n", 
                results[i-1].name, 
                results[i-1].address, 
                results[i-1].phoneNumber);
        #endif
    }

    return entriesCount;
}

int add_new_record(int clientSocket, dataEntry newEntry, char errorMessage[MSG_LENGHT]) {
    sendDataEntry(clientSocket, &newEntry);

    int outcome;
    receive_signal(clientSocket, &outcome);
    if (outcome < 0) {
        receiveMsg(clientSocket, errorMessage);
    }

    return outcome;
}

int delete_record(int clientSocket, dataEntry entryToDelete, char errorMessage[MSG_LENGHT]) {
    sendDataEntry(clientSocket, &entryToDelete);

    //Receive validation
    int outcome;
    receive_signal(clientSocket, &outcome);
    if (outcome != 0)
        receiveMsg(clientSocket, errorMessage);

    return outcome;
}

int edit_record(int clientSocket, dataEntry entryToEdit, dataEntry editedEntry, char errorMessage[MSG_LENGHT]) {
    sendDataEntry(clientSocket, &entryToEdit);
    sendDataEntry(clientSocket, &editedEntry);

    int outcome;
    receive_signal(clientSocket, &outcome);
    if (outcome < 0)
        receiveMsg(clientSocket, errorMessage);

    return outcome;
}

void logout(int clientSocket) {
    int logout = LOGOUT;
    send_signal(clientSocket, &logout);
    close(clientSocket);
    exit(EXIT_SUCCESS);
}

void handle_sigint(int sig) {
    if (busy) logoutRequested = 1;
    else logout(clientSocket);
}
