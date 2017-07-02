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
#define BUFFERSIZE 255//the maxsize of single buffer
#define MAXSITES 20//the max number of site in single input


int type_command(const char* command) {
	char new_command[BUFFERSIZE];
	memset(new_command, 0, BUFFERSIZE);
	strncpy(new_command, command, BUFFERSIZE);
	char *tok = strtok(new_command, " ");
	const char* ping = "pingSites";
	const char* show = "showHandles";
	const char* showStatus = "showHandleStatus";
	const char* exit = "exit";
	const char* help = "help";
	if ((strncmp(tok, ping, strlen(ping)) == 0 && strlen(tok) == strlen(ping)) || (strncmp(tok, show, strlen(show)) == 0 && strlen(tok) == strlen(show)) ||
		(strncmp(tok, showStatus, strlen(showStatus)) == 0 && strlen(tok) == strlen(showStatus))) {
		return 1;
	}
	else if (strncmp(tok, exit, strlen(exit)) == 0 && strlen(tok) == strlen(exit)) {
		return 2;
	}
	else if (strncmp(tok, help, strlen(help)) == 0 && strlen(tok) == strlen(help)) {
		return 3;
	}
	return 0;
}


int main(int argc, char *argv[]) {
	if (argc < 3) {
		perror("Please use ./client <Server's IP address> <Server's TCP port> to run the client program");
		exit(1);
	}
	//Set the server socket:
	int serverfd;
	struct sockaddr_in server_addr;
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
		int commamdType = type_command(command);
		if (commamdType == 1) {
			if (write(serverfd, command, strlen(command)) < 0) {
				perror("Writing to socket failed");
				exit(1);
			}
			memset(command, 0, BUFFERSIZE);
			while ((recv(serverfd, command, 2000, 0)>0) && (strncmp(command,
				"END", 3) != 0)) {
				printf("%s", command);
				memset(command, 0, BUFFERSIZE);
			}
		}
		else if (commamdType == 2) {
			puts("Disconnect with Server.");
			close(serverfd);
			return 0;
		}
		else if (commamdType == 3) {
			puts("Commands:");
			puts("1. pingSites <host1>,<host2>...");
			puts("  1.2. pingSites www.google.com,www.cnn.com");
			printf("  1.3. Input the list of web sites.This command shows an handle.The maximum number of sites in one command is %d.\n", MAXSITES);
			puts("2. showHandles");
			puts("  2.1. This command shows the handles of different requests made by all the client of the server");
			puts("3. showHandleStatus <handle>");
			puts("  3.1. This command shows the status of request in following format:");
			puts("       Handle SiteName Average Minimum Maximum Status");
			puts("4. showHandleStatus");
			puts("   Show the status of all handles");
			puts("5. exit");
			puts("   Exits the client");
			puts("6. help");
			puts("   Show all the commands");
		}
		else {
			char *p = strtok(command, " \n");
			printf("No command \'%s\' found, please use 'help' to lists all the commands and their syntax.\n", p);
		}
	}
	return 0;
}
