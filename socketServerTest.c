#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "SocketUtilities.h"

#define DEBUG 1

void add_new_record(int clientSocket);
void print_all_entries();
void send_entries(int clientSocket);

int main(){

    char * tempUser = "User\n";
    char * tempPassword = "Password\n";

    int server_socket = create_server_socket(PORT);

    printf("Ctrl+C to terminate the server.\n\n");

    while(1){
        printf("Server is online listening for connection...\n");
        listen(server_socket, 1);
        int client_socket = accept_client_connection(server_socket);

        if (fork() == 0)
        {
            printf("Connection enstablished, awaiting user request. \n\n");
            
            int choice;
            receive_signal(client_socket, &choice);

            switch (choice)
            {
            case 1:
                printf("%d - Search record\n\n", choice);
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
        printf("\nNome: %s Indirizzo: %s Numero: %s \n", 
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

