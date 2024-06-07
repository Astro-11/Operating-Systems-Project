#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "DatabaseHandler.h"
#include "CommonUtils.h"

#define DEBUG 0
//Not used for now
#define IS_READING 1
#define IS_EDITING 2
int dbStatus = 0;

void debug_populate_db() {
    dataEntry debugDatabase[] = {
        {"Mario Rossi", "Via Roma 1, 00100 Roma", "06 12345678"},
        {"Mario Lezzi", "Via Roma 1, 00100 Roma", "06 12345678"},
        {"Luigi Bianchi", "Piazza del Duomo 2, 50122 Firenze", "  055 1234567  "},
        {"Giulia Verdi", "Via Garibaldi 3, 10121 Torino", "011 1234567"},
        {"Francesca Neri", "     Via Milano 4, 20121 Milano", "02 1234567"},
        {"Antonio Russo", "Piazza San Marco 5, 30124 Venezia", "041 1234567   "},
        {"Alessandro Ferrari", "    Via Mazzini 6, 80134 Napoli", "081 1234567"},
        {"Elena Ricci", "Corso Vittorio Emanuele 7, 90133 Palermo", "+39 091 1234567"},
        {"Matteo Conti", "Via Dante 8, 40121 Bologna", "051 1234567"},
        {"Silvia Moretti", "Via Emilia 9, 29121 Piacenza", "0523 123456"},
        {"Stefano Marino", "Via San Lorenzo 10, 16121 Genova", "+010 1234567"},
        {"Simone Gallo", "Via Napoli 11, 85100 Potenza", "+0971 123456"},
        {"Sofia Fontana", "Via Roma 12, 67100 L'Aquila", "+0862 123456"},
        {"Andrea Lombardi", "Via Torino 13, 10121 Torino", "011 7654321"},
        {"Chiara Rizzi", "Via Firenze 14, 50122 Firenze", " 055 765 4321"},
        {"Lorenzo Benedetti", "Via Veneto 15, 00187 Roma", "06 7654321"},
        {"Paolo Mancini", "Via Po 16, 10123 Torino", "011 8765432"},
        {"Martina Riva", "Via Mura dei Francesi 17, 00043 Ciampino", "06 7890123"},
        {"Davide Testa", "Via Salaria 18, 00199 Roma", "06 23456789"},
        {"     Sara D'Amico", "Via Manzoni 19, 80123 Napoli", "081 2345678"},
        {"Giorgio Greco     ", "Corso Europa 20, 16132 Genova", "010 2345678"},
        {"Federica Colombo", "Via del Corso 21, 00186 Roma", "06 3456789"},
        {"Marco Gatti", "Via Nomentana 22, 00162 Roma", "06 9876543"},
        {"Alessia De Luca", "Via Cassia 23, 00189 Roma", "06 5432167"},
        {"Roberto Vitale", "Via Tuscolana 24, 00181 Roma", "06 6543210"},
        {"Laura Ferraro", "Via dei Mille 25, 80135 Napoli", "+39 081 8765432"},
        {"Giovanni Ruggiero", "Via Guglielmo Marconi 26, 24060 Brusaporto", "035 6543210"},
        {"Anna Barbieri", "Via Cavour 27, 96017 Noto", "0931 654321"},
        {"Massimo Grasso", "Via Indipendenza 28, 40121 Bologna", "39 051 3456789"},
        {"Teresa Longo", "Via Nuova 29, 80122 Napoli", "081 5678901"},
        {"Pietro Donati", "   ", "02 5678901"}
    };

    int arraySize = sizeof(debugDatabase)/sizeof(debugDatabase[0]);
    FILE * db = fopen(YELLOWPAGES_DB, "w+"); //WARNING: OVERWRITES!

    for(int i = 0; i < arraySize; i++) {
        save_entry(debugDatabase[i], db);
    }

    close_db(db);
}

FILE * open_db_read() {
    FILE *myFilePtr;
    myFilePtr = fopen(YELLOWPAGES_DB, "r");
    if (myFilePtr == NULL) perror("Failed to open db: ");
    return myFilePtr; 
}

//NOTE A: might be worth it to create at least one backup before overwriting the whole db
//WARNING: OVERWRITES!
FILE * open_db_write() {
    FILE *myFilePtr;
    //myFilePtr = fopen("YelloPageBackup.data", "r"); Copy into this before overwriting?
    myFilePtr = fopen(YELLOWPAGES_DB, "wb"); 
    if (myFilePtr == NULL) perror("Failed to open db: ");
    return myFilePtr;
}

void close_db(FILE *filePointer) {
    int errorCode;
    errorCode = fclose(filePointer);
    if (errorCode == EOF) perror("Failed to close db: ");
}

//NOTE S: Given that it only saves the writing of two paramaters this may be an unneded abstraction
//NOTE A: Given that it prevents me from saving with a wrong size, this is a needed abstraction
//NOTE A: Also, we should include some validation.
//Returns 0 if succesful, -1 if trying to save invalid dataEntry
int save_entry(dataEntry newDataEntry, FILE *filePointer) {
    if (validate_entry(newDataEntry) != 0)
        return FALSE;
    sanitize_entry(&newDataEntry);
    fwrite(&newDataEntry, sizeof(dataEntry), 1, filePointer);
    return TRUE;
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

int check_name(char name[]) {
    int count = 0;
    char c;
    int i = 0;
    while ((c = name[i++]) != '\0') 
        if (!isalpha(c)){
            if ( c != ' ') 
                return 0; 
        } else
            count++;
    
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
            count++;
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
            //              &&
            //     c != '.' &&
            //     c != '-' &&
            //     c != '_' 
                return 0; 
        } else{
            count++;
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
int save_database_to_file(dataEntry entries[], int entriesCount){
    FILE* db = open_db_write();

    int savedEntriesCount = 0;
    for(int i = 0; i < entriesCount; i++)
        if (save_entry(entries[i], db))
            savedEntriesCount++;
    
    // if (fclose(db) == EOF) 
    //     perror("Failed to close db: ");

    close_db(db);
    return savedEntriesCount;
}
