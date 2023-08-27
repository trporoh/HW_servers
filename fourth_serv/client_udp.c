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

    cli_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (cli_sock == -1) {
        perror("Error creating socket\n");
        exit(EXIT_FAILURE);
    }

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &(srv_addr.sin_addr));

    new_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (new_sock == -1) {
        perror("Error creating socket\n");
        exit(EXIT_FAILURE);
    }

    char msg[] = "Hello";
    strcpy(buffer, msg);

    if (sendto(cli_sock, buffer, BUFF_SIZE, 0, (struct sockaddr*)&srv_addr,
        sizeof(srv_addr)) < 0) {
        perror("Error sending message");
        close(cli_sock);
        exit(EXIT_FAILURE);
    }

    if (recvfrom(cli_sock, &port, sizeof(int), 0, NULL, NULL) < 0) {
        perror("Error receiving response");
        close(cli_sock);
        exit(EXIT_FAILURE);
    }

    if (port != 0) {
        memset(&new_addr, 0, sizeof(new_addr));
        new_addr.sin_family = AF_INET;
        new_addr.sin_port = htons(port);
        inet_pton(AF_INET, SERVER_IP, &(new_addr.sin_addr));
    }

    char msg_new[BUFF_SIZE] = "Hello time\n";

    if (sendto(new_sock, msg_new, BUFF_SIZE, 0, (struct sockaddr*)&new_addr,
        sizeof(new_addr)) < 0) {
        perror("Error sending message");
        close(cli_sock);
        exit(EXIT_FAILURE);
    }

    int received_bytes = recvfrom(new_sock, buffer, BUFF_SIZE, 0, NULL, NULL);
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