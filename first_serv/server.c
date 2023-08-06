#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
//#include <sys/time.h>

#define addr "127.0.0.1"
#define port "9990"

char* shtos(struct sockaddr_in server, int size) {

	char* string = (char*)malloc(2);
	char* ptr = &(server.sin_port);
	for (int i = 0; i < size; i++) {
		string[size - i - 1] = *ptr;
		ptr++;
	}

	return string;
}

char* itoa(int number) {

	char* string = (char*)malloc(4);
	char* ptr = &number;
	for (int i = 0; i < 4; i++) {
		string[4 - i - 1] = *ptr;
		ptr++;
	}

	return string;
}

int main(int args, char** argv) {
	
	int request_fd;
	int index = 0;

	char* received_msg = (char*)malloc(256);

	struct sockaddr_in server, client;

	time_t time_struct;
	socklen_t length = sizeof(client);

	request_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (request_fd == -1) {
		perror("Socket create error");
		exit(EXIT_FAILURE);
	}

	memset(&server, '\0', sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(addr);
	server.sin_port = htons(atoi(port));

	if (bind(request_fd, (struct sockaddr*)&server, sizeof(struct sockaddr_in))) {
		perror("Socket bind error");
		close(request_fd);
		exit(EXIT_FAILURE);
	}

	while (1) {


		printf("Waiting for massege...\n");

		memset(&client, '\0', sizeof(client));

		if (-1 == recvfrom(request_fd, received_msg, strlen(received_msg), 0, (struct sockaddr*)&client, &length)) {
			perror("Socket receive error");
			close(request_fd);
			exit(EXIT_FAILURE);
		}

		printf("Massege received\n");

		pid_t child_proc = fork();

		index++;

		if (child_proc == 0) {

			int proc_port = atoi(port) + index;
			int tm_time;
			char* sended_info;
			char* proc_received;

			struct sockaddr_in proc_struct;
			int proc_fd = socket(AF_INET, SOCK_DGRAM, 0);
			if (proc_fd == -1) {
				perror("Socket create error");
				exit(EXIT_FAILURE);
			}

			proc_struct.sin_family = AF_INET;
			proc_struct.sin_addr.s_addr = inet_addr(addr);
			proc_struct.sin_port = htons(proc_port);

			sended_info = (char*)malloc(sizeof(proc_struct));
			proc_received = (char*)malloc(256);
			strcpy(sended_info, shtos(proc_struct, 2));

			if (bind(proc_fd, (struct sockaddr*)&proc_struct, sizeof(struct sockaddr_in))) {
				perror("Socket bind error");
				close(request_fd);
				exit(EXIT_FAILURE);
			}

			if (-1 == sendto(request_fd, sended_info, strlen(sended_info), 0, (struct sockaddr*)&client, sizeof(struct sockaddr_in))) {
				perror("Socket send error");
				close(request_fd);
				exit(EXIT_FAILURE);
			}

			printf("Child proc wait for massege...\n");
			recvfrom(proc_fd, proc_received, 256, 0, (struct sockaddr*)&client, sizeof(struct sockaddr_in)); //убедимся что произошёл конект(клиент отпрваит что-то)
			printf("Massege received\n");

			memset(sended_info, 0, strlen(sended_info));
			strcpy(sended_info, ctime(&time_struct));

			if(-1 == sendto(proc_fd, sended_info, 256, 0, (struct sockaddr*)&client, sizeof(struct sockaddr_in))){
				perror("Socket send error");
				close(request_fd);
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);

		}
	}


}

