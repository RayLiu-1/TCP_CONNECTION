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
	int a = 1;
	int b = 2;
	const int* c = &b;

	printf("%d\n", *c);
	const int* c = &a;
	printf("%d\n", *c);
	return 0;
}
