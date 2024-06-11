#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "ClientProcedures.h"
#include "CommonUtils.h"

#define DEBUG 1

static int clientSocket;
static int busy = 0;
static int logoutRequested = 0;

void handle_sigint(int sig);


int main(){
    signal(SIGINT, handle_sigint);
    clientSocket = init("1234");

    choice_loop:
    printf("\nSelect an option:\n"
            "1 - Search record\n"
            "2 - Add new record\n"
            "3 - Remove record\n"
            "4 - Edit record\n"
            "5 - Logout\n");

    char choiceStr[SIGNAL_LENGTH];
    fgets(choiceStr, SIGNAL_LENGTH, stdin);
    int choice;
    //sscanf(choiceStr, "%d", &choice);
    choice = (int)strtol(choiceStr, 0, 10);
    busy = 1;

    int outcome = 0;
    char errorMessage[MSG_LENGHT];

    switch (choice)
    {
    case SEARCH_DB:{
        dataEntry testQuery = { "Mario", "","" };
        
        #if DEBUG
            int j = 0;
            while(j < 255)
                printf("%c", testQuery.name[j++]);
            printf("\n");
            j = 0;
            while(j < 255)
                printf("%c", testQuery.address[j++]);
            printf("\n");
            j = 0;
            while(j < 255)
                printf("%c", testQuery.phoneNumber[j++]);
            printf("\n");
        #endif

        dataEntry results[MAX_RESULTS];
        int count = search_record(clientSocket, testQuery, results);
        
        #if DEBUG
        printf("Total entries: %d\n\n", count);
        int i = 0;
        while(i < count){
            print_data_entry(results[i]);
            printf("\n");
            i++;
        }
        #endif

        } break;
    case ADD_RECORD:{
        char name[MSG_LENGHT];
        char address[MSG_LENGHT];
        char phoneNumber[MSG_LENGHT];

        printf("Name: ");
        fgets(name, MSG_LENGHT, stdin);
        printf("Address: ");
        fgets(address, MSG_LENGHT, stdin);
        printf("Number: ");
        fgets(phoneNumber, MSG_LENGHT, stdin);

        dataEntry newEntry;
        strcpy(newEntry.name, rtrim(name));
        strcpy(newEntry.address, rtrim(address));
        strcpy(newEntry.phoneNumber, rtrim(phoneNumber));

        #if DEBUG
            FILE *file = fopen("output.txt", "w");
            if (file == NULL) return 1;
            fprintf(file, "%s\n", newEntry.name);
            fprintf(file, "%s\n", newEntry.address);
            fprintf(file, "%s\n", newEntry.phoneNumber);
            fclose(file);
        #endif
        
        outcome = add_new_record(clientSocket, newEntry, errorMessage);
        if (outcome < 0) 
            printf("Request failed: %s\n", errorMessage);
        else 
            printf("Entry succesfully added\n");

        } break;
    case REMOVE_RECORD:{
        dataEntry testEntry = {"Mario Rossi", "", ""};

        outcome = delete_record(clientSocket, testEntry, errorMessage);
        if (outcome < 0) 
            printf("Request failed: %s\n", errorMessage);
        else 
            printf("%s succesfully removed from database\n", testEntry.name);

        } break;
    case EDIT_RECORD:{
        dataEntry entryToEdit = {"Mario Lezzi", "", ""};

        dataEntry searchResults[10];
        int resultsCount = search_record(clientSocket, entryToEdit, searchResults);
        
        #if DEBUG
            printf("\nresultCount == %d\nsearchResults[0]:\n",resultsCount);
            print_data_entry(searchResults[0]);
        #endif

        if (resultsCount == 1) {
            entryToEdit = searchResults[0];
            
            #if DEBUG
                printf("\nresultCount == %d\nAssignment to entryToEdit:\n",resultsCount);
                print_data_entry(entryToEdit);
            #endif

            dataEntry editedEntry = {"Mario Rossi", "Via Roma 1, 00100 Roma", "0612345678"};
            outcome = edit_record(clientSocket, entryToEdit, editedEntry, errorMessage);

            if (outcome < 0) 
                printf("Request failed.\n%s\n", errorMessage);
            else 
                printf("Entry succesfully edited\n");
        }
        else if (resultsCount == 0) { 
            printf("Request failed: no results were found");
        } 
        else {
            printf("Request failed: more than one results was found\n");
        }

        } break;
    case LOGOUT:
        logout(clientSocket, 1);
        exit(EXIT_SUCCESS);
        break;
    default:
        printf("Invalid option selected, try again: \n");
        break;
    }
    
    if (logoutRequested) 
        logout(clientSocket, 1);
    else busy = 0;

    goto choice_loop;
}

void handle_sigint(int sig) {
    if (busy) logoutRequested = 1;
    else {
        logout(clientSocket, 1);
        exit(EXIT_FAILURE);
    }
}
