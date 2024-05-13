#include <stdio.h>

typedef struct {
    char name[256];
    char address[256];
    char phoneNumber[256];
} dataEntry;

void closeFile(FILE *filePointer);

void writeEntry(dataEntry dataEntry, FILE *filePointer);

dataEntry readEntry(FILE *filePointer);

int countEntries(FILE *filePointer, int sizeOfEntry);

int readEntries(FILE *filePointer, dataEntry dataEntries[]);