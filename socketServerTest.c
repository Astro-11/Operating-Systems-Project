#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "DatabaseHandler.h"
#include "SocketUtilities.h"

#define DEBUG 1

void add_new_record(int clientSocket);
void print_all_entries();
void send_entries(int clientSocket);
void search_record(int clientSocket, dataEntry entries[], int entriesCount);
int matches(dataEntry entry, dataEntry filter);

int main(){

    int server_socket = create_server_socket(PORT);

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
        listen(server_socket, 10);
        int client_socket = accept_client_connection(server_socket);

        if (fork() == 0)
        {
            printf("Connection enstablished, awaiting user request. \n\n");

            int choice;
            receive_signal(client_socket, &choice);

            switch (choice)
            {
            case 1:
                printf("%d - Search database\n\n", choice);
                search_record(client_socket, entries, entriesCount);
                printf("Search results succesfully sent to client\n");
                break;

            case 2:
                printf("%d - Add new record\n\n", choice);
                add_new_record(client_socket);
                printf("Record succesfully saved in db\n");
                break;

            case 3:
                printf("%d - Remove record\n\n", choice);
                break;

            default:
                break;
            }

            printf("Closing connection.\n\n");
        }
        else close(client_socket);
    }
    
    close(server_socket);

    return 0;
}

void search_record(int clientSocket, dataEntry entries[], int entriesCount) {
    dataEntry query;
    receiveDataEntry(clientSocket, &query);

    printf("Received query: \n\n\tName: %s\n\tAddress: %s\n\tPhone: %s\n\n", 
                    query.name, 
                    query.address, 
                    query.phoneNumber);
                    
    const char *fileName = "filteredEntryees.anActualReadableTxtFileAndNotRandomBytes";

    FILE *fp = fopen(fileName, "w");
    if (fp == NULL) {
        perror("Error opening file");
        //return 1; // Return an error code
    }

    dataEntry queryResults[entriesCount];
    int resultsCount = 0;

    for(int i = 0; i < 30; i++){
        if(matches(entries[i], query)){
            queryResults[resultsCount++] = entries[i];

            fprintf(fp, "Name____: %s\n", entries[i].name);
            fprintf(fp, "Address_: %s\n", entries[i].address);
            fprintf(fp, "Phone___: %s\n", entries[i].phoneNumber);
            fprintf(fp, "----------------\n");
            printf("beep!\n");
        } else {
            printf("nope!\n");
        }
    }

    printf("\nSearch results saved locally in file.\n\n");
    fclose(fp);

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

