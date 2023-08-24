#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <malloc.h>


#define addr "127.0.0.1"
#define port "7777"

key_t semaphore[4] = {1984, 451, 20000, 127};
pid_t child_proc[4];
short ports[4] = { 6694, 6695, 6696, 6697};

char* shtos(short* number);
char* itoa(int number);


int main(int args, char** argv) {

	int request_fd;
	int index = 0;

	char* received_msg = (char*)malloc(256);
	char* sended_msg = (char*)malloc(128);
	memset(sended_msg, 0, 128);

	struct sockaddr_in server, client;

	socklen_t length = sizeof(struct sockaddr_in);

	request_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (request_fd == -1) {
		perror("Socket create error");
		exit(EXIT_FAILURE);
	}

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(addr); //addr - 127.0.0.1
	server.sin_port = htons(atoi(port)); 

	if (bind(request_fd, (struct sockaddr*)&server, sizeof(struct sockaddr_in))) {
		perror("Socket bind error");
		close(request_fd);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < 4; i++) {

		semget(semaphore[0], 1, 0666 | IPC_CREAT);

		child_proc[i] = fork();
		if (-1 == child_proc[i]) {
			perror("child proc create failure");
			exit(EXIT_FAILURE);
		}

		if (child_proc[i] == 0) {

			char* sended_info;
			char* proc_received;

			socklen_t lengthforchild = sizeof(struct sockaddr_in);

			struct sockaddr_in proc_struct, clienttosend;
			time_t time_struct;

			int proc_fd = socket(AF_INET, SOCK_DGRAM, 0);

			if (proc_fd == -1) {
				perror("Socket create error");
				exit(EXIT_FAILURE);
			}

			proc_struct.sin_family = AF_INET;
			proc_struct.sin_addr.s_addr = inet_addr(addr);
			proc_struct.sin_port = htons(ports[i]);
			printf("%d\n", proc_struct.sin_port);

			sended_info = (char*)malloc(sizeof(proc_struct));
			proc_received = (char*)malloc(256);

			if (bind(proc_fd, (struct sockaddr*)&proc_struct, sizeof(struct sockaddr_in))) {
				perror("Socket bind error");
				close(request_fd);
				exit(EXIT_FAILURE);
			}

			while (1) {

				memset(&clienttosend, '\0', sizeof(struct sockaddr_in));

				recvfrom(proc_fd, proc_received, 256, 0, (struct sockaddr*)&clienttosend, &lengthforchild); //убедимся что произошёл конект(клиент отпрваит что-то)
				semctl(semaphore[i], 0, SETVAL, 0);

				memset(sended_info, '\0', strlen(sended_info));
				strcpy(sended_info, ctime(&time_struct));

				if (-1 == sendto(proc_fd, sended_info, strlen(sended_info), 0, (struct sockaddr*)&clienttosend, lengthforchild)) {
					perror("Socket send error in");
					close(request_fd);
					exit(EXIT_FAILURE);
				}
				memset(&clienttosend, '\0', sizeof(clienttosend));
				semctl(semaphore[i], 0, SETVAL, 1);
			}

			exit(EXIT_SUCCESS);
		}
	}

	while (1) {

		int flag = 1;
		char* port_to_send = malloc(8);

		memset(&client, '\0', sizeof(client));

		if (-1 == recvfrom(request_fd, received_msg, strlen(received_msg), 0, (struct sockaddr*)&client, &length)) {
			perror("Socket receive error");
			close(request_fd);
			exit(EXIT_FAILURE);
		}

		while (flag) {

			for (int i = 0; i < 4; i++) {

				if (!semctl(semaphore[i], 0, GETVAL, 0)) {

					continue;
				}
				else {
					strcpy(port_to_send, shtos(&ports[i]));

					if (-1 == sendto(request_fd, port_to_send, strlen(port_to_send), 0, (struct sockaddr*)&client, length)) {
						perror("Socket send error");
						close(request_fd);
						exit(EXIT_FAILURE);
					}
					flag = 0;
					break;

				}
			}
		}
	}
	
	exit(EXIT_SUCCESS);
}


char* shtos(short *number) {

	char* string = (char*)malloc(2);
	char* ptr = number;
	for (int i = 0; i < 2; i++) {
		string[i] = *ptr;
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