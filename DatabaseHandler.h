#include "CommonDefines.h"

#ifndef DATABASE_HANDLER_H
#define DATABASE_HANDLER_H

void debug_populate_db();

FILE * open_db_read();

FILE * open_db_write();

void close_db(FILE *filePointer);

int save_entry(dataEntry dataEntry, FILE *filePointer);

void readEntry(FILE *filePointer, dataEntry * readEntry);

int countEntries(FILE *filePointer, int sizeOfEntry);

int readEntries(FILE *filePointer, dataEntry dataEntries[]);

int remove_all_whitespace(char str[]);
int remove_extra_whitespace(char str[]);

int check_name(char name[]);
int check_phone_number(char phoneNumber[]); 
int check_address(char address[]); 

int validate_entry(dataEntry entry);
void sanitize_entry(dataEntry *entry);

int save_database_to_file(dataEntry *db, int db_size);

#endif // DATABASE_HANDLER_H