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

int matches(dataEntry entry, dataEntry filter) {
    if (strlen(filter.name) > 0 && strstr(entry.name, filter.name) == NULL) {
        return 0;
    }
    if (strlen(filter.address) > 0 && strstr(entry.address, filter.address) == NULL) {
        return 0;
    }
    if (strlen(filter.phoneNumber) > 0 && strstr(entry.phoneNumber, filter.phoneNumber) == NULL) {
        return 0;
    }
    return 1;
}



int main(){

    dataEntry database[30] = {
        {"Mario Rossi", "Via Roma 1, 00100 Roma", "+39 06 12345678"},
        {"Luigi Bianchi", "Piazza del Duomo 2, 50122 Firenze", "+39 055 1234567"},
        {"Giulia Verdi", "Via Garibaldi 3, 10121 Torino", "+39 011 1234567"},
        {"Francesca Neri", "Via Milano 4, 20121 Milano", "+39 02 1234567"},
        {"Antonio Russo", "Piazza San Marco 5, 30124 Venezia", "+39 041 1234567"},
        {"Alessandro Ferrari", "Via Mazzini 6, 80134 Napoli", "+39 081 1234567"},
        {"Elena Ricci", "Corso Vittorio Emanuele 7, 90133 Palermo", "+39 091 1234567"},
        {"Matteo Conti", "Via Dante 8, 40121 Bologna", "+39 051 1234567"},
        {"Silvia Moretti", "Via Emilia 9, 29121 Piacenza", "+39 0523 123456"},
        {"Stefano Marino", "Via San Lorenzo 10, 16121 Genova", "+39 010 1234567"},
        {"Simone Gallo", "Via Napoli 11, 85100 Potenza", "+39 0971 123456"},
        {"Sofia Fontana", "Via Roma 12, 67100 L'Aquila", "+39 0862 123456"},
        {"Andrea Lombardi", "Via Torino 13, 10121 Torino", "+39 011 7654321"},
        {"Chiara Rizzi", "Via Firenze 14, 50122 Firenze", "+39 055 7654321"},
        {"Lorenzo Benedetti", "Via Veneto 15, 00187 Roma", "+39 06 7654321"},
        {"Paolo Mancini", "Via Po 16, 10123 Torino", "+39 011 8765432"},
        {"Martina Riva", "Via Mura dei Francesi 17, 00043 Ciampino", "+39 06 7890123"},
        {"Davide Testa", "Via Salaria 18, 00199 Roma", "+39 06 23456789"},
        {"Sara D'Amico", "Via Manzoni 19, 80123 Napoli", "+39 081 2345678"},
        {"Giorgio Greco", "Corso Europa 20, 16132 Genova", "+39 010 2345678"},
        {"Federica Colombo", "Via del Corso 21, 00186 Roma", "+39 06 3456789"},
        {"Marco Gatti", "Via Nomentana 22, 00162 Roma", "+39 06 9876543"},
        {"Alessia De Luca", "Via Cassia 23, 00189 Roma", "+39 06 5432167"},
        {"Roberto Vitale", "Via Tuscolana 24, 00181 Roma", "+39 06 6543210"},
        {"Laura Ferraro", "Via dei Mille 25, 80135 Napoli", "+39 081 8765432"},
        {"Giovanni Ruggiero", "Via Guglielmo Marconi 26, 24060 Brusaporto", "+39 035 6543210"},
        {"Anna Barbieri", "Via Cavour 27, 96017 Noto", "+39 0931 654321"},
        {"Massimo Grasso", "Via Indipendenza 28, 40121 Bologna", "+39 051 3456789"},
        {"Teresa Longo", "Via Nuova 29, 80122 Napoli", "+39 081 5678901"},
        {"Pietro Donati", "Via San Raffaele 30, 20121 Milano", "+39 02 5678901"}
    };

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