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
#define port 7782

int main(int args, char** argv) {
	
	int request_fd;

	char* received_msg = (char*)malloc(256);

	struct sockaddr_in server, client;
	struct tm* time;

	time_t time_struct;
	socklen_t length;

	request_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (request_fd == -1) {
		perror("Socket create error");
		exit(EXIT_FAILURE);
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(addr);
	server.sin_port = htons(port);

	if (bind(request_fd, (struct sockaddr*)&server, sizeof(struct sockaddr_in))) {
		perror("Socket bind error");
		close(request_fd);
		exit(EXIT_FAILURE);
	}

	while (1) {

		printf("Waiting fot massege...\n");

		recvfrom(request_fd, received_msg, strlen(received_msg), 0, (struct sockaddr*)&client, sizeof(struct sockaddr_in));
		printf("Massege received\n");

		pid_t child_proc = fork();

		if (child_proc == 0) {

			int proc_port = htons(port + 1);
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
			proc_struct.sin_port = proc_port;

			sended_info = (char*)malloc(sizeof(proc_struct));
			proc_received = (char*)malloc(256);
			strcpy(sended_info, (char*)&proc_struct.sin_port);

			if (bind(proc_fd, (struct sockaddr*)&proc_struct, sizeof(struct sockaddr_in))) {
				perror("Socket bind error");
				close(request_fd);
				exit(EXIT_FAILURE);
			}

			sendto(request_fd, sended_info, strlen(sended_info), 0, (struct sockaddr*)&client, sizeof(struct sockaddr_in));

			recvfrom(proc_fd, proc_received, 256, 0, (struct sockaddr*)&client, sizeof(struct sockaddr_in)); //убедимся что произошёл конект(клиент отпрваит что-то)

			memset(sended_info, 0, strlen(sended_info));
			time = localtime(&time_struct);
			tm_time = time->tm_hour;
			strcpy(sended_info, (char*)tm_time);
			tm_time = time->tm_min;
			strcat(sended_info, (char*)tm_time);
			tm_time = time->tm_sec;
			strcat(sended_info, (char*)tm_time);

			sendto(proc_fd, sended_info, 256, 0, (struct sockaddr*)&client, sizeof(struct sockaddr_in));

		}
	}


}

