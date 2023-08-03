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
#define port 7782zz

int main(int args, char** argv) {

	int request_fd;

	char* received_msg = (char*)malloc(256);
	char* sended_msg = "Hello, im a client";

	short int* addrport;
	int* received_time;
	int hour, min, sec;

	struct sockaddr_in server;

	socklen_t length;

	request_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (request_fd == -1) {
		perror("Socket create error");
		exit(EXIT_FAILURE);
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(addr);
	server.sin_port = htons(port);

	sendto(request_fd, sended_msg, strlen(sended_msg), 0, (struct sockaddr*)&server, sizeof(struct sockaddr_in));

	printf("Waiting for massege...\n");
	recvfrom(request_fd, received_msg, strlen(received_msg), 0, (struct sockaddr*)&server, sizeof(struct sockaddr_in));
	printf("Massege received\n");

	addrport = received_msg;
	server.sin_port = *addrport;

	sendto(request_fd, sended_msg, strlen(sended_msg), 0, (struct sockaddr*)&server, sizeof(struct sockaddr_in)); //отправили сообщение для подтверждения соединения на новый порт

	recvfrom(request_fd, received_msg, sizeof(received_msg), 0, (struct sockaddr*)&server, sizeof(struct sockaddr_in)); // получение строки времени
	
	received_time = received_msg;
	hour = (int)*received_time;
	received_time++;
	min = (int)*received_time;
	received_time++;
	sec = (int)*received_time;

	printf("Time is: %d:%d:%d\n", hour, min,sec);

	exit(EXIT_SUCCESS);
}

