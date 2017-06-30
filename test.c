#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/time.h>   
//#include <arpa/inet.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/types.h> 
//#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h> 
#define CLIENTPORT 8010
#define BUFFERSIZE 256//the maxsize of single buffer

int main(int argc, char *argv[]) {
	char str1[20];
	char str2[20];
	char a[20]= "aaabbb bbb bbb ";
	char *p = strtok(a, " ");
	puts(p);
	puts(a);
	p = strtok(NULL, " ");
	
	puts(p);
	puts(a);
	return 0;
}
