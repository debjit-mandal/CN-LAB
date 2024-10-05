#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


int main(int argc, char *argv[]) {
    int sockfd, numbytes;
    char buffer[1024], file_name[256];
    struct sockaddr_in server_addr;
    FILE *file;

    if (argc != 3) {
        printf("Usage: %s <Server IP Address> <port>\n", argv[0]);
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    bzero(&(server_addr.sin_zero), 8);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("Connect failed");
        exit(1);
    }

    printf("Connected to the server.\n");

    numbytes = recv(sockfd, buffer, 1024, 0);
    if (numbytes == -1) {
        perror("Receive failed");
        exit(1);
    }

    buffer[numbytes] = '\0';
    printf("Files on server:\n%s", buffer);

    printf("Enter the file name to download: ");
    fgets(file_name, sizeof(file_name), stdin);
    file_name[strcspn(file_name, "\n")] = '\0'; 

    send(sockfd, file_name, strlen(file_name), 0);

    numbytes = recv(sockfd, buffer, 1024, 0);
    if (numbytes == -1) {
        perror("Receive failed");
        exit(1);
    }
    buffer[numbytes] = '\0';
    printf("%s", buffer);

    if (strncmp(buffer, "File found", 10) == 0) {
        file = fopen(file_name, "w");
        if (file == NULL) {
            perror("File open failed");
            exit(1);
        }

        while ((numbytes = recv(sockfd, buffer, 1024, 0)) > 0) {
            fwrite(buffer, sizeof(char), numbytes, file);
        }

        fclose(file);
        printf("File '%s' downloaded successfully.\n", file_name);
    }

    close(sockfd);
    return 0;
}
