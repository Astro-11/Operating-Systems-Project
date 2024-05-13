#include <sys/socket.h>
#include "DatabaseHandler.h"

int getMsgLenght() {
    return 256;
}

int getDataEntryLenght() {
    int size = sizeof(dataEntry);
    return size;
}

int receiveMsg(int socket, char * buffer) {
    return recv(socket, buffer, getMsgLenght(), 0);
}

int sendMsg(int socket, char * buffer) {
    return send(socket, buffer, getMsgLenght(), 0);
}

int receiveDataEntry(int socket, dataEntry * buffer) {
    return recv(socket, buffer, getDataEntryLenght(), 0);
}

int sendDataEntry(int socket, dataEntry * buffer) {
    return send(socket, buffer, getDataEntryLenght(), 0);
}