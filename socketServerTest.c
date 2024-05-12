#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>



int main(){

    // creo il sokcet
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    //Setsockopt
    //Faccio il setup del socket appena creato
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;
    
    int check = bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    printf("Ctrl+C to terminate the server.\n\n");

    while(1){
        printf("Server is online listening for connection...\n");
        listen(server_socket, 5);
        int client_socket = accept(server_socket, NULL, NULL);
        printf("Connection enstablished \nWrite message for client: ");
        char msg[256];
        fgets(msg, sizeof(msg), stdin);  // Read an integer from user input

        int msgLen = strlen(msg);
        // if (msgLen > 0 && msg[msgLen - 1] == '\n') {
        //      msg[msgLen - 1] = '\0'; // Replace '\n' with '\0' to terminate the string
        // }
        FILE *fp = fopen("serverResponses.txt", "a");
        if (fp != NULL) {
            fprintf(fp, "Server raw   : %s\n", msg);
            msg[strcspn(msg, "\n\r")] = 0; 
            fprintf(fp, "Server clean : %s\n", msg);
            fclose(fp);
        }
        
        send(client_socket, msg, strlen(msg), 0);
        printf("Sent the message \"%s\" to the client.\n closing connection.\n\n", msg);
    }
    

    

    close(server_socket);

    return 0;
}