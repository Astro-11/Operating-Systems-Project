#include <stdio.h>
#include <string.h>

#define ERROR "An error has occured"

struct dataEntry {
    char * name;
    char * address;
    char * phoneNumber;
};

void closeFile(FILE *filePointer) {
    int errorCode;
    errorCode = fclose(filePointer);
    if (errorCode == EOF) printf(ERROR);
}

void writeToFile(char *string, FILE *filePointer) {
    int writtenBytes = 0;
    //int startingPosition = ftell(filePointer);
    writtenBytes = fwrite(string, 4, strlen(string), filePointer);
    //if (writtenBytes != ftell(filePointer) - startingPosition) printf(ERROR);
}

void readFile(FILE *filePointer) {
    int currentPos = fseek(filePointer, 0, SEEK_SET);
    if (currentPos != 0) printf(ERROR);

    char buffer[100];
    int readChars = 0;

    while (1) {
        readChars = fread(buffer, sizeof(buffer), 1, filePointer);
        printf("%s\n", buffer);
        if (readChars <= 0) break;
    }
}

int main(int argc, char const *argv[])
{
    FILE *myFilePtr;
    myFilePtr = fopen("Database.txt", "w+");
    if (myFilePtr == NULL) printf(ERROR);

    char * toWrite = "test";
    writeToFile(toWrite, myFilePtr);
    readFile(myFilePtr);
    
    return 0;
}
