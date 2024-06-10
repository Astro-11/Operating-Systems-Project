#include "CommonDefines.h"

#ifndef CLIENT_PROCEDURES_H
#define CLIENT_PROCEDURES_H

int init(char password[]);
int search_record(int clientSocket, dataEntry searchedEntry, dataEntry results[]);
int add_new_record(int clientSocket, dataEntry newEntry, char errorMessage[MSG_LENGHT]);
int delete_record(int clientSocket, dataEntry entryToDelete, char errorMessage[MSG_LENGHT]);
int edit_record(int clientSocket, dataEntry entryToEdit, dataEntry editedEntry, char errorMessage[MSG_LENGHT]);
void logout(int clientSocket, int request);

#endif 