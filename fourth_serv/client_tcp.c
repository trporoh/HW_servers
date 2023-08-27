#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFF_SIZE 1024
#define SERVER_IP "127.0.0.1"
#define PORT 8080

int main() {
    int cli_sock, new_sock;
    struct sockaddr_in srv_addr, new_addr;
    char buffer[BUFF_SIZE];
    int port = 0;

    cli_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_sock == -1) {
        perror("Error creating socket\n");
        exit(EXIT_FAILURE);
    }

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &(srv_addr.sin_addr));

    new_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (new_sock == -1) {
        perror("Error creating socket\n");
        exit(EXIT_FAILURE);
    }

    char msg[] = "Hello";
    strcpy(buffer, msg);

    if (connect(cli_sock, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) == -1) {
        perror("Connection error");
        close(cli_sock);
        exit(EXIT_FAILURE);
    }

    if (send(cli_sock, msg, strlen(msg), 0) < 0) {
        perror("Error sending message");
        close(cli_sock);
        exit(EXIT_FAILURE);
    }

    if (recv(cli_sock, &port, sizeof(int), 0) < 0) {
        perror("Error receiving response");
        close(cli_sock);
        exit(EXIT_FAILURE);
    }
    char msg_new[BUFF_SIZE] = "Hello time\n";

    if (port != 0) {
        memset(&new_addr, 0, sizeof(new_addr));
        new_addr.sin_family = AF_INET;
        new_addr.sin_port = htons(port);
        inet_pton(AF_INET, SERVER_IP, &(new_addr.sin_addr));
    }

    if (connect(new_sock, (struct sockaddr*)&new_addr, sizeof(srv_addr)) == -1) {
        perror("Connection error");
        close(new_sock);
        exit(EXIT_FAILURE);
    }

    if (send(new_sock, msg_new, BUFF_SIZE, 0) < 0) {
        perror("Error sending message");
        close(new_sock);
        exit(EXIT_FAILURE);
    }

    int received_bytes = recv(new_sock, buffer, BUFF_SIZE, 0);
    if (received_bytes < 0) {
        perror("Error receiving response");
        close(new_sock);
        exit(EXIT_FAILURE);
    }

    buffer[received_bytes] = '\0';
    printf("Server response: %s\n", buffer);
    close(new_sock);

    close(cli_sock);
    return 0;
}