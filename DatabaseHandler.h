#ifndef DATABASE_HANDLER_H
#define DATABASE_HANDLER_H

#include <stdio.h>

#define DATAENTRY_LENGHT sizeof(dataEntry)
#define YELLOWPAGES_DB "YellowPage.data"

typedef struct {
    char name[256];
    char address[256];
    char phoneNumber[256];
} dataEntry;

void debug_populate_db();

FILE * open_db_read();

FILE * open_db_write();

void close_db(FILE *filePointer);

void writeEntry(dataEntry dataEntry, FILE *filePointer);

void readEntry(FILE *filePointer, dataEntry * readEntry);

int countEntries(FILE *filePointer, int sizeOfEntry);

int readEntries(FILE *filePointer, dataEntry dataEntries[]);

int remove_all_whitespace(char str[]);

int validate_entry(dataEntry entry);

char* rtrim(char *str);

void save_database_to_file(dataEntry *db, int db_size);

#endif // DATABASE_HANDLER_H