#include "DatabaseHandler.h"

int getMsgLenght();

int getDataEntryLenght();

int receiveMsg(int socket, char * buffer);

int sendMsg(int socket, char * buffer);

int receiveDataEntry(int socket, dataEntry * buffer);

int sendDataEntry(int socket, dataEntry * buffer);