#include "CommonDefines.h"
#include <stdio.h>

#ifndef DATABASE_HANDLER_H
#define DATABASE_HANDLER_H

// void debug_populate_db();

//FILE * open_db_read();

//FILE * open_db_write();

FILE * open_db_read(void (*f)(int errorCode, char* errorMessage));
FILE * open_db_write(void (*f)(int errorCode, char* errorMessage));
int close_db(FILE *filePointer, void (*f)(int errorCode, char* errorMessage));

int countEntries(FILE *filePointer, int sizeOfEntry);
int readEntries(FILE *filePointer, dataEntry dataEntries[]);

int check_name(char name[]);
int check_phone_number(char phoneNumber[]); 
int check_address(char address[]); 

int validate_entry(dataEntry entry);
void sanitize_entry(dataEntry *entry);

int save_database_to_file(FILE* filePointer, dataEntry entries[], int entriesCount);

#endif // DATABASE_HANDLER_H