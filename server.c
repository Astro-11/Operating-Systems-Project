#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#include "DatabaseHandler.h"
#include "SocketUtilities.h"

#define DEBUG 1
#define ADMIN_PASSWORD "1234"

void admin_loop(int clientSocket);
void user_loop(int clientSocket);

int login_procedure(int clientSocket);
void add_new_record_procedure(int clientSocket, dataEntry entries[], int * entriesCount);
void search_record_procedure(int clientSocket, dataEntry entries[], int entriesCount);
void delete_record_procedure(int clientSocket, dataEntry entries[], int entriesCount);
void edit_record_procedure(int clientSocket, dataEntry entries[], int entriesCount);
void logout_procedure();

int add_new_record(dataEntry entries[], int * entriesCount, dataEntry newDataEntry);
int delete_record(dataEntry entries[], int entriesCount, dataEntry entryToDelete);
int search_records(dataEntry entries[], int entriesCount, dataEntry query, dataEntry queryResults[]);
int edit_record(dataEntry entries[], int entriesCount, dataEntry entryToEdit, dataEntry editedEntry);

int search_record_position(dataEntry entries[], int entriesCount, dataEntry query);
int matches(dataEntry entry, dataEntry filter);
void print_all_entries(dataEntry entries[], int entriesCount);
void send_entries(int clientSocket, dataEntry entries[], int entriesCount);
void update_runtime_database(dataEntry newRuntimeDatabase[], int * newRuntimeEntriesCount);
void handle_sigint(int sig);
void handle_admin_death_signal(int sig);
void handle_user_death_signal(int sig);
// #################################################################################
// #################################################################################

/* 
 * This global variables are needed because it's not possible to 
 * pass arguments to the function that handles SIGINT.
 */
static dataEntry *runtimeDatabase; 
static int runtimeEntriesCount;
static int outdatedRuntimeDb = 0;
static volatile int adminOnline = 0;
static volatile int usersOnline = 0;
static pid_t mainServerPid = 0;
static pid_t userServersGroupPid = 0;

int main(){
    // Handles saving database when server is terminated 
    signal(SIGINT, handle_sigint);
    signal(SIGUSR1, handle_admin_death_signal);
    signal(SIGUSR2, handle_user_death_signal);

    mainServerPid = getpid();
    int serverSocket = create_server_socket(PORT);

    debug_populate_db(); //WARNING: OVERWRITES!
    //runtimeEntriesCount needs to be incremented when admin adds new entry
    update_runtime_database(runtimeDatabase, &runtimeEntriesCount);
    printf("RuntimeEntriesCount: %d\n", runtimeEntriesCount);
    //print_all_entries(runtimeDatabase, runtimeEntriesCount);

    printf("Ctrl+C to terminate the server %d.\n\n", (int)(getpid()));
    
    while(1){
        printf("Main server is online listening for connection...\n");
        listen(serverSocket, 10);
        int clientSocket = accept_client_connection(serverSocket);

        int clientType = login_procedure(clientSocket);
        if (clientType == -1) continue; 
        else if (clientType == ADMIN) adminOnline = 1;
        else if (clientType == BASE) usersOnline++;

        if (outdatedRuntimeDb == 1) {
            update_runtime_database(runtimeDatabase, &runtimeEntriesCount);
            outdatedRuntimeDb = 0;
        }

        int pid = fork();
        if (pid == 0){
             printf("Connection enstablished, awaiting user request. \n\n");

            if(clientType == BASE) {
                adminOnline = 0;
                //If userServersGroup not initialized give it the pid of the first user server
                if (userServersGroupPid == 0) userServersGroupPid = getpid(); 
                setpgid(getpid(), userServersGroupPid);
                user_loop(clientSocket);
            }
            else if (clientType == ADMIN) {
                admin_loop(clientSocket);
            }
        } 
        else 
        {
            printf("Closing connection with main server.\nCreated process %d to handle requests.\n\n", pid);
            //If userServersGroup not initialized give it the pid of the first user server
            if (userServersGroupPid == 0) userServersGroupPid = pid;
            close(clientSocket);
        }
    }
    
    // NOTE S: We never reach here right?
    close(serverSocket); 

    return 0;
}

