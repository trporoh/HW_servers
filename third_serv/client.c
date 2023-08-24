#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>

#define addr "127.0.0.1"
#define port "7783"

pthread_t threads[1000];

struct argv {

	struct sockaddr_in* serv;
	socklen_t* len;
	int fd;
	short* serv_port;
	char* send_msg;
	char* recv_msg;
};

void* clients(void* argv);

int main(int args, char** argv) {

	int fd[1000];

	struct argv datapthread[1000];

	for (int i = 0; i < 1000; i++) {

		datapthread[i].serv = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
		datapthread[i].serv->sin_family = AF_INET;
		datapthread[i].serv->sin_addr.s_addr = inet_addr(addr);
		datapthread[i].serv->sin_port = htons(atoi(port));
		datapthread[i].send_msg = (char*)malloc(32);
		datapthread[i].recv_msg = (char*)malloc(32);
		strcpy(datapthread[i].send_msg, "Hello, im a client!");

		datapthread[i].fd = socket(AF_INET, SOCK_DGRAM, 0);
		if (fd[i] == -1) {
			perror("Socket create error");
			exit(EXIT_FAILURE);
		}
	}
	for (int i = 0; i < 1000; i++) {

		pthread_create(&(threads[i]), 0, clients, (void*)&(datapthread[i]));
	}

	exit(EXIT_SUCCESS);
}

void* clients(void* argv) {

	if (-1 == sendto(((struct argv*)argv)->fd, ((struct argv*)argv)->send_msg,
		strlen(((struct argv*)argv)->send_msg), 0,
		(struct sockaddr*)(((struct argv*)argv)->serv), sizeof(struct sockaddr_in))) {
		perror("Send error");

		exit(EXIT_FAILURE);
	}

	memset((((struct argv*)argv)->serv), '\0', sizeof(struct sockaddr_in));

	if (-1 == recvfrom(((struct argv*)argv)->fd, ((struct argv*)argv)->recv_msg,
		64, 0,
		(struct sockaddr*)(((struct argv*)argv)->serv), ((struct argv*)argv)->len)) {
		perror("receive ERR");
	}

	/*((struct argv*)argv)->serv_port = ((struct argv*)argv)->recv_msg;

	((struct argv*)argv)->serv->sin_port = htons(*(((struct argv*)argv)->serv_port));*/

	//printf("massege received: %d\n", *(((struct argv*)argv)->serv_port));

	/*memset(((struct argv*)argv)->recv_msg, '\0',
		sizeof(((struct argv*)argv)->recv_msg));

	sendto(((struct argv*)argv)->fd, ((struct argv*)argv)->send_msg,
		strlen(((struct argv*)argv)->send_msg), 0,
		(struct sockaddr*)(((struct argv*)argv)->serv), sizeof(struct sockaddr_in)); //отправили сообщение для подтверждения соединения на новый порт

	recvfrom(((struct argv*)argv)->fd, ((struct argv*)argv)->recv_msg,
		strlen(((struct argv*)argv)->send_msg), 0,
		(struct sockaddr*)(((struct argv*)argv)->serv), sizeof(struct sockaddr_in)); // получение строки времени*/

	printf("Time is: %s\n", ((struct argv*)argv)->recv_msg);


	close(((struct argv*)argv)->fd);
}