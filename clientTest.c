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
    clientSocket = init("1234");

    choice_loop:
    printf("\nSelect an option:\n"
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
    case SEARCH_DB:{
        dataEntry testQuery = { "Mario", "","" };
        
        #if DEBUG
        int j = 0;
        while(j < 255)
            printf("%c", testQuery.name[j++]);
        printf("\n");
        j = 0;
        while(j < 255)
            printf("%c", testQuery.address[j++]);
        printf("\n");
        j = 0;
        while(j < 255)
            printf("%c", testQuery.phoneNumber[j++]);
        printf("\n");
        #endif

        dataEntry results[10];
        int count = search_record(clientSocket, testQuery, results);
        
        #if DEBUG
        printf("Total entries: %d\n\n", count);
        int i = 0;
        while(i < count){
            print_data_entry(results[i]);
            printf("\n");
            i++;
        }
        #endif

        } break;
    case ADD_RECORD:{
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
        if (outcome < 0) 
            printf("Request failed: %s\n", errorMessage);
        else 
            (printf("Entry succesfully added\n"));

        } break;
    case REMOVE_RECORD:{
        dataEntry testEntry = {"Mario Rossi", "", ""};

        outcome = delete_record(clientSocket, testEntry, errorMessage);
        if (outcome < 0) 
            printf("Request failed: %s\n", errorMessage);
        else 
            printf("%s succesfully removed from database\n", testEntry.name);

        } break;
    case EDIT_RECORD:{
        dataEntry entryToEdit = {"Mario Rossi", "", ""};

        dataEntry searchResults[10];
        int resultsCount = search_record(clientSocket, entryToEdit, searchResults);
        
        #if DEBUG
            printf("\nresultCount == %d\nsearchResults[0]:\n",resultsCount);
            print_data_entry(searchResults[0]);
        #endif

        if (resultsCount == 1) {
            entryToEdit = searchResults[0];
            
            #if DEBUG
                printf("\nresultCount == %d\nAssignment to entryToEdit:\n",resultsCount);
                print_data_entry(entryToEdit);
            #endif

            dataEntry editedEntry = {"Mario Draghi", "", ""};
            outcome = edit_record(clientSocket, entryToEdit, editedEntry, errorMessage);

            if (outcome < 0) 
                printf("Request failed.\n%s\n", errorMessage);
            else 
                printf("Entry succesfully edited\n");
        }
        else if (resultsCount == 0) { 
            printf("Request failed: no results were found");
        } 
        else {
            printf("Request failed: more than one results was found\n");
        }

        } break;
    case LOGOUT:
        logout(clientSocket);
        break;
    default:
        printf("Invalid option selected, try again: \n");
        
        break;
    }
    
    if (logoutRequested) 
        logout(clientSocket);
    else busy = 0;

    goto choice_loop;
}

//Set password as 0 to authenticate as BASE
//Returns client socket if succesful, -1 if failed 
int init(char password[]) {
    signal(SIGINT, handle_sigint);
    clientSocket = create_client_socket(SERVER_IP, PORT);

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

void handle_sigint(int sig) {
    if (busy) logoutRequested = 1;
    else logout(clientSocket);
}
