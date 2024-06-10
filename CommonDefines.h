#ifndef COMMON_DEFINES_H
#define COMMON_DEFINES_H

#define YELLOWPAGES_DB "YellowPage.data"

#define SERVER_IP "127.0.0.1"
#define PORT 9890

#define DATAENTRY_LENGHT sizeof(dataEntry)
#define SIGNAL_LENGTH 4
#define MSG_LENGHT 256

#define MAX_RESULTS 128

#define ADMIN 0
#define BASE 1

#define ACCESS_GRANTED "ACCESS_GRANTED"
#define ACCESS_DENIED "ACCESS_DENIED"

typedef struct {
    char name[MSG_LENGHT];
    char address[MSG_LENGHT];
    char phoneNumber[MSG_LENGHT];
} dataEntry;

typedef enum {
    SEARCH_DB = 1,
    ADD_RECORD,
    REMOVE_RECORD,
    EDIT_RECORD,
    LOGOUT,
} serverRequest;

#endif