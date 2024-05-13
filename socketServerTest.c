#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "SocketUtilities.h"

int main(){

    int msgLenght = getMsgLenght();

    char * tempUser = "User\n";
    char * tempPassword = "Password\n";

    // creo il sokcet
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    //Setsockopt
    //Faccio il setup del socket appena creato
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY; //INADDR_LOOPBACK
    
    int check = bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    printf("Ctrl+C to terminate the server.\n\n");
    printf("Server is online listening for connection...\n");

    while(1){
        listen(server_socket, 1);
        int client_socket = accept(server_socket, NULL, NULL);
        if (fork() == 0)
        {
            printf("Connection enstablished, awaiting user credentials. \n");
            char user[msgLenght];
            receiveMsg(client_socket, user);
            printf("Received user: %s", user);
            char password[msgLenght];
            receiveMsg(client_socket, password);
            printf("Received password: %s", password);

            int sameUser = strcmp(user, tempUser);
            int samePassword = strcmp(password, tempPassword);
            //printf("User: %s, Expected User: %s, Password: %s, Expected Password: %s\n", user, tempUser, password, tempPassword);
            //printf("Is same user: %d, has same password: %d", sameUser, samePassword);

            if(sameUser == 0 && samePassword == 0) {
                printf("User authenticated succesfully\n");

                //Write two entries to mock database
                FILE *myFilePtr;
                myFilePtr = fopen("Database.txt", "w+");
                dataEntry newDataEntry1 = { "Andrea" , "Via Tesla", "111"};
                dataEntry newDataEntry2 = { "Simone" , "Israele", "666"};
                writeEntry(newDataEntry1, myFilePtr);
                writeEntry(newDataEntry2, myFilePtr);

                //Count number of entries and send count to Client
                int entriesCount = countEntries(myFilePtr, sizeof(dataEntry));
                char entriesCountMsg[256];
                sprintf(entriesCountMsg, "%d", entriesCount);
                sendMsg(client_socket, entriesCountMsg);

                //Read entries
                dataEntry dataEntries[entriesCount];
                if (entriesCount != readEntries(myFilePtr, dataEntries)) printf("An error has occured while reading entries from file");

                //Send as many entries as present in the db
                int i = 0;
                while (i < entriesCount) {
                    int sent = sendDataEntry(client_socket, &dataEntries[i]);
                    printf("Sent %d bytes\n", sent);
                    i++;
                }
            }
            //TODO: send authentication failed signal to Client
            else printf("User failed authentication\n");

            printf("Closing connection.\n\n");
        }
        else close(client_socket);
    }
    
    close(server_socket);

    return 0;
}