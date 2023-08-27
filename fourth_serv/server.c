#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUFF_SIZE 1024
#define SERVER_IP "127.0.0.1"
#define PORT 8080

pthread_mutex_t mutex;

int get_free_port() {
    int sockfd;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = 0;

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Error binding socket");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (getsockname(sockfd, (struct sockaddr*)&addr, &addr_len) < 0) {
        perror("Error getting port");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    int free_port = ntohs(addr.sin_port);

    close(sockfd);

    return free_port;
}

int time_get(char* buffer) {
    time_t current_time;
    current_time = time(NULL);

    if (current_time == (time_t)-1) {
        perror("Error getting time");
        return 1;
    }
    char* time_msg = ctime(&current_time);

    strcpy(buffer, time_msg);
    return 0;
}

void* new_srv_udp(void* thread_data) {
    int* port = (int*)thread_data;
    int srv_sock;
    struct sockaddr_in srv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    char buffer[BUFF_SIZE];

    printf("%d:new_srv - port: %d\n", *port, *port);
    srv_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (srv_sock == -1) {
        perror("Error creating socket\n");
        exit(EXIT_FAILURE);
    }

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(*port);
    inet_pton(AF_INET, SERVER_IP, &(srv_addr.sin_addr));

    if (bind(srv_sock, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) == -1) {
        perror("Error binding socket\n");
        close(srv_sock);
        exit(EXIT_FAILURE);
    }

    pthread_mutex_unlock(&mutex);

    int received_bytes = recvfrom(srv_sock, buffer, BUFF_SIZE, 0,
        (struct sockaddr*)&cli_addr, &cli_len);
    if (received_bytes == -1) {
        perror("Error accepting connection\n");
        close(srv_sock);
        exit(EXIT_FAILURE);
    }

    time_get(buffer);

    if (sendto(srv_sock, buffer, BUFF_SIZE, 0, (struct sockaddr*)&cli_addr,
        sizeof(cli_addr)) < 0) {
        perror("Error sending response\n");
        close(srv_sock);
        exit(EXIT_FAILURE);
    }

    close(srv_sock);
    return 0;
}

void* new_srv_tcp(void* thread_data) {
    int* port = (int*)thread_data;
    int srv_sock, cli_sock;
    struct sockaddr_in srv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    char buffer[BUFF_SIZE];

    printf("%d:new_srv - port: %d\n", *port, *port);
    srv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (srv_sock == -1) {
        perror("Error creating socket\n");
        exit(EXIT_FAILURE);
    }

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(*port);
    inet_pton(AF_INET, SERVER_IP, &(srv_addr.sin_addr));

    if (bind(srv_sock, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) == -1) {
        perror("Error binding socket\n");
        close(srv_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(srv_sock, 5) == -1) {
        perror("Listening error");
        close(srv_sock);
        exit(EXIT_FAILURE);
    }

    pthread_mutex_unlock(&mutex);

    while (1) {
        cli_sock = accept(srv_sock, (struct sockaddr*)&cli_addr, &cli_len);
        if (cli_sock == -1) {
            perror("Error accepting connection");
            continue;
        }

        int received_bytes = recv(cli_sock, buffer, BUFF_SIZE, 0);
        if (received_bytes == -1) {
            perror("Error accepting connection\n");
            close(cli_sock);
            close(srv_sock);
            exit(EXIT_FAILURE);
        }

        time_get(buffer);

        if (send(cli_sock, buffer, BUFF_SIZE, 0) < 0) {
            perror("Error sending response\n");
            close(cli_sock);
            close(srv_sock);
            exit(EXIT_FAILURE);
        }
    }

    close(srv_sock);
    return 0;
}

int main() {
    fd_set read_fds;
    int max_fd;

    int client_count = 0;
    int srv_sock_udp, srv_sock_tcp, cli_sock_tcp;
    int port;
    struct sockaddr_in srv_addr, cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    pthread_t tid;
    pthread_mutex_init(&mutex, NULL);

    char buffer[BUFF_SIZE];

    // Addr
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &(srv_addr.sin_addr));

    // UDP
    srv_sock_udp = socket(AF_INET, SOCK_DGRAM, 0);
    if (srv_sock_udp == -1) {
        perror("Error creating socket\n");
        exit(EXIT_FAILURE);
    }

    if (bind(srv_sock_udp, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) ==
        -1) {
        perror("Error binding socket\n");
        close(srv_sock_udp);
        exit(EXIT_FAILURE);
    }

    // TCP
    srv_sock_tcp = socket(AF_INET, SOCK_STREAM, 0);
    if (srv_sock_tcp == -1) {
        perror("Error creating socket\n");
        exit(EXIT_FAILURE);
    }

    if (bind(srv_sock_tcp, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) ==
        -1) {
        perror("Error binding socket\n");
        close(srv_sock_tcp);
        exit(EXIT_FAILURE);
    }

    if (listen(srv_sock_tcp, 5) == -1) {
        perror("Listening error");
        close(srv_sock_tcp);
        exit(EXIT_FAILURE);
    }

    printf("The server is waiting for a connection...\n");

    while (1) {
        tid = 0;

        FD_ZERO(&read_fds);
        FD_SET(srv_sock_udp, &read_fds);
        FD_SET(srv_sock_tcp, &read_fds);

        max_fd = (srv_sock_tcp > srv_sock_udp) ? srv_sock_tcp : srv_sock_udp;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("Select error");
            close(srv_sock_tcp);
            exit(EXIT_FAILURE);
        }

        // UDP Ready
        if (FD_ISSET(srv_sock_udp, &read_fds)) {
            printf("UDP socket is ready for reading.\n");

            int received_bytes = recvfrom(srv_sock_udp, buffer, BUFF_SIZE, 0,
                (struct sockaddr*)&cli_addr, &cli_len);
            if (received_bytes == -1) {
                perror("Error accepting connection\n");
                close(srv_sock_udp);
                exit(EXIT_FAILURE);
            }

            buffer[received_bytes] = '\0';
            client_count++;
            printf("Received from client %d: %s\n", client_count, buffer);

            port = get_free_port();

            pthread_mutex_lock(&mutex);

            if (pthread_create(&tid, NULL, new_srv_udp, &port) != 0) {
                perror("pthread_create");
                return 1;
            }

            pthread_mutex_lock(&mutex);

            if (sendto(srv_sock_udp, &port, sizeof(int), 0,
                (struct sockaddr*)&cli_addr, sizeof(cli_addr)) < 0) {
                perror("Error sending response");
                close(srv_sock_udp);
                exit(EXIT_FAILURE);
            }

            pthread_mutex_unlock(&mutex);
        }

        // TCP Ready
        if (FD_ISSET(srv_sock_tcp, &read_fds)) {
            printf("TCP socket is ready for reading.\n");

            cli_sock_tcp =
                accept(srv_sock_tcp, (struct sockaddr*)&cli_addr, &cli_len);
            if (cli_sock_tcp == -1) {
                perror("Error accepting connection");
                continue;
            }

            int received_bytes = recv(cli_sock_tcp, buffer, BUFF_SIZE, 0);
            if (received_bytes == -1) {
                perror("Error accepting connection\n");
                close(srv_sock_tcp);
                exit(EXIT_FAILURE);
            }

            buffer[received_bytes] = '\0';
            client_count++;
            printf("Received from client %d: %s\n", client_count, buffer);

            port = get_free_port();

            pthread_mutex_lock(&mutex);

            if (pthread_create(&tid, NULL, new_srv_tcp, &port) != 0) {
                perror("pthread_create");
                return 1;
            }

            pthread_mutex_lock(&mutex);

            if (send(cli_sock_tcp, &port, sizeof(int), 0) < 0) {
                perror("Error sending response");
                close(srv_sock_tcp);
                exit(EXIT_FAILURE);
            }

            pthread_mutex_unlock(&mutex);
        }
    }

    close(srv_sock_udp);
    close(srv_sock_tcp);
    return 0;
}