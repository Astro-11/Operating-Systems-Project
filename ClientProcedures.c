// #include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include <signal.h>
#include <unistd.h>

#include "ClientProcedures.h"
#include "SocketUtilities.h"

int init(char password[]);
int search_record(int clientSocket, dataEntry searchedEntry, dataEntry results[]);
int add_new_record(int clientSocket, dataEntry newEntry, char errorMessage[MSG_LENGHT]);
int delete_record(int clientSocket, dataEntry entryToDelete, char errorMessage[MSG_LENGHT]);
int receive_entries(int clientSocket);
int edit_record(int clientSocket, dataEntry entryToEdit, dataEntry editedEntry, char errorMessage[MSG_LENGHT]);
void logout(int clientSocket);

//Set password as 0 to authenticate as BASE
//Returns client socket if succesful, -1 if failed 
int init(char password[]) {
    int clientSocket = create_client_socket(SERVER_IP, PORT);

    if (strcmp(password, "0") == 0) {
        no_login(clientSocket);
    } else {
        char response[MSG_LENGHT];
        login(clientSocket, password, response);
        if(strcmp(response, ACCESS_GRANTED) != 0 )
            return -1;
    }

    return clientSocket;
}

// Return: the number of matching records found
// Side-effect: Saves the entries found in the parameter results[]
int search_record(int clientSocket, dataEntry searchedEntry, dataEntry results[]) {
    int choice = SEARCH_DB;
    send_signal(clientSocket, &choice);
    sendDataEntry(clientSocket, &searchedEntry);

    //Receive and parse the number of entries saved in the db
    int entriesCount;
    receive_signal(clientSocket, &entriesCount);

    //Receive as many entries as present
    int i = 0;
    while (i < entriesCount) {
        dataEntry receivedDataEntry;
        receiveDataEntry(clientSocket, &receivedDataEntry);
        results[i] = receivedDataEntry;
        i++;
    }

    return entriesCount;
}

// Return: the outcome of the operation as an `int`  
// outcome = -1 -> Invalid entry (Invalid characters or errors filling fields of the data entry)
// outcome = -2 -> One or more records already match the whole "signature" of the query and as such is not valid
// else         -> outcome contains the total number of entries that are now present 
// Side-effect: If failure saves an error message to `errorMessage`
int add_new_record(int clientSocket, dataEntry newEntry, char errorMessage[MSG_LENGHT]) {
    int choice = ADD_RECORD;
    send_signal(clientSocket, &choice);
    sendDataEntry(clientSocket, &newEntry);

    int outcome;
    receive_signal(clientSocket, &outcome);
    if (outcome < 0)
        receiveMsg(clientSocket, errorMessage);

    return outcome;
}

// Return: the outcome of the operation as an `int`  
// outcome =  0 -> Successfull deletion
// outcome = -1 -> No matching entries found
// outcome = -2 -> More than one record already match the whole "signature" of the query, it should be exactly 1.
// Side-effect: If failure saves an error message to `errorMessage`
int delete_record(int clientSocket, dataEntry entryToDelete, char errorMessage[MSG_LENGHT]) {
    int choice = REMOVE_RECORD;
    send_signal(clientSocket, &choice);
    sendDataEntry(clientSocket, &entryToDelete);

    int outcome;
    receive_signal(clientSocket, &outcome);
    if (outcome != 0)
        receiveMsg(clientSocket, errorMessage);

    return outcome;
}

int edit_record(int clientSocket, dataEntry entryToEdit, dataEntry editedEntry, char errorMessage[MSG_LENGHT]) {
    int choice = EDIT_RECORD;
    send_signal(clientSocket, &choice);

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