void admin_loop(int clientSocket) {

    while(1) {

        int choice;
        receive_signal(clientSocket, &choice);

        switch (choice) {
            case SEARCH_DB:
                printf("%d - Search database\n\n", choice);
                search_record_procedure(clientSocket, runtimeDatabase, runtimeEntriesCount);
                printf("Search results succesfully sent to client\n");
                break;

            case ADD_RECORD:
                printf("%d - Add new record\n\n", choice);
                add_new_record_procedure(clientSocket, runtimeDatabase, &runtimeEntriesCount);
                printf("runtimeEntriesCount: %d\n",runtimeEntriesCount);
                break;

            case REMOVE_RECORD:
                printf("%d - Remove record\n\n", choice);
                delete_record_procedure(clientSocket, runtimeDatabase, runtimeEntriesCount);
                printf("runtimeEntriesCount: %d\n",runtimeEntriesCount);
                break;

            case EDIT_RECORD:
                printf("%d - Edit record\n\n", choice);
                edit_record_procedure(clientSocket, runtimeDatabase, runtimeEntriesCount);
                break;

            case LOGOUT: 
                printf("%d - Logout attempt\n\n", choice);
                logout_procedure();
                close(clientSocket);
                exit(EXIT_SUCCESS);
                break;

            default:
                printf("%d - Unexpected request\n\n", choice);
                break; 
        }
    }
}

void user_loop(int clientSocket) {

    while(1) {

        int choice;
        receive_signal(clientSocket, &choice);
        if (outdatedRuntimeDb == 1) {
            update_runtime_database(runtimeDatabase, &runtimeEntriesCount);
            outdatedRuntimeDb = 0;
        }
            
        switch (choice) {
            case SEARCH_DB:
                printf("%d - Search database\n\n", choice);
                search_record_procedure(clientSocket, runtimeDatabase, runtimeEntriesCount);
                printf("Search results succesfully sent to client\n");
                break;

            case LOGOUT: 
                printf("%d - Logout attempt\n\n", choice);
                logout_procedure();
                close(clientSocket);
                exit(EXIT_SUCCESS);
                break;
                
            default:
                if (choice <= 5) printf("%d - Unauthorized request\n\n", choice);
                else printf("%d - Unexpected request\n\n", choice);
                break; 
        }
    }
}

//Returns -1 if login failed, 0 if admin logged in, 1 if user logged in
int login_procedure(int clientSocket) {
    int clientType;
    receive_signal(clientSocket, &clientType);
    if(clientType == ADMIN) {
        char submittedPassword[MSG_LENGHT];
        receiveMsg(clientSocket, submittedPassword);

        if (strcmp(submittedPassword, ADMIN_PASSWORD) != 0){
            sendMsg(clientSocket, ACCESS_DENIED);
            close(clientSocket);

            printf("Someone attemted to login with wrong credentials.\nACCESS DENIED.\n");
            return -1;
        }
        if (adminOnline == 1){
            sendMsg(clientSocket, ACCESS_DENIED);
            close(clientSocket);

            printf("An admin attempted to log in while privileges are already in use.\nACCESS DENIED.\n");
            return -1;
        }
        
        sendMsg(clientSocket, ACCESS_GRANTED);
        printf("Admin access granted to a client.\n");
        return ADMIN;
    }
    // As long as we have only two types of login we should only use else or a we could have uncovered control flows paths
    // else if (clientType == BASE) {
    else  {    
        return BASE;
    }
}

