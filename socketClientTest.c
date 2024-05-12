#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 256
int main(){

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;
    int check = connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    char server_response[BUFFER_SIZE];
    //recv(client_socket, &server_response, sizeof(server_response), 0);
    int valread;
    valread = recv(client_socket, server_response, BUFFER_SIZE - 1, 0);
    if (valread == -1) {
        perror("Receive error");
    } else if (valread == 0) {
        // Connection closed by server
        printf("Server closed the connection\n");
    } else {
        server_response[valread] = '\0'; // Null-terminate the received data
        printf("Server response: %s\n", server_response);
    }

    
    FILE *fp = fopen("serverResponses.txt", "a");
    if (fp != NULL) {
        fprintf(fp, "Simple Client: %s\n", server_response);
        fprintf(fp, "----------------\n");
        fclose(fp);
    }
    close(client_socket);

    return 0;
}