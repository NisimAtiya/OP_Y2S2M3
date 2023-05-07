#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_MSG_LEN 1024

void client_side(char *IP, char *PORT);

void server_side(char *PORT);

int main(int argc, char* argv[]){
    if (argc > 4 || argc < 3) {
        printf("The usage for client is: stnc -c <IP PORT>\n");
        printf("The usage for server is: stnc -s <PORT>\n");
        return 1;
    }
    if(strcmp(argv[1],"-c")==0){
        if (atoi(argv[3]) < 1024 || atoi(argv[3]) > 65535){
            printf("Invalid PORT: %s\n", argv[3]);
            return 1;
        }
        client_side(argv[2],argv[3]);
    }
    if(strcmp(argv[1],"-s")==0){
        if (atoi(argv[2]) < 1024 || atoi(argv[2]) > 65535){
            printf("Invalid PORT: %s\n", argv[3]);
            return 1;
        }
        server_side(argv[2]);
    }
    return 1;
}

void server_side(char *PORT) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(atoi(PORT));

    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Failed to listen on socket");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", ntohs(server_address.sin_port));
    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    int client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_address_size);

    if (client_socket < 0) {
        perror("Failed to accept client connection");
        exit(EXIT_FAILURE);
    }
    printf("Accepted connection from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
    struct pollfd pfds[2];
    pfds[0].fd = 0;          // Standard input
    pfds[0].events = POLLIN; // Tell me when ready to read
    pfds[0].revents = 0;
    pfds[1].fd = client_socket;
    pfds[1].events = POLLIN; // Tell me when ready to read
    pfds[1].revents = 0;
    while (1){
        int num_events = poll(pfds, 2, 50000); // 5 second timeout, 2 is the count of elements in the pfds array
        if (num_events == -1) { //error
            perror("poll");
            exit(1);
        }
        if(num_events==0){  //5 seconds passed
            continue;
        } else{
            if(pfds[0].revents & POLLIN){
                char msg[MAX_MSG_LEN];
                fgets(msg, MAX_MSG_LEN, stdin);
                send(client_socket, msg, strlen(msg), 0);
            }
            if(pfds[1].revents & POLLIN){
                char msg[MAX_MSG_LEN];
                ssize_t n = recv(client_socket, msg, MAX_MSG_LEN, 0);
                if (n <= 0){
                    printf("Disconnected from client\n");
                    break;
                }
                msg[n] = '\0';
                printf("\nClient: %s\n", msg);

            }
        }



        close(server_socket);
    }
}



void client_side(char *IP, char *PORT) {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(IP);
    server_address.sin_port = htons(atoi(PORT));

    if (connect(client_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("Failed to connect to server");
        exit(EXIT_FAILURE);
    }
    printf("connect to server\n");
    struct pollfd pfds[2];
    pfds[0].fd = 0;          // Standard input
    pfds[0].events = POLLIN; // Tell me when ready to read
    pfds[0].revents = 0;
    pfds[1].fd = client_socket;
    pfds[1].events = POLLIN; // Tell me when ready to read
    pfds[1].revents = 0;
    while (1){
        int num_events = poll(pfds, 2, 50000); // 5 second timeout, 2 is the count of elements in the pfds array
        if (num_events == -1) { //error
            perror("poll");
            exit(1);
        }
        if(num_events==0){  //5 seconds passed
            continue;
        } else{
                if(pfds[0].revents & POLLIN){
                    char msg[MAX_MSG_LEN];
                    fgets(msg, MAX_MSG_LEN, stdin);
                    send(client_socket, msg, strlen(msg), 0);
                }
                if(pfds[1].revents & POLLIN){
                    char msg[MAX_MSG_LEN];
                    ssize_t n = recv(client_socket, msg, MAX_MSG_LEN, 0);
                    if (n <= 0){
                        printf("Disconnected from server\n");
                        break;
                    }
                    msg[n] = '\0';
                    printf("\nServer: %s\n", msg);
                }

        }

    }

    close(client_socket);

}

