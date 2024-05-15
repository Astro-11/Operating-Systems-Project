#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "DatabaseHandler.h"

#define DEBUG 0

//Not used for now
#define IS_READING 1
#define IS_EDITING 2
int dbStatus = 0;

void debug_populate_db() {
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

    FILE * db = fopen("Database.txt", "w+"); //WARNING: OVERWRITES!

    for(int i = 0; i < 30; i++) {
        writeEntry(database[i], db);
    }

    close_db(db);
}

FILE * open_db_read() {
    FILE *myFilePtr;
    myFilePtr = fopen("Database.txt", "r");
    if (myFilePtr == NULL) perror("Failed to open db: ");
    return myFilePtr; 
}

FILE * open_db_write() {
    FILE *myFilePtr;
    myFilePtr = fopen("Database.txt", "r+");
    if (myFilePtr == NULL) perror("Failed to open db: ");
    return myFilePtr;
}

void close_db(FILE *filePointer) {
    int errorCode;
    errorCode = fclose(filePointer);
    if (errorCode == EOF) perror("Failed to close db: ");
}

void writeEntry(dataEntry dataEntry, FILE *filePointer) {
    fwrite(&dataEntry, sizeof(dataEntry), 1, filePointer);
}

void readEntry(FILE *filePointer, dataEntry * readEntry) {
    int readChars = 0;

    readChars = fread(readEntry, sizeof(*readEntry), 1, filePointer);

    #if DEBUG
    printf("Nome: %s, Indirizzo: %s, Numero: %s \n", (*readEntry).name, (*readEntry).address, (*readEntry).phoneNumber);
    printf("File is in position %ld \n", ftell(filePointer));
    printf("Read chars: %d \n", readChars);
    #endif
}

int countEntries(FILE *filePointer, int sizeOfEntry) {
    int startingPos = ftell(filePointer);
    if (startingPos < 0) perror("Failed to save pointer inside of db: ");
    rewind(filePointer);

    int entriesCount = 0;
    int hasRead = 0;
    char tempStorage[sizeOfEntry];

    while (1) {
        hasRead = fread(tempStorage, sizeOfEntry, 1, filePointer);
        if (hasRead <= 0) break;
        entriesCount++;
    } 

    fseek(filePointer, 0, startingPos);
    return entriesCount;
}

int readEntries(FILE *filePointer, dataEntry dataEntries[]) {
    int startingPos = ftell(filePointer);
    if (startingPos < 0) perror("Failed to save pointer inside of db: ");
    rewind(filePointer);

    dataEntry readEntry;
    int hasRead = 0;
    int entriesCount = 0;

    while (1) {
        hasRead = fread(&readEntry, sizeof(readEntry), 1, filePointer);
        if (hasRead <= 0) break;
        dataEntries[entriesCount++] = readEntry;
    }

    fseek(filePointer, 0, startingPos); 
    return entriesCount;
}

void readEntireFile(FILE *filePointer) {
    int currentPos = fseek(filePointer, 0, SEEK_SET);
    if (currentPos != 0) perror("Failed to reset pointer inside of db: ");

    char buffer[100];
    int readChars = 0;

    do {
        readChars = fread(buffer, sizeof(buffer), 1, filePointer);
        printf("%s\n", buffer);
    } while (readChars <= 0);
}

char* rtrim(char *str) {
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        len--;
    }
    str[len] = '\0';
    return str;
}

/*int main(int argc, char const *argv[])
{
    FILE *myFilePtr;
    myFilePtr = fopen("Database.txt", "w+");
    if (myFilePtr == NULL) printf(ERROR);

    dataEntry newDataEntry1 = { "Andrea" , "Via Tesla", "111"};
    dataEntry newDataEntry2 = { "Simone" , "Israele", "666"};
    writeEntry(newDataEntry1, myFilePtr);
    writeEntry(newDataEntry2, myFilePtr);

    int entriesCount = countEntries(myFilePtr, sizeof(dataEntry));
    dataEntry dataEntries[entriesCount];
    if (entriesCount != readEntries(myFilePtr, dataEntries)) printf(ERROR);

    printf("There are %d entries: \n", entriesCount);
    int i = 0;
    while (i < entriesCount) {
        printf("Nome: %s, Indirizzo: %s, Numero: %s \n", dataEntries[i].name, dataEntries[i].address, dataEntries[i].phoneNumber);
        i++;
    }

    return 0;
}*/