void delete_record_procedure(int clientSocket, dataEntry entries[], int entriesCount) {
    dataEntry entryToDelete;
    receiveDataEntry(clientSocket, &entryToDelete);

    int outcome = delete_record(entries, entriesCount, entryToDelete);

    //Send outcome signal
    send_signal(clientSocket, &outcome);

    //Send possible failure message
    char failureMessage[MSG_LENGHT];
    if (outcome != 0) {
        if (outcome == -1) {
            char tempFailureMessage[MSG_LENGHT] = "All the fields in the query must be present";
            strcpy(failureMessage, tempFailureMessage);
        }
        else if (outcome == -2) {
            char tempFailureMessage[MSG_LENGHT] = "There are no such entries in the database";
            strcpy(failureMessage, tempFailureMessage);
        }
        sendMsg(clientSocket, failureMessage);
        printf("Outcome %d - failed to remove record: %s\n", outcome, failureMessage);
        return;
    }

    printf("Record succesfully removed from database\n");
}

//Returns 0 if succesful, -1 if all dataEntry fields not present, -2 if dataEntry not found
int delete_record(dataEntry entries[], int entriesCount, dataEntry entryToDelete) {
    //Validate query
    if (validate_entry(entryToDelete) < 0) return -1;
    sanitize_entry(&entryToDelete);

    //This assumes that there cannot be duplicated entries
    //Search record
    int entryToDeletePosition = search_record_position(entries, entriesCount, entryToDelete);
    if (entryToDeletePosition < 0) return -2;

    //Delete (for now only sets first char to \0)
    entries[entryToDeletePosition].name[0] = '\0';
    entries[entryToDeletePosition].address[0] = '\0';
    entries[entryToDeletePosition].phoneNumber[0] = '\0';
    return 0;
}

void edit_record_procedure(int clientSocket, dataEntry entries[], int entriesCount) {
    dataEntry entryToEdit;
    receiveDataEntry(clientSocket, &entryToEdit);
    dataEntry editedEntry;
    receiveDataEntry(clientSocket, &editedEntry);

    int outcome = edit_record(entries, entriesCount, entryToEdit, editedEntry);
    send_signal(clientSocket, &outcome);

    if (outcome == -1) {
        char errorMessage[MSG_LENGHT] = "Entry cannot be edited: no such entry in the database\n";
        printf("Outcome %d - failed to edit record\n%s\n", outcome, errorMessage);
        sendMsg(clientSocket, errorMessage);
    }
    else if (outcome == -2) {
        char errorMessage[MSG_LENGHT] = "Entry cannot be edited: invalid modifications\n";
        printf("Outcome %d - failed to edit record\n%s\n", outcome, errorMessage);
        sendMsg(clientSocket, errorMessage);
    }
}

//Returns 0 if succesful, -1 if invalid entryToEdit, -2 if invalid editedEntry
//NOTE S: But it never returns 0?
int edit_record(dataEntry entries[], int entriesCount, dataEntry entryToEdit, dataEntry editedEntry) {
    //Check if entryToEdit is valid and sanitize it
    if (validate_entry(entryToEdit) < 0) return -1;
    sanitize_entry(&entryToEdit);

    //Check if entryToEdit is present in the database
    int position = search_record_position(entries, entriesCount, entryToEdit);
    if (position < 0) return -1;

    //Copy all the fields that are present in editedEntry
    int emptyEntry = 1;
    if (strlen(editedEntry.name) > 0) {
        if (check_name(editedEntry.name) <= 0) return -2;
        remove_extra_whitespace(editedEntry.name); //Could cause problems
        strcpy(entries[position].name, editedEntry.name);
        emptyEntry = 0;
    }
    if (strlen(editedEntry.address) > 0) {
        if (check_address(editedEntry.address) <= 0) return -2;
        remove_extra_whitespace(editedEntry.address); //Could cause problems
        strcpy(entries[position].address, editedEntry.address);
        emptyEntry = 0;
    }
    if (strlen(editedEntry.phoneNumber) > 0) {
        if (check_phone_number(editedEntry.phoneNumber) != 10) return -2;
        remove_all_whitespace(editedEntry.phoneNumber);
        strcpy(entries[position].phoneNumber, editedEntry.phoneNumber);
        emptyEntry = 0;
    }
    if (emptyEntry == 1) return -2;

    //For now only prints name after success
    printf("Edit was a success: new name %s\n", entries[position].name);
}


