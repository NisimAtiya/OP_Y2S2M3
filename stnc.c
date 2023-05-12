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
#include <sys/un.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#define MAX_MSG_LEN 1024
#define SOCK_PATH "./mysocket.sock"

void client_side(char *IP, char *PORT);
void server_side(char *PORT);
int generate_file();
unsigned char cecksum__(char *filename);
void client_p(char* argv[]);
void server_p(char* argv[],int q);

void client_uds_dgram(char* argv[]);

void client_uds_stream(char* argv[]);

void client_mmap(char* argv[]);

void client_pipe(char* argv[]);

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

void server_mmap(char *argv);

void server_pipe(char *argv);

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
            client_uds_dgram(argv);
        } else{
            client_uds_stream(argv);
        }
    }
    if(strcmp(argv[5],"mmap")==0){
        client_mmap(argv);
    }
    if(strcmp(argv[5],"pipe")==0){
        client_pipe(argv);
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

void client_pipe(char* argv[]) {
    int fd;
    char* buffer;
    ssize_t bytes_read;
    generate_file();
    // Open the named pipe for writing
    fd = open(argv[6], O_WRONLY);
    if (fd < 0) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Read the file into a buffer
    FILE *fp;
    if ((fp = fopen("large_file.txt", "rb")) == NULL) {
        perror("fopen");
        exit(1);
    }
    buffer = (char*)malloc(104857600);

    bytes_read = fread(buffer, 1, 104857600, fp);

    // Write the buffer to the named pipe
    write(fd, buffer, bytes_read);


    // Close the named pipe and file
    close(fd);
    fclose(fp);
    free(buffer);
}

void client_mmap(char* argv[]) {
    printf("client_mmap\n");
    long file_size;
    size_t result;


    generate_file();
    FILE *file = fopen("large_file.txt", "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);


    char *buffer = (char *) malloc(sizeof(char) * file_size);

    int fd = open(argv[6], O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Resize the file to the length of the data
    if (ftruncate(fd, file_size) < 0) {
        perror("Failed to resize file");
        exit(EXIT_FAILURE);
    }

    // Map the file to memory
    char *ptr = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Failed to map file");
        exit(EXIT_FAILURE);
    }

    // Copy the file into the buffer
    result = fread(buffer, 1, file_size, file);
    if (result != file_size) {
        perror("Failed to read file");
        exit(EXIT_FAILURE);
    }

    // Write the data to the mapped memory
    memcpy(ptr, buffer, file_size);

    // Unmap the memory and close the file
    if (munmap(ptr, file_size) < 0) {
        perror("Failed to unmap file");
        exit(EXIT_FAILURE);
    }
    // Free the buffer
    free(buffer);
    close(fd);

}

void client_uds_stream(char* argv[]){
    sleep(1);
    printf("client_uds_stream\n");

    int sockfd, len;
    struct sockaddr_un remote;
    char buf[BUFSIZ];

    // create a UDS stream socket
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // set the remote address and length
    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);

    // connect to the remote socket
    if (connect(sockfd, (struct sockaddr *)&remote, len) == -1) {
        perror("connect");
        exit(1);
    }

    generate_file();
    char c;
    c = cecksum__("large_file.txt");
    if (send(sockfd, &c, sizeof(c), 0) == -1) {
        perror("send");
        exit(1);
    }

    // open the file to be sent
    FILE *fp;
    if ((fp = fopen("large_file.txt", "rb")) == NULL) {
        perror("fopen");
        exit(1);
    }

    // read the file data and send it to the remote socket
    while (fgets(buf, BUFSIZ, fp) != NULL) {
        if (send(sockfd, buf, strlen(buf), 0) == -1) {
            perror("send");
            exit(1);
        }
    }

    // close the file and socket
    fclose(fp);
    close(sockfd);
}




void client_uds_dgram(char* argv[]){

    int sockfd, len;
    struct sockaddr_un remote;
    char buf[BUFSIZ];

    // create a UDS datagram socket
    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // set the remote address and length
    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);

    generate_file();
    char c;
    c = cecksum__("large_file.txt");
    if (sendto(sockfd, &c, sizeof(c), 0, (struct sockaddr *)&remote, len) == -1) {
        perror("sendto");
        exit(1);
    }

    // open the file to be sent
    FILE *fp;
    if ((fp = fopen("large_file.txt", "rb")) == NULL) {
        perror("fopen");
        exit(1);
    }


    // read the file data and send it to the remote socket
    while (fgets(buf, BUFSIZ, fp) != NULL) {
        if (sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&remote, len) == -1) {
            perror("sendto");
            exit(1);
        }
    }

    // close the file and socket
    fclose(fp);
    close(sockfd);
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
    else if(strcmp(buffer,"ipv4 udp")==0){
        server_ipv4_udp(argv);
    }
    else if(strcmp(buffer,"ipv6 tcp")==0){
        server_ipv6_tcp(argv);
    }
    else if(strcmp(buffer,"ipv6 udp")== 0){
        server_ipv6_udp(argv);
    }
    else if(strcmp(buffer,"uds dgram")==0){
        server_uds_dgram(argv);
    }
    else if(strcmp(buffer,"uds stream")==0){
        server_uds_stream(argv);
    }
    char buffer2[20];
    strcpy(buffer2, buffer + 5);
    strncpy(buffer, buffer, 4);
    buffer[4] = '\0'; // null-terminate the buffer
    if(strcmp(buffer,"mmap")==0){
        server_mmap(buffer2);
    }
    else if(strcmp(buffer,"pipe")==0){
        server_pipe(buffer2);
    }

    close(server_socket);
}

