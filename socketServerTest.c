#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

#include "DatabaseHandler.h"
#include "SocketUtilities.h"

#define DEBUG 1

void add_new_record(int clientSocket);
void search_record_procedure(int clientSocket, dataEntry entries[], int entriesCount);
void delete_record_procedure(int clientSocket, dataEntry entries[], int entriesCount);

int delete_record(dataEntry entries[], int entriesCount, dataEntry entryToDelete);
int search_records(dataEntry entries[], int entriesCount, dataEntry query, dataEntry queryResults[]);

int search_record_position(dataEntry entries[], int entriesCount, dataEntry query);
int matches(dataEntry entry, dataEntry filter);
void print_all_entries();
void send_entries(int clientSocket);

int main(){

    int serverSocket = create_server_socket(PORT);

    printf("Ctrl+C to terminate the server.\n\n");

    debug_populate_db(); //WARNING: OVERWRITES!
    print_all_entries();
    FILE * db = open_db_read();
    int entriesCount = countEntries(db, DATAENTRY_LENGHT);
    dataEntry entries[entriesCount];
    readEntries(db, entries);
    close_db(db);

    while(1){
        printf("Server is online listening for connection...\n");
        listen(serverSocket, 10);
        int clientSocket = accept_client_connection(serverSocket);

        //if (fork() == 0)
        //{
            printf("Connection enstablished, awaiting user request. \n\n");

            int choice;
            receive_signal(clientSocket, &choice);

            switch (choice)
            {
            case 1:
                printf("%d - Search database\n\n", choice);
                search_record_procedure(clientSocket, entries, entriesCount);
                printf("Search results succesfully sent to client\n");
                break;

            case 2:
                printf("%d - Add new record\n\n", choice);
                add_new_record(clientSocket);
                printf("Record succesfully saved in db\n");
                break;

            case 3:
                printf("%d - Remove record\n\n", choice);
                delete_record_procedure(clientSocket, entries, entriesCount);
                break;

            default:
                break;
            }

            printf("Closing connection.\n\n");
        //}
        //else close(clientSocket);
    }
    
    close(serverSocket);

    return 0;
}

void delete_record_procedure(int clientSocket, dataEntry entries[], int entriesCount) {
    dataEntry entryToDelete;
    receiveDataEntry(clientSocket, &entryToDelete);

    int outcome = delete_record(entries, entriesCount, entryToDelete);

    //Send outcome signal
    char outcomeMsg[SIGNAL_LENGTH];
    sprintf(outcomeMsg, "%d", outcome);
    send_signal(clientSocket, outcomeMsg);

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

    //Send to client the number of results
    char countMsg[SIGNAL_LENGTH];
    sprintf(countMsg, "%d", resultsCount);
    send_signal(clientSocket, countMsg);

    //Send to client the results
    int i = 0;
    while (i < resultsCount) {
        printf("Sending %s...\n", queryResults[i].name);
        sendDataEntry(clientSocket, &queryResults[i]);
        i++;
    }
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

void add_new_record(int clientSocket) {
    dataEntry newDataEntry;
    receiveDataEntry(clientSocket, &newDataEntry);

    printf("Received record: \nName: %sAddress: %sNumber: %s \n", 
        newDataEntry.name, 
        newDataEntry.address, 
        newDataEntry.phoneNumber);

    FILE * db = open_db_write();
    fseek(db, 0, SEEK_END);
    writeEntry(newDataEntry, db);

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

void send_entries(int clientSocket) {

    //Count number of entries and send count to Client
    FILE * db = open_db_read();
    int entriesCount = countEntries(db, sizeof(dataEntry));
    char entriesCountMsg[256];
    sprintf(entriesCountMsg, "%d", entriesCount);
    sendMsg(clientSocket, entriesCountMsg);

    //Read entries
    dataEntry dataEntries[entriesCount];
    if (entriesCount != readEntries(db, dataEntries)) printf("An error has occured while reading entries from file");

    //Send as many entries as present in the db
    int i = 0;
    while (i < entriesCount) {
        int sent = sendDataEntry(clientSocket, &dataEntries[i]);
        printf("Sent %d bytes\n", sent);
        i++;
    }

    close_db(db);
}

int matches(dataEntry entry, dataEntry filter) {
    if (strlen(filter.name) > 0 && strstr(entry.name, filter.name) == NULL) {
        printf("name mismatch\n");
        return 0;
    }
    if (strlen(filter.address) > 0 && strstr(entry.address, filter.address) == NULL) {
        printf("addr mismatch\n");
        return 0;
    }
    if (strlen(filter.phoneNumber) > 0 && strstr(entry.phoneNumber, filter.phoneNumber) == NULL) {
        printf("phone\n");
        return 0;
    }
    return 1;
}