void search_record_procedure(int clientSocket, dataEntry entries[], int entriesCount) {
    //Receive query
    dataEntry query;
    receiveDataEntry(clientSocket, &query);

    printf("Received query: \n\n\tName: %s\n\tAddress: %s\n\tPhone: %s\n\n", 
                    query.name, 
                    query.address, 
                    query.phoneNumber);
                    
    //Search for record
    dataEntry queryResults[entriesCount];
    int resultsCount = search_records(entries, entriesCount, query, queryResults);

    send_entries(clientSocket, queryResults, resultsCount);
}

//Returns the number of found entries
int search_records(dataEntry entries[], int entriesCount, dataEntry query, dataEntry queryResults[]) {
    const char *fileName = "filteredEntryees.anActualReadableTxtFileAndNotRandomBytes";

    FILE *fp = fopen(fileName, "w");
    if (fp == NULL) {
        perror("Error opening file");
        //return 1; // Return an error code
    }

    int resultsCount = 0;

    for(int i = 0; i < entriesCount; i++){
        if(matches(entries[i], query)){
            queryResults[resultsCount++] = entries[i];

            fprintf(fp, "Name____: %s\n", entries[i].name);
            fprintf(fp, "Address_: %s\n", entries[i].address);
            fprintf(fp, "Phone___: %s\n", entries[i].phoneNumber);
            fprintf(fp, "----------------\n");
            //printf("beep!\n");
        } else {
            //printf("nope!\n");
        }
    }

    //printf("\nSearch results saved locally in file.\n\n");
    fclose(fp);
    return resultsCount;
}

//Returns query position in array or -1 if query not present
int search_record_position(dataEntry entries[], int entriesCount, dataEntry query) {
    for(int i = 0; i < entriesCount; i++) {
        if(matches(entries[i], query)) return i;
    }
    return -1;
}


void add_new_record_procedure(int clientSocket, dataEntry entries[], int * entriesCount) {
    dataEntry newDataEntry;
    receiveDataEntry(clientSocket, &newDataEntry);

    printf("Received record: \nName: %s\nAddress: %s\nNumber: %s \n\n", 
        newDataEntry.name, 
        newDataEntry.address, 
        newDataEntry.phoneNumber);

    int outcome = add_new_record(entries, entriesCount, newDataEntry);
    send_signal(clientSocket, &outcome);

    if (outcome == -1) {
        char errorMessage[MSG_LENGHT] = "Entry cannot be added: invalid entry\n";
        printf("Outcome %d - failed to add record\n%s\n", outcome, errorMessage);
        sendMsg(clientSocket, errorMessage);
    }
    else if (outcome == -2) {
        char errorMessage[MSG_LENGHT] = "Entry cannot be added: entry already present in the database\n";
        printf("Outcome %d - failed to add record\n%s\n", outcome, errorMessage);
        sendMsg(clientSocket, errorMessage);
    }
    else printf("Record succesfully added, database now has %d entries\n", outcome);
}

//Returns the new entriesCount if succesful, -1 if newDataEntry is invalid, -2 if newDataEntry is already present in the db
int add_new_record(dataEntry entries[], int * entriesCount, dataEntry newDataEntry) {
    //NOTE A: At the moment we are validating and sanitizing every entry both when adding it to the runtimeDb and the actual db. 
    //        Should we do it only once?
    if (validate_entry(newDataEntry) < 0) return -1;
    sanitize_entry(&newDataEntry);
    if (search_record_position(entries, *entriesCount, newDataEntry) >= 0) return -2;

    entries[*entriesCount] = newDataEntry;
    *entriesCount = *entriesCount + 1;
    return *entriesCount;
}

void print_all_entries(dataEntry entries[], int entriesCount) {
    int i = 0;
    while (i < entriesCount) {
        printf("\nNome: %s \nIndirizzo: %s \nNumero: %s \n", 
            entries[i].name, 
            entries[i].address, 
            entries[i].phoneNumber);
        i++;
    }
}

