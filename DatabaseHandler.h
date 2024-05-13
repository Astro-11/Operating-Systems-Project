#include <stdio.h>

typedef struct {
    char * name;
    char * address;
    char * phoneNumber;
} dataEntry;

void closeFile(FILE *filePointer);

void writeEntry(dataEntry dataEntry, FILE *filePointer);

dataEntry readEntry(FILE *filePointer);

int countEntries(FILE *filePointer, int sizeOfEntry);

int readEntries(FILE *filePointer, dataEntry dataEntries[]);