#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "SocketUtilities.h"

char* rtrim(char *str) {
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        len--;
    }
    str[len] = '\0';
    return str;
}

int main(){

    int msgLenght = getMsgLenght();

    char * tempUser = "User";
    char * tempPassword = "Password";

    int server_socket = create_server_socket(PORT);

    printf("Ctrl+C to terminate the server.\n\n");
    

    while(1){
        printf("Server is online listening for connection...\n");
        listen(server_socket, 1);

        int client_socket = accept_client_connection(server_socket);

        if (fork() == 0)
        {
            printf("Connection enstablished, awaiting user credentials. \n");
            char user[msgLenght];
            receiveMsg(client_socket, user);
            printf("Received user: %s", user);
            char password[msgLenght];
            receiveMsg(client_socket, password);
            printf("Received password: %s", password);

            
            int sameUser = strcmp(rtrim(user), tempUser);
            int samePassword = strcmp(rtrim(password), tempPassword);
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