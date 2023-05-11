#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#define MAX_MSG_LEN 1024

void client_side(char *IP, char *PORT);
void server_side(char *PORT);
int generate_file();
unsigned char cecksum__(char *filename);
void client_p(char* argv[]);
void server_p(char* argv[],int q);

void client_uds_dgram();

void client_uds_stream();

void client_mmap(char *filename);

void client_pipe(char *filename);

void client_ipv4_tcp(char *ip, char *port);

void client_ipv4_udp(char *ip, char *port);

void client_ipv6_tcp(char *ip, char *port);

void client_ipv6_udp(char *ip, char *port);

void server_ipv4_tcp(char* argv[]);

void server_ipv4_udp(char* argv[]);

void server_ipv6_tcp(char* argv[]);

void server_ipv6_udp(char* argv[]);

void server_uds_dgram(char* argv[]);

void server_uds_stream(char* argv[]);

int main(int argc, char* argv[]){

    if (argc > 7 || argc < 3) {
        printf("The usage for client is: stnc -c <IP PORT>\n");
        printf("The usage for server is: stnc -s <PORT>\n");
        return 1;
    }
    int p = 0;
    int q = 0;
    for (int i = 0; i < argc; ++i) {
        if(strcmp(argv[i],"-p")==0) {
            p=1;
        }
        if(strcmp(argv[i],"-q")==0) {
            q=1;
        }
    }

    if(strcmp(argv[1],"-c")==0){
        if(p!=1) {
            if (atoi(argv[3]) < 1024 || atoi(argv[3]) > 65535) {
                printf("Invalid PORT: %s\n", argv[3]);
                return 1;
            }
            client_side(argv[2], argv[3]);
            return 0;
        }
        client_p(argv);
        return 0;

    }

    if(strcmp(argv[1],"-s")==0){
        if (p!=1) {
            if (atoi(argv[2]) < 1024 || atoi(argv[2]) > 65535) {
                printf("Invalid PORT: %s\n", argv[3]);
                return 1;
            }
            server_side(argv[2]);
            return 0;
        }
        server_p(argv,q);
        return 0;
    }

    return 0;
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
int generate_file(){
    FILE *file = fopen("large_file.txt", "wb"); // Open file in binary write mode
    if (file == NULL) {
        printf("Error: Unable to create file.\n");
        return 1;
    }

    const long long SIZE = 100000000; // 100 MB in bytes 100000000
    const char BYTE = 'A'; // Arbitrary byte to write to the file

    for (long long i = 0; i < SIZE; i++) {
        fwrite(&BYTE, 1, 1, file); // Write 1 byte to file
    }

    fclose(file); // Close file
    return 0;
}
unsigned char cecksum__(char *filename){
    FILE *file = fopen(filename, "rb"); // Open file in binary read mode
    if (file == NULL) {
        printf("Error: Unable to open file.\n");
        return 0;
    }

    unsigned char temp = 0;
    unsigned char byte;
    int i =0;
    while (fread(&byte, 1, 1, file) == 1) {
        i++;
        if(i==500){
            break;
        }
        temp += byte;

    }


    fclose(file);
    return temp;
}
void client_p(char* argv[]){
    printf("client_p\n");
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(2727);

    if (connect(client_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("Failed to connect to server");
        exit(EXIT_FAILURE);
    }
    char buffer[1024];
    strcpy(buffer, argv[5]);
    strcat(buffer, " ");
    strcat(buffer, argv[6]);
    buffer[sizeof(argv[5]) + sizeof(argv[6]) + 1 ] = '\0';
    send(client_socket, buffer, strlen(buffer), 0);
    close(client_socket);

    if(strcmp(argv[5],"ipv4")==0){
        if(strcmp(argv[6],"tcp")==0){
            client_ipv4_tcp(argv[2],argv[3]);
        } else{
            client_ipv4_udp(argv[2],argv[3]);
        }
    }
    if(strcmp(argv[5],"ipv6")==0){
        if(strcmp(argv[6],"tcp")==0){
            client_ipv6_tcp(argv[2],argv[3]);
        } else{
            client_ipv6_udp(argv[2],argv[3]);
        }
    }
    if(strcmp(argv[5],"uds")==0){
        if(strcmp(argv[6],"dgram")==0){
            client_uds_dgram();
        } else{
            client_uds_stream();
        }
    }
    if(strcmp(argv[5],"mmap")==0){
        client_mmap(argv[6]);
    }
    if(strcmp(argv[5],"pipe")==0){
        client_pipe(argv[6]);
    }



}

void client_ipv6_udp(char *ip, char *port) {
    printf("client_ipv6_udp\n");
    sleep(1);
    int client_socket = socket(AF_INET6, SOCK_DGRAM, 0);

    if (client_socket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin6_family = AF_INET6;
    inet_pton(AF_INET6, ip, &(server_address.sin6_addr));
    server_address.sin6_port = htons(atoi(port));

    generate_file();
    char c;
    c = cecksum__("large_file.txt");
    if (sendto(client_socket, &c, sizeof(char), 0, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("Failed to send data to server");
        exit(EXIT_FAILURE);
    }

    // Open the file for reading
    FILE *file = fopen("large_file.txt", "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    size_t bytes_read;
    int bytes_sent;
    socklen_t server_address_size = sizeof(server_address);
    while ((bytes_read = fread(buffer, 1, 1024, file)) > 0) {
        bytes_sent = sendto(client_socket, buffer, bytes_read, 0, (struct sockaddr*) &server_address, server_address_size);
        if (bytes_sent < 0 || bytes_sent != bytes_read) {
            perror("Failed to send data to server");
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);
    close(client_socket);
}

void client_ipv6_tcp(char *ip, char *port) {
    sleep(1);
    int client_socket = socket(AF_INET6, SOCK_STREAM, 0);

    if (client_socket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin6_family = AF_INET6;
    inet_pton(AF_INET6, ip, &server_address.sin6_addr);
    server_address.sin6_port = htons(atoi(port));

    if (connect(client_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("Failed to connect to server");
        exit(EXIT_FAILURE);
    }
    generate_file();
    char c;
    c = cecksum__("large_file.txt");
    if (send(client_socket, &c, sizeof(char), 0) < 0) {
        perror("Failed to send data to server");
        exit(EXIT_FAILURE);
    }
    // Open the file for reading
    FILE *file = fopen("large_file.txt", "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, 1024, file)) > 0) {
        if (send(client_socket, buffer, bytes_read, 0) < 0) {
            perror("Failed to send data to server");
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);
    close(client_socket);
}


void client_ipv4_udp(char *ip, char *port) {
    printf("client_ipv4_udp\n");
    sleep(1);
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (client_socket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = htons(atoi(port));

    generate_file();
    char c;
    c = cecksum__("large_file.txt");
    if (sendto(client_socket, &c, sizeof(char), 0, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("Failed to send data to server");
        exit(EXIT_FAILURE);
    }

    // Open the file for reading
    FILE *file = fopen("large_file.txt", "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    size_t bytes_read;
    int bytes_sent;
    socklen_t server_address_size = sizeof(server_address);
    while ((bytes_read = fread(buffer, 1, 1024, file)) > 0) {
        bytes_sent = sendto(client_socket, buffer, bytes_read, 0, (struct sockaddr*) &server_address, server_address_size);
        if (bytes_sent < 0 || bytes_sent != bytes_read) {
            perror("Failed to send data to server");
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);
    close(client_socket);
}

void client_ipv4_tcp(char *ip, char *port) {
    sleep(1);
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = htons(atoi(port));
    printf("port : %d\n",server_address.sin_port);

    if (connect(client_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("Failed to connect to server");
        exit(EXIT_FAILURE);
    }
    generate_file();
    char c;
    c = cecksum__("large_file.txt");
    if (send(client_socket, &c, sizeof(char), 0) < 0) {
        perror("Failed to send data to server");
        exit(EXIT_FAILURE);
    }
    // Open the file for reading
    FILE *file = fopen("large_file.txt", "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, 1024, file)) > 0) {
        if (send(client_socket, buffer, bytes_read, 0) < 0) {
            perror("Failed to send data to server");
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);
    close(client_socket);
}

void client_pipe(char *filename) {

}

void client_mmap(char *filename) {

}

void client_uds_stream() {

}

void client_uds_dgram() {

}









void server_p(char* argv[],int q){
    printf("server_p\n");
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(2727);

    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }


    if (listen(server_socket, 5) < 0) {
        perror("Failed to listen on socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    int client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_size);

    if (client_socket < 0) {
        perror("Failed to accept client connection");
        exit(EXIT_FAILURE);
    }
    char buffer[1024];
    int bytes_received = recv(client_socket, &buffer, sizeof(buffer), 0);
    if (bytes_received < 0) {
        perror("Error receiving character");
        exit(1);
    }
    buffer[bytes_received] = '\0';  // Null-terminate the received data
    if(strcmp(buffer,"ipv4 tcp")==0){
        server_ipv4_tcp(argv);
    }
    if(strcmp(buffer,"ipv4 udp")==0){
        server_ipv4_udp(argv);
    }
    if(strcmp(buffer,"ipv6 tcp")==0){
        server_ipv6_tcp(argv);
    }
    if(strcmp(buffer,"ipv6 udp")== 0){
        server_ipv6_udp(argv);
    }
    if(strcmp(buffer,"uds dgram")==0){
        server_uds_dgram(argv);
    }
    if(strcmp(buffer,"uds stream")==0){
        server_uds_stream(argv);
    }
    close(server_socket);
}

void server_uds_stream(char* argv[]) {

}

void server_uds_dgram(char* argv[]) {

}

void server_ipv6_udp(char* argv[]) {
    printf("server_ipv6_udp\n");
    int server_socket = socket(AF_INET6, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 server_address, client_address;
    memset(&server_address, 0, sizeof(server_address));
    memset(&client_address, 0, sizeof(client_address));

    server_address.sin6_family = AF_INET6;
    server_address.sin6_addr = in6addr_any;
    server_address.sin6_port = htons(atoi(argv[2]));

    struct timeval timeout;
    timeout.tv_sec = 3; // 5 seconds timeout
    timeout.tv_usec = 0;

    if (setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    // Receive a check sum
    char c;
    socklen_t client_address_size = sizeof(client_address);
    if (recvfrom(server_socket, &c, sizeof(char), 0, (struct sockaddr*) &client_address, &client_address_size) < 0) {
        perror("Failed to receive data from client");
        exit(EXIT_FAILURE);
    }

    // Open the file for writing
    FILE *file = fopen("file_received.txt", "wb");
    if (file == NULL) {
        perror("Failed to create file");
        exit(EXIT_FAILURE);
    }

    // Receive the file data from the client and write it to the file
    char buffer[1024];
    ssize_t bytes_received;
    struct timespec start_time, end_time;
    double elapsed_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    while (1) {
        if((bytes_received = recvfrom(server_socket, buffer, 1024, 0, (struct sockaddr*) &client_address, &client_address_size)) > 0){
            if(strcmp(buffer,"end")==0) break;
            if (fwrite(buffer, 1, bytes_received, file) != bytes_received) {
                perror("Failed to write to file");
                exit(EXIT_FAILURE);
            }
        }
        if (bytes_received < 0){
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                break;
            }else {
                perror("recvfrom failed");
                exit(EXIT_FAILURE);
            }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    fclose(file);
    elapsed_time = (end_time.tv_sec - start_time.tv_sec - 3) * 1000.0 ; // seconds to milliseconds
    elapsed_time += (end_time.tv_nsec - start_time.tv_nsec) / 1000000.0; // nanoseconds to milliseconds
    struct stat file1_stat, file2_stat;
    const char* file1_path = "file_received.txt";
    const char* file2_path = "large_file.txt";
    // Get the size of file1
    if (stat(file1_path, &file1_stat) < 0) {
        perror("Failed to get file1 size");
        exit(EXIT_FAILURE);
    }

    // Get the size of file2
    if (stat(file2_path, &file2_stat) < 0) {
        perror("Failed to get file2 size");
        exit(EXIT_FAILURE);
    }
    char c__;
    c__ = cecksum__("file_received.txt");
    // Compare the sizes of the files
    if (file1_stat.st_size != file2_stat.st_size) {
        printf("The data was not transferred successfully, so the test is not accurate\n");
    }else if(c==c__){
        printf("ipv4_udp,%f\n",elapsed_time);

    } else{
        printf("The data was not transferred successfully, so the test is not accurate\n");
    }
    // Close the file and the socket
    close(server_socket);


}

void server_ipv6_tcp(char* argv[]) {
    int server_socket = socket(AF_INET6, SOCK_STREAM, 0);

    if (server_socket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin6_family = AF_INET6;
    server_address.sin6_addr = in6addr_any;
    server_address.sin6_port = htons(atoi(argv[2]));



    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }
    if (listen(server_socket, 5) < 0) {
        perror("Failed to listen on socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 client_address;
    socklen_t client_address_size = sizeof(client_address);
    int client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_size);

    if (client_socket < 0) {
        perror("Failed to accept client connection");
        exit(EXIT_FAILURE);
    }

    // Receive a check sum
    char c;
    if (recv(client_socket, &c, sizeof(char), 0) < 0) {
        perror("Failed to receive data from client");
        exit(EXIT_FAILURE);
    }

    // Open the file for writing
    FILE *file = fopen("file_received.txt", "wb");
    if (file == NULL) {
        perror("Failed to create file");
        exit(EXIT_FAILURE);
    }

    // Receive the file data from the client and write it to the file
    char buffer[1024];
    ssize_t bytes_received;
    struct timespec start_time, end_time;
    double elapsed_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    while ((bytes_received = recv(client_socket, buffer, 1024, 0)) > 0) {
        if (fwrite(buffer, 1, bytes_received, file) != bytes_received) {
            perror("Failed to write to file");
            exit(EXIT_FAILURE);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    fclose(file);
    elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0; // seconds to milliseconds
    elapsed_time += (end_time.tv_nsec - start_time.tv_nsec) / 1000000.0; // nanoseconds to milliseconds

    if (bytes_received < 0) {
        perror("Failed to receive data from client");
        exit(EXIT_FAILURE);
    }

    char c__;
    c__ = cecksum__("file_received.txt");
    if(c==c__){
        printf("ipv6_tcp,%f\n",elapsed_time);

    } else{
        printf("The data was not transferred successfully, so the test is not accurate\n");
    }
    // Close the file and the socket
    close(server_socket);
}



void server_ipv4_tcp(char* argv[]) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(atoi(argv[2]));

    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }
    if (listen(server_socket, 5) < 0) {
        perror("Failed to listen on socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    int client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_size);

    if (client_socket < 0) {
        perror("Failed to accept client connection");
        exit(EXIT_FAILURE);
    }
    // Receive a check sum
    char c;
    if (recv(client_socket, &c, sizeof(char), 0) < 0) {
        perror("Failed to receive data from client");
        exit(EXIT_FAILURE);
    }
// Open the file for writing
    FILE *file = fopen("file_received.txt", "wb");
    if (file == NULL) {
        perror("Failed to create file");
        exit(EXIT_FAILURE);
    }

// Receive the file data from the client and write it to the file
    char buffer[1024];
    ssize_t bytes_received;
    struct timespec start_time, end_time;
    double elapsed_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    while ((bytes_received = recv(client_socket, buffer, 1024, 0)) > 0) {
        if (fwrite(buffer, 1, bytes_received, file) != bytes_received) {
            perror("Failed to write to file");
            exit(EXIT_FAILURE);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    fclose(file);
    elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0; // seconds to milliseconds
    elapsed_time += (end_time.tv_nsec - start_time.tv_nsec) / 1000000.0; // nanoseconds to milliseconds

    if (bytes_received < 0) {
        perror("Failed to receive data from client");
        exit(EXIT_FAILURE);
    }
    char c__;
    c__ = cecksum__("file_received.txt");
    if(c==c__){
        printf("ipv4_tcp,%f\n",elapsed_time);

    } else{
        printf("The data was not transferred successfully, so the test is not accurate\n");
    }
    // Close the file and the socket
    close(server_socket);


}
void server_ipv4_udp(char* argv[]) {
    printf("server_ipv4_udp\n");
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address, client_address;
    memset(&server_address, 0, sizeof(server_address));
    memset(&client_address, 0, sizeof(client_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(atoi(argv[2]));
    struct timeval timeout;
    timeout.tv_sec = 3; // 5 seconds timeout
    timeout.tv_usec = 0;

    if (setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    // Receive a check sum
    char c;
    socklen_t client_address_size = sizeof(client_address);
    if (recvfrom(server_socket, &c, sizeof(char), 0, (struct sockaddr*) &client_address, &client_address_size) < 0) {
        perror("Failed to receive data from client");
        exit(EXIT_FAILURE);
    }

    // Open the file for writing
    FILE *file = fopen("file_received.txt", "wb");
    if (file == NULL) {
        perror("Failed to create file");
        exit(EXIT_FAILURE);
    }

    // Receive the file data from the client and write it to the file
    char buffer[1024];
    ssize_t bytes_received;
    struct timespec start_time, end_time;
    double elapsed_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    while (1) {
        if((bytes_received = recvfrom(server_socket, buffer, 1024, 0, (struct sockaddr*) &client_address, &client_address_size)) > 0){
            if(strcmp(buffer,"end")==0) break;
            if (fwrite(buffer, 1, bytes_received, file) != bytes_received) {
                perror("Failed to write to file");
                exit(EXIT_FAILURE);
            }
        }
        if (bytes_received < 0){
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                break;
            }else {
                perror("recvfrom failed");
                exit(EXIT_FAILURE);
            }
        }


    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    fclose(file);
    elapsed_time = (end_time.tv_sec - start_time.tv_sec - 3) * 1000.0 ; // seconds to milliseconds
    elapsed_time += (end_time.tv_nsec - start_time.tv_nsec) / 1000000.0; // nanoseconds to milliseconds
    struct stat file1_stat, file2_stat;
    const char* file1_path = "file_received.txt";
    const char* file2_path = "large_file.txt";

    // Get the size of file1
    if (stat(file1_path, &file1_stat) < 0) {
        perror("Failed to get file1 size");
        exit(EXIT_FAILURE);
    }

    // Get the size of file2
    if (stat(file2_path, &file2_stat) < 0) {
        perror("Failed to get file2 size");
        exit(EXIT_FAILURE);
    }
    char c__;
    c__ = cecksum__("file_received.txt");
    // Compare the sizes of the files
    if (file1_stat.st_size != file2_stat.st_size) {
        printf("The data was not transferred successfully, so the test is not accurate\n");
    }else if(c==c__){
        printf("ipv4_udp,%f\n",elapsed_time);

    } else{
        printf("The data was not transferred successfully, so the test is not accurate\n");
    }
    // Close the file and the socket
    close(server_socket);
}