void send_entries(int clientSocket, dataEntry entries[], int entriesCount) {
    //Send to client the number of results
    char countMsg[SIGNAL_LENGTH];
    sprintf(countMsg, "%d", entriesCount);
    send_signal(clientSocket, &entriesCount);

    //Send to client the results
    int i = 0;
    while (i < entriesCount) {
        printf("Sending %s...\n", entries[i].name);
        sendDataEntry(clientSocket, &entries[i]);
        i++;
    }
}

int matches(dataEntry entry, dataEntry filter) {
    if (strlen(filter.name) > 0 && strstr(entry.name, filter.name) == NULL) {
        // printf("name mismatch\n");
        return 0;
    }
    if (strlen(filter.address) > 0 && strstr(entry.address, filter.address) == NULL) {
        // printf("addr mismatch\n");
        return 0;
    }
    if (strlen(filter.phoneNumber) > 0 && strstr(entry.phoneNumber, filter.phoneNumber) == NULL) {
        // printf("phone\n");
        return 0;
    }
    return 1;
}

void update_runtime_database(dataEntry newRuntimeDatabase[], int * newRuntimeEntriesCount) {
    FILE * databasePointer = open_db_read();
    *newRuntimeEntriesCount = countEntries(databasePointer, DATAENTRY_LENGHT);

    //NOTE A: for now we do not handle runtime overflow
    //However, it might be worth looking into dynamic data structures for the runtime db
    int runtimeDatabaseSize = 256;
    while(runtimeDatabaseSize < runtimeEntriesCount * 2){
        runtimeDatabaseSize *= 2;
    }

    dataEntry placeholder[runtimeDatabaseSize];
    runtimeDatabase = placeholder;

    readEntries(databasePointer, runtimeDatabase);
    close_db(databasePointer);
}

//For all servers
void handle_sigint(int sig) {
    printf("\n\nCaught signal %d (Ctrl+C). Cleaning up process %d...\n", sig, (int)(getpid()));
    
    //If Main Server kill all User Servers first, then die
    if (mainServerPid == getpid()) {
        printf("Sending SIGINT signal to user server group %d\n", (int)(userServersGroupPid));
        if (userServersGroupPid != 0) kill(userServersGroupPid, SIGINT);
    }
    //If Admin Server save db first, then die
    else if (adminOnline == 1) {
        save_database_to_file(runtimeDatabase, runtimeEntriesCount);
    }

    exit(EXIT_FAILURE); 
}

//For User Server and Admin Server
void logout_procedure() {
    //If child server 
    if(mainServerPid != getpid()) {
        //If admin then save to db and inform main server of your demise
        if (adminOnline == 1) {
            printf("Saving runtime database to file...\n");
            save_database_to_file(runtimeDatabase, runtimeEntriesCount);
            kill(mainServerPid, SIGUSR1);
            exit(EXIT_SUCCESS);
        }
        //If user then only inform main server of your demise
        else if (adminOnline == 0) {
            kill(mainServerPid, SIGUSR2);
            exit(EXIT_SUCCESS);
        }
    }
}

//For main server and child user servers
void handle_admin_death_signal(int sig){
    //If main server reset admin counter and tell child user servers to update db
    if (mainServerPid == getpid()) {
        adminOnline = 0;
        outdatedRuntimeDb = 1;
        if (userServersGroupPid != 0) kill(userServersGroupPid, SIGUSR1);
        printf("There are now %d admins online\n", adminOnline);
    }
    //If child user server update db in next loop
    else {
        outdatedRuntimeDb = 1;
        printf("Process %d will updated its database\n", (int)(getpid()));
    }
}

//For main server only
void handle_user_death_signal(int sig) {
    usersOnline--;
    if (usersOnline == 0) userServersGroupPid = 0; //Reset control group if no more users online
    printf("There are now %d users online\n", usersOnline);
}
