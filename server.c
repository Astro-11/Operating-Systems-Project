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
void add_new_record_procedure(int clientSocket);
void search_record_procedure(int clientSocket, dataEntry entries[], int entriesCount);
void delete_record_procedure(int clientSocket, dataEntry entries[], int entriesCount);
void edit_record_procedure(int clientSocket, dataEntry entries[], int entriesCount);
void logout_procedure();

void add_new_record(int clientSocket, dataEntry newDataEntry);
int delete_record(dataEntry entries[], int entriesCount, dataEntry entryToDelete);
int search_records(dataEntry entries[], int entriesCount, dataEntry query, dataEntry queryResults[]);
void edit_record(int clientSocket, dataEntry entries[], int entriesCount, dataEntry entryToEdit, dataEntry editedEntry);

int search_record_position(dataEntry entries[], int entriesCount, dataEntry query);
int matches(dataEntry entry, dataEntry filter);
void print_all_entries();
void send_entries(int clientSocket, dataEntry entries[], int entriesCount);
void update_runtime_database(dataEntry newRuntimeDatabase[], int * newRuntimeEntriesCount);
void handle_sigint(int sig);
void handle_admin_reset_signal(int sig);
void handle_user_reset_signal(int sig);
// #################################################################################
// #################################################################################

/* 
 * This global variables are needed because it's not possible to 
 * pass arguments to the function that handles SIGINT.
 *
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
    signal(SIGUSR1, handle_admin_reset_signal);
    signal(SIGUSR2, handle_user_reset_signal);

    mainServerPid = getpid();
    int serverSocket = create_server_socket(PORT);

    debug_populate_db(); //WARNING: OVERWRITES!
    //runtimeEntriesCount needs to be incremented when admin adds new entry
    update_runtime_database(runtimeDatabase, &runtimeEntriesCount);
    printf("RuntimeEntriesCount: %d\n", runtimeEntriesCount);

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
                add_new_record_procedure(clientSocket);
                runtimeEntriesCount++;
                printf("Record succesfully saved in db\n");
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
    int outcome = 0;

    //Validate query
    outcome = validate_entry(entryToDelete);
    if (outcome < 0) return -1;

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

    edit_record(clientSocket, entries, entriesCount, entryToEdit, editedEntry);
}

void edit_record(int clientSocket, dataEntry entries[], int entriesCount, dataEntry entryToEdit, dataEntry editedEntry) {
    int position = search_record_position(entries, entriesCount, entryToEdit);
    send_signal(clientSocket, &position);

    if (position < 0) {
        char errorMessage[MSG_LENGHT] = "Entry cannot be edited: no such entry in the database\n";
        printf("%s", errorMessage);
        sendMsg(clientSocket, errorMessage);
        return;
    }

    if (strlen(editedEntry.name) > 0)
        strcpy(entries[position].name, editedEntry.name);
    if (strlen(editedEntry.address) > 0)
        strcpy(entries[position].address, editedEntry.address);
    if (strlen(editedEntry.phoneNumber) > 0)
        strcpy(entries[position].phoneNumber, editedEntry.phoneNumber);

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


//NOTE S: this function would be deprecated because we never save to file single a single Entry
//NOTE A: this function is a server procedure, what it actually does when saving should be irrelevant. 
//NOTE A: the code will be refactored to better reflect this.
void add_new_record_procedure(int clientSocket) {
    dataEntry newDataEntry;
    receiveDataEntry(clientSocket, &newDataEntry);

    printf("Received record: \nName: %s\nAddress: %s\nNumber: %s \n\n", 
        newDataEntry.name, 
        newDataEntry.address, 
        newDataEntry.phoneNumber);

    add_new_record(clientSocket, newDataEntry);
}

void add_new_record(int clientSocket, dataEntry newDataEntry) {
    FILE * db = open_db_write();
    fseek(db, 0, SEEK_END);
    save_entry(newDataEntry, db);

    #if DEBUG
    //Checks if record was actually written
    fseek(db, -DATAENTRY_LENGHT, SEEK_CUR);
    dataEntry readNewEntry;
    readEntry(db, &readNewEntry);
    printf("Saved record: \nName: %sAddress: %sNumber: %s \n", 
        readNewEntry.name, 
        readNewEntry.address, 
        readNewEntry.phoneNumber);
    #endif

    close_db(db);
}

void print_all_entries() {
    FILE * db = open_db_read();
    int entriesCount = countEntries(db, sizeof(dataEntry));
    dataEntry dataEntries[entriesCount];
    if (entriesCount != readEntries(db, dataEntries)) printf("An error has occured while reading entries from file");

    int i = 0;
    while (i < entriesCount) {
        printf("\nNome: %s \nIndirizzo: %s \nNumero: %s \n", 
            dataEntries[i].name, 
            dataEntries[i].address, 
            dataEntries[i].phoneNumber);
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
void handle_admin_reset_signal(int sig){
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
void handle_user_reset_signal(int sig) {
    usersOnline--;
    if (usersOnline == 0) userServersGroupPid = 0; //Reset control group if no more users online
    printf("There are now %d users online\n", usersOnline);
}