void server_pipe(char* argv) {
    long file_size;
    int fd;
    char* buffer = malloc(104857600 * sizeof(char)); // Allocate memory for the buffer
    ssize_t bytes_read;

    // Create the named pipe
    mkfifo(argv, 0666);

    // Open the named pipe for reading
    fd = open(argv, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Wait for the client to write data to the named pipe
    while ((bytes_read = read(fd, buffer, 104857600)) == 0) {
        usleep(1000); // Sleep for 1ms
    }

    // receive file data from the client and write it to a file
    FILE *fp;
    if ((fp = fopen("received_file.txt", "wb")) == NULL) {
        perror("fopen");
        exit(1);
    }
    struct timespec start_time, end_time;
    double elapsed_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    // Write the buffer to disk
    fwrite(buffer, bytes_read, 1, fp);
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    elapsed_time = (end_time.tv_sec - start_time.tv_sec ) * 1000.0 ; // seconds to milliseconds
    elapsed_time += (end_time.tv_nsec - start_time.tv_nsec) / 1000000.0; // nanoseconds to milliseconds

    // Close the named pipe and file
    close(fd);
    fclose(fp);
    free(buffer);

    // Remove the named pipe
    unlink(argv);

    printf("pipe,%f\n",elapsed_time);
}

void server_mmap(char* argv) {
    printf("server_mmap\n");
    long file_size;
    int fd;
    char* buffer;
    char* ptr;

    fd = open(argv, O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Resize the file to the length of the data
    if (ftruncate(fd, 104857600) < 0) {
        perror("Failed to resize file");
        exit(EXIT_FAILURE);
    }

    // Map the file to memory
    ptr = mmap(NULL, 104857600, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Failed to map file");
        exit(EXIT_FAILURE);
    }

    // Wait for the client to write data to the mapped memory
    printf("Waiting for data from client...\n");
    while (strlen(ptr) == 0) {
        usleep(1000); // Sleep for 1ms
    }

    // Copy the data from the mapped memory to a buffer
    file_size = strlen(ptr);
    buffer = (char*)malloc(file_size * sizeof(char));
    memcpy(buffer, ptr, file_size);

    // receive file data from the client and write it to a file
    FILE *fp;
    if ((fp = fopen("received_file.txt", "wb")) == NULL) {
        perror("fopen");
        exit(1);
    }
    ssize_t bytes_received;
    struct timespec start_time, end_time;
    double elapsed_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    // Write the buffer to disk
    fwrite(buffer, file_size, 1, fp);
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    elapsed_time = (end_time.tv_sec - start_time.tv_sec ) * 1000.0 ; // seconds to milliseconds
    elapsed_time += (end_time.tv_nsec - start_time.tv_nsec) / 1000000.0; // nanoseconds to milliseconds

    // Unmap the memory and close the file
    if (munmap(ptr, 104857600) < 0) {
        perror("Failed to unmap file");
        exit(EXIT_FAILURE);
    }
    close(fd);
    free(buffer);
    printf("mmap,%f\n",elapsed_time);
}

void server_uds_stream(char* argv[]) {
    printf("server_uds_stream\n");

    int sockfd, client_fd, len;
    struct sockaddr_un local, remote;
    char buf[BUFSIZ];

    // create a UDS stream socket
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // bind the socket to a local address
    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, SOCK_PATH);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);
    if (bind(sockfd, (struct sockaddr *)&local, len) == -1) {
        perror("bind");
        exit(1);
    }

    // listen for incoming connections
    if (listen(sockfd, 5) == -1) {
        perror("listen");
        exit(1);
    }

    // accept the first incoming connection
    len = sizeof(remote);
    if ((client_fd = accept(sockfd, (struct sockaddr *)&remote, &len)) == -1) {
        perror("accept");
        exit(1);
    }

    // Receive a check sum
    char c;
    if (recv(client_fd, &c, sizeof(c), 0) == -1) {
        perror("recv");
        exit(1);
    }

    // receive file data from the client and write it to a file
    FILE *fp;
    if ((fp = fopen("received_file.txt", "wb")) == NULL) {
        perror("fopen");
        exit(1);
    }
    ssize_t bytes_received;
    struct timespec start_time, end_time;
    double elapsed_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    while (1) {
        if ((bytes_received = recv(client_fd, buf, BUFSIZ - 1, 0)) == 0) {
            break;
        }
        if (fwrite(buf, 1, bytes_received, fp)!=bytes_received){
            perror("Failed to write to file");
            exit(EXIT_FAILURE);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    elapsed_time = (end_time.tv_sec - start_time.tv_sec ) * 1000.0 ; // seconds to milliseconds
    elapsed_time += (end_time.tv_nsec - start_time.tv_nsec) / 1000000.0; // nanoseconds to milliseconds

    char c__;
    c__ = cecksum__("received_file.txt");
    // Compare the sizes of the files
    if(c==c__){
        printf("uds_stream,%f\n",elapsed_time);

    } else{
        printf("The data was not transferred successfully, so the test is not accurate\n");
    }

    // close the socket
    fclose(fp);
    close(sockfd);
}


void server_uds_dgram(char* argv[]) {
    printf("server_uds_dgram\n");

    int sockfd, len;
    struct sockaddr_un local, remote;
    char buf[BUFSIZ];

    // create a UDS datagram socket
    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // bind the socket to a local address
    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, SOCK_PATH);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);
    if (bind(sockfd, (struct sockaddr *)&local, len) == -1) {
        perror("bind");
        exit(1);
    }
    // set the timeout for the socket
    struct timeval tv;
    tv.tv_sec = 3;  // set the timeout to 3 seconds
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    // Receive a check sum
    char c;
    if (recvfrom(sockfd, &c, sizeof(c), 0, (struct sockaddr *)&remote, &len) == -1) {
        perror("recvfrom");
        exit(1);
    }


    // receive file data from the client and write it to a file
    FILE *fp;
    if ((fp = fopen("received_file.txt", "wb")) == NULL) {
        perror("fopen");
        exit(1);
    }
    ssize_t bytes_received;
    struct timespec start_time, end_time;
    double elapsed_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    while (1) {
        len = sizeof(remote);
        if ((bytes_received = recvfrom(sockfd, buf, BUFSIZ - 1, 0, (struct sockaddr *)&remote, &len)) == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else {
                perror("recvfrom");
                exit(1);
            }
        }
        if (fwrite(buf, 1, bytes_received, fp)!=bytes_received){
            perror("Failed to write to file");
            exit(EXIT_FAILURE);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    fclose(fp);
    elapsed_time = (end_time.tv_sec - start_time.tv_sec - 3) * 1000.0 ; // seconds to milliseconds
    elapsed_time += (end_time.tv_nsec - start_time.tv_nsec) / 1000000.0; // nanoseconds to milliseconds
    struct stat file1_stat, file2_stat;
    const char* file1_path = "received_file.txt";
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
        printf("uds_dgram,%f\n",elapsed_time);

    } else{
        printf("The data was not transferred successfully, so the test is not accurate\n");
    }

    // close the socket
    close(sockfd);

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
    timeout.tv_sec = 3; // 3 seconds timeout
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