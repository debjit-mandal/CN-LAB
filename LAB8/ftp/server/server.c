#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>


void list_files(int new_fd) {
    DIR *d;
    struct dirent *dir;
    char file_list[1024] = {0};
    
    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG) {
                strcat(file_list, dir->d_name);
                strcat(file_list, "\n");
            }
        }
        closedir(d);
    }
    
    send(new_fd, file_list, strlen(file_list), 0);
}

void send_file(int new_fd, char *file_name) {
    printf("Attempting to open file: %s\n", file_name);
    FILE *file = fopen(file_name, "r");
    char buffer[1024] = {0};
    ssize_t bytes_read;

    if (file == NULL) {
        printf("File '%s' not found on server.\n", file_name);
        strcpy(buffer, "File not found on server.\n");
        send(new_fd, buffer, strlen(buffer), 0);
        return;
    }

    strcpy(buffer, "File found. Starting download...\n");
    send(new_fd, buffer, strlen(buffer), 0);

    while ((bytes_read = fread(buffer, 1, 1024, file)) > 0) {
        send(new_fd, buffer, bytes_read, 0);
        memset(buffer, 0, 1024);
    }

    fclose(file);
    printf("File '%s' sent successfully.\n", file_name);
}

int main(void) {
    int listen_fd, new_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size;
    char buffer[1024];
    
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
    }

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(50000);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero), 8);

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("Bind failed");
        exit(1);
    }

    if (listen(listen_fd, 10) == -1) {
        perror("Listen failed");
        exit(1);
    }

    printf("Server is listening on port 50000...\n");

    while (1) {
        sin_size = sizeof(struct sockaddr_in);
        new_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &sin_size);
        if (new_fd == -1) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected\n");

        list_files(new_fd);

        recv(new_fd, buffer, 1024, 0);
        buffer[strcspn(buffer, "\n")] = '\0';

        printf("Client requested file: %s\n", buffer);

        send_file(new_fd, buffer);

        close(new_fd);
    }

    close(listen_fd);
    return 0;
}
