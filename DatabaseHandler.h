#ifndef DATABASE_HANDLER_H
#define DATABASE_HANDLER_H

#include <stdio.h>

#define DATAENTRY_LENGHT sizeof(dataEntry)

typedef struct {
    char name[256];
    char address[256];
    char phoneNumber[256];
} dataEntry;

FILE * open_db_read();

FILE * open_db_write();

void close_db(FILE *filePointer);

void writeEntry(dataEntry dataEntry, FILE *filePointer);

void readEntry(FILE *filePointer, dataEntry * readEntry);

int countEntries(FILE *filePointer, int sizeOfEntry);

int readEntries(FILE *filePointer, dataEntry dataEntries[]);

#endif // DATABASE_HANDLER_H