#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>

#define addr "127.0.0.1"
#define port "9990"

int main(int args, char** argv) {

	int request_fd;

	char* received_msg = (char*)malloc(256);
	char* sended_msg = "Hello, im a client";

	short* addrport;

	struct sockaddr_in server, subserver;

	socklen_t length;

	request_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (request_fd == -1) {
		perror("Socket create error");
		exit(EXIT_FAILURE);
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(addr);
	server.sin_port = htons(atoi(port));

	sendto(request_fd, sended_msg, strlen(sended_msg), 0, (struct sockaddr*)&server, sizeof(struct sockaddr_in));

	printf("Waiting for massege...\n");
	memset(&subserver, '\0', sizeof(subserver));
	recvfrom(request_fd, received_msg, sizeof(received_msg), 0, (struct sockaddr*)&server, sizeof(struct sockaddr_in));

	addrport = received_msg;

	printf("Massege received: %d\n", *addrport);

	server.sin_port = htons(*addrport);

	sendto(request_fd, sended_msg, strlen(sended_msg), 0, (struct sockaddr*)&server, sizeof(struct sockaddr_in)); //отправили сообщение для подтверждения соединения на новый порт

	recvfrom(request_fd, received_msg, 256, 0, (struct sockaddr*)&server, sizeof(struct sockaddr_in)); // получение строки времени

	printf("Time is: %s\n", received_msg);

	exit(EXIT_SUCCESS);
}

