#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(){

    int msgLenght = 256;

    char user[msgLenght];
    char password[msgLenght];

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;
    int check = connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    printf("Insert your username: ");
    fgets(user, sizeof(user), stdin);
    send(client_socket, user, sizeof(user), 0);
    printf("Insert your password: ");
    fgets(password, sizeof(password), stdin);
    send(client_socket, password, sizeof(password), 0);

    close(client_socket);
    
    return 0;
}