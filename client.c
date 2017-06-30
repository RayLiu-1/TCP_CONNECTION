#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/time.h>   
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/types.h> 
//#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h> 
#define CLIENTPORT 8010//client port
#define BUFFERSIZE 255//the maxsize of single buffer

int valid_command(const char* command) {
	char new_command[BUFFERSIZE];
	memset(new_command, 0, BUFFERSIZE);
	strncpy(new_command, command, BUFFERSIZE);
	char *tok = strtok(new_command, " ");
	const char* ping = "pingSites";
	const char* show = "showHandles";
	const char* showStatus = "showHandleStatus";
	const char* exit = "exit";
	const char* help = "help";
	if (strncmp(tok, ping, sizeof(ping)) == 0 || strncmp(tok, show, sizeof(show)) == 0 || 
		strncmp(tok, showStatus, sizeof(showStatus)) == 0) {
		return 1;
	}
	else if (strncmp(tok, ping, sizeof(ping)) == 0 || strncmp(tok, show, sizeof(show)) == 0 ||
		strncmp(tok, showStatus, sizeof(showStatus)) == 0) {
		return 1;
	}
	else if (strncmp(tok, exit, sizeof(exit)) == 0) {
		return 2;
	}
	else if (strncmp(tok, help, sizeof(help)) == 0) {
		return 3;
	}
	return 0;
}


int main(int argc, char *argv[]) {
	if (argc < 3) {
		perror("Please use ./client <Server's IP address> <port> to run the client program");
		exit(1);
	}
	//Set the server socket:
	int serverfd, portno;
	struct sockaddr_in server_addr;
	portno = atoi(argv[2]);
	if ((serverfd = socket(AF_INET, SOCK_STREAM, 0))<0) {
		perror("Creating socket failed");
		exit(1);
	}
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	//Connect to the server
	if (connect(serverfd, (struct sockaddr *)&server_addr,sizeof(server_addr))<0) {
		perror("Connecting server failed");
		exit(1);
	}
	puts("Get connection with the server.");
	while (1) {
		puts("Please enter the commands:");
		char command[BUFFERSIZE];
		memset(command, 0, sizeof(command));
		fgets(command, BUFFERSIZE, stdin);
		int commamdType = valid_command(command);
		if (commamdType == 1) {
			if (write(serverfd, command, strlen(command)) < 0) {
				perror("Writing to socket failed");
				exit(1);
			}
			memset(command, 0, BUFFERSIZE);
			if (read(serverfd, command, BUFFERSIZE) < 0) {
				perror("Reading from socket failed");
				exit(1);
			}

			printf("%s\n", command);
		}
		else if (commamdType == 2) {
			//todo
		}
	}
	
	
	return 0;
}
