#include <stdio.h>
#include <string.h>
#include "DatabaseHandler.h"

#define ERROR "An error has occured"

void closeFile(FILE *filePointer) {
    int errorCode;
    errorCode = fclose(filePointer);
    if (errorCode == EOF) printf(ERROR);
}

void writeEntry(dataEntry dataEntry, FILE *filePointer) {
    fwrite(&dataEntry, sizeof(dataEntry), 1, filePointer);
}

dataEntry readEntry(FILE *filePointer) {
    int currentPos = fseek(filePointer, 0, SEEK_SET);
    if (currentPos != 0) printf(ERROR);

    dataEntry readEntry;
    int readChars = 0;

    while(1) {
        readChars = fread(&readEntry, sizeof(readEntry), 1, filePointer);
        if (readChars <= 0) break;
        /*printf("Nome: %s, Indirizzo: %s, Numero: %s \n", readEntry.name, readEntry.address, readEntry.phoneNumber);
        printf("File is in position %ld \n", ftell(filePointer));
        printf("Read chars: %d \n", readChars);*/
    } 
}

int countEntries(FILE *filePointer, int sizeOfEntry) {
    int startingPos = ftell(filePointer);
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
    if (currentPos != 0) printf(ERROR);

    char buffer[100];
    int readChars = 0;

    do {
        readChars = fread(buffer, sizeof(buffer), 1, filePointer);
        printf("%s\n", buffer);
    } while (readChars <= 0);
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
