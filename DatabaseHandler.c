#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "DatabaseHandler.h"
#include "CommonDefines.h"
#include "CommonUtils.h"

#define DEBUG 0

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
int save_entry(dataEntry newDataEntry, FILE *filePointer);


FILE * open_db_read(void (*f)(int errorCode, char* errorMessage)) {
    FILE *myFilePtr;
    myFilePtr = fopen(YELLOWPAGES_DB, "r");
    if (myFilePtr == NULL) (*f)(0, "Failed to open database");
    return myFilePtr; 
}

//NOTE A: might be worth it to create at least one backup before overwriting the whole db
//WARNING: OVERWRITES!
FILE * open_db_write(void (*f)(int errorCode, char* errorMessage)) {
    FILE *myFilePtr;
    //myFilePtr = fopen("YelloPageBackup.data", "r"); Copy into this before overwriting?
    myFilePtr = fopen(YELLOWPAGES_DB, "wb"); 
    if (myFilePtr == NULL) (*f)(0, "Failed to open database");
    return myFilePtr;
}

int close_db(FILE *filePointer, void (*f)(int errorCode, char* errorMessage)) {
    int errorCode;
    errorCode = fclose(filePointer);
    if (errorCode == EOF) {
        (*f)(0, "Failed to close database");
        return -1;
    }
    return 0;
}

//Returns the number of entries found or -1 in case of error
int countEntries(FILE *filePointer, int sizeOfEntry) {
    int startingPos = ftell(filePointer);
    if (startingPos < 0) return -1;
    rewind(filePointer);

    int entriesCount = 0;
    int hasRead = 0;
    char tempStorage[sizeOfEntry];

    while (1) {
        hasRead = fread(tempStorage, sizeOfEntry, 1, filePointer);
        if (hasRead <= 0) break;
        entriesCount++;
    } 

    int seekResult = fseek(filePointer, 0, startingPos);
    if (seekResult < 0) return -1;
    return entriesCount;
}

//Returns the number of entries read or -1 in case of error
int readEntries(FILE *filePointer, dataEntry dataEntries[]) {
    int startingPos = ftell(filePointer);
    if (startingPos < 0) return -1;
    rewind(filePointer);

    dataEntry readEntry;
    int hasRead = 0;
    int entriesCount = 0;

    while (1) {
        hasRead = fread(&readEntry, sizeof(readEntry), 1, filePointer);
        if (hasRead <= 0) break;
        dataEntries[entriesCount++] = readEntry;
    }

    int seekResult = fseek(filePointer, 0, startingPos); 
    if (seekResult < 0) return -1;
    return entriesCount;
}

int check_name(char name[]) {
    int count = 0;
    char c;
    int i = 0;
    while ((c = name[i++]) != '\0') 
        if (!isalpha(c)){
            if ( c != ' ') 
                return 0; 
        } else
            if(++count > MAX_FIELD_LENGHT - 10)
                return 0;
    
    return count;
}

int check_address(char address[]) {
    int count = 0;
    char c;
    int i = 0;
    while ((c = address[i++]) != '\0') 
        if (c != ' ' && isspace(c)){
                return 0; 
        } else{
            if(++count > MAX_FIELD_LENGHT - 10)
                return 0;
        }
    return count;
}

int check_phone_number(char phoneNumber[]) {
    int count = 0;
    char c;
    int i = 0;
    while ((c = phoneNumber[i++]) != '\0') 
        if (!isdigit(c)){
            if (c != ' ')
                return 0; 
        } else{
            if(++count > MAX_FIELD_LENGHT - 10)
                return 0;;
        }
    return count;
}

//Return 0 if entry is valid, negative otherwise
int validate_entry(dataEntry entry) {

    if (check_name(entry.name) <= 0)
        return -1;
    
    if (check_address(entry.address) <= 0) 
        return -2;
    
    if (check_phone_number(entry.phoneNumber) != 10)
        return -3;
    
    return 0;
}


// sanitize_entry() assumes the input has already been validated
// Input needs to be passed explicitly with pointer and field accessed with -> operator 
// because by default structs are passed by copy and not reference 
void sanitize_entry(dataEntry *entry) {
    remove_extra_whitespace(entry->name);
    remove_extra_whitespace(entry->address);
    remove_all_whitespace(entry->phoneNumber);
}



//NOTE A: the database IS the file. Referring to runtime dataEntry arrays as db might be confusing.
//Returns the number of entries actually saved to db
int save_database_to_file(FILE* filePointer, dataEntry entries[], int entriesCount) {
    int savedEntriesCount = 0;
    for(int i = 0; i < entriesCount; i++)
        if (save_entry(entries[i], filePointer))
            savedEntriesCount++;

    return savedEntriesCount;
}

//Only local
//Returns 0 if succesful, -1 if trying to save invalid dataEntry
int save_entry(dataEntry newDataEntry, FILE *filePointer) {
    sanitize_entry(&newDataEntry);

    int out;
    if ((out = validate_entry(newDataEntry)) != 0){
        #if DEBUG
            printf("-----------\n");
            printf("Skipped %s. Invalid because of %d.\n", newDataEntry.name, out);
            print_data_entry(newDataEntry);
        #endif
        
        return -1;
    }
    if (fwrite(&newDataEntry, sizeof(dataEntry), 1, filePointer) != 1)
        return -1;

    return 0;
}
