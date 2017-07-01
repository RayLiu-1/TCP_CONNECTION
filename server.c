#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/time.h>   
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/types.h> 
#define QUEUESIZE 5//the max number of requests in queue
#define BUFFERSIZE 255//the maxsize of single buffer
#define WORKSNO 5//the number of works
#define PINGPORT 80//the tcp port to ping
#define CONNECTIONNO 10//the number of connecting trials
#define MAXHANDLES 50//the max number of handles
#define MAXSITES 20//the max number of site in single input

struct Node * Handles[MAXHANDLES];
struct Queue requests;
int max_handle;//current max handle
pthread_mutex_t lock;//thread mutex lock
pthread_t ping_thread[WORKSNO];

enum Status {
	IN_QUEUE,
	IN_PROGRESS,
	COMPELTE,
	NOT_FOUND,
	TIME_OUT,
};

static const char *STATUS_STRING[] = {
	"IN_QUEUE", "IN_PROGRESS", "COMPELTE", "NOT_FOUND","TIME_OUT",
};

struct Node {
	struct Node* next;
	struct Node* nextInHandle;
	enum Status curStatus;
	int handle;
	int min;
	int max;
	int avery;
	struct sockaddr_in client_addr;
	char site[256];
};

struct Queue {
	struct Node* head;
	struct Node* tail;
	int size;
};

void HandlesAdd(struct Node* element) {
	struct Node *p = Handles[element->handle];
	Handles[element->handle] = element;
	element->nextInHandle = p;

	//printf("%s\n", Handles[element->handle]->site);
}

void QueueAdd(struct Queue *que, struct Node* element) {
	pthread_mutex_lock(&lock);
	if (que->size == 0) {
		que->head = element;
		que->head->next = NULL;
		que->tail = element;
	}
	else {
		que->tail->next = element;
		que->tail = que->tail->next;
	}
	que->size++;
	element->curStatus = IN_QUEUE;
	HandlesAdd(element);
	pthread_mutex_unlock(&lock);
}

struct Node * Pop(struct Queue *que) {
	pthread_mutex_lock(&lock);
	if (que->size == 0) {
		pthread_mutex_unlock(&lock);
		return NULL;
	}
	else {
		struct Node *p = que->head;
		que->head = que->head->next;
		if (que->size == 1)
			que->tail = NULL;
		que->size--;
		p->curStatus = IN_PROGRESS;
		pthread_mutex_unlock(&lock);
		return p;
	}
	return NULL;
}

void *recerving_handler(void *pfd) {
	int client_fd = *(int*)pfd;
	int read_size;
	char request[BUFFERSIZE];
	memset(request, 0, BUFFERSIZE);
	while ((read_size = recv(client_fd, request, BUFFERSIZE, 0)) > 0) {
		if (strncmp("pingSites ", request, 10) == 0) {
			char* host;
			host = strtok(request, " \n");
			char handle_msg[BUFFERSIZE];
			memset(handle_msg, 0, BUFFERSIZE);
			host = strtok(NULL, " ,\n");
			if (strlen(host)>0) {	
				if (max_handle + 1 >= MAXHANDLES) {
					max_handle = 0;
				}
				if (Handles[max_handle + 1] != NULL) {
					struct Node* p = Handles[max_handle + 1];
					while (p != NULL) {
						struct Node* newP = p->nextInHandle;
						while (p->curStatus == IN_PROGRESS || p->curStatus == IN_QUEUE);
						free(p);
						p = newP;
					}
				}
				sprintf(handle_msg, "Your handle is %d\n", max_handle+1);
				int handle = ++max_handle;
				struct Node *newNode;
				int sites = 0;
				while (host != NULL&& (sites++ <MAXSITES)) {
					//printf("%d\n", sizeof(struct Node));
					newNode =(struct Node *) malloc(sizeof(struct Node));
					newNode->handle = handle;
					newNode->next = NULL;
					newNode->nextInHandle = NULL;
					newNode->max = 0;
					newNode->min = 0;
					newNode->avery = 0;
					memset(newNode->site, 0, BUFFERSIZE);
					strncpy(newNode->site, host, strlen(host));
					//printf("%s 1\n", host);
					QueueAdd(&requests, newNode);
					//printf("%s 2\n", host);
					host = strtok(NULL, " ,\n ");
				}
			}
			else {
				const char * no_site = "Please use \'pingSites <host>\'.";
				strncpy(handle_msg, no_site,sizeof(no_site));
			}
			if (write(client_fd, handle_msg, strlen(handle_msg)) < 0) {
				perror("Wrinting to socket failed");
				exit(1);
			}
				//fclose(fp);
		}
		else if (strncmp("showHandleStatus", request, 16)==0) {
			//puts("asd222");
			char *handleA;
			int handleI = max_handle+1;
			handleA = strtok(request, " \n");
			handleA = strtok(NULL, " ,\n");
			char handle_msg[BUFFERSIZE];
			if (handleA!=NULL) {
				handleI = atoi(handleA);
				struct Node * p = Handles[handleI];
				while (p != NULL) {
					memset(handle_msg, 0, BUFFERSIZE);
					sprintf(handle_msg, "%d %s %d %d %d %s\n", p->handle, p->site, p->avery, p->min, p->max, STATUS_STRING[p->curStatus]);
					if (write(client_fd, handle_msg, strlen(handle_msg)) < 0) {
						perror("Wrinting to socket failed");
						exit(1);
					}
					p = p->nextInHandle;
				}
			}
			else {
				int handle = 1;
				while (Handles[handle] != NULL) {
					struct Node * p = Handles[handle];
					while (p != NULL) {
						memset(handle_msg, 0, BUFFERSIZE);
						sprintf(handle_msg, "%d %s %d %d %d %s\n", p->handle, p->site, p->avery, p->min, p->max, STATUS_STRING[p->curStatus]);
						if (write(client_fd, handle_msg, strlen(handle_msg)) < 0) {
							perror("Wrinting to socket failed");
							exit(1);
						}
						p = p->nextInHandle;
					}
					handle++;
				}
			}
		
		}
		else if (strncmp("showHandles", request, 11) == 0) {
			int handle = 1;
			char handles[BUFFERSIZE];
			while (handle <= max_handle) {
				memset(handles, 0, BUFFERSIZE);
				sprintf(handles, "%d,", handle++);
				struct Node *p = Handles[handle];
				while (p != NULL) {
					strncat(handles, p->site, strlen(p->site));
					strncat(handles, ",", 1);
				}
				printf(handles);
				handles[strlen(handles) - 1] = '\n';
				if ((write(client_fd, handles, strlen(handles))) < 0) {
					perror("Wrinting to socket failed");
				}
			}
		}
		else {
			const char* msg = "Invalid command.\n";
			//strncpy(msg, "Invalid command.\n",18);
			if ((write(client_fd, msg, strlen(msg))) < 0) {
				perror("Wrinting to socket failed");
			}
		}
		char * end_msg = "END";
		if (write(client_fd, end_msg, strlen(end_msg)) < 0) {
			perror("Wrinting to socket failed");
		}	
		memset(request, 0, BUFFERSIZE);
	}
}

void *ping_handler(void *pworker) {
	int worker = *(int *)pworker;
	while (1) {
		struct Node* p = Pop(&requests);
		if (p!=NULL) {
			//puts("sd");
			int handle = p->handle;
			/*char logfile[BUFFERSIZE];
			memset(logfile, 0, BUFFERSIZE);
			sprintf(logfile, "%lu.log", handle);
			FILE* fp;
			fp = fopen(logfile, "rw");
			if (fp == NULL) {
				perror("Opening file failed");
				exit(1);
			}*/
			/*char record[BUFFERSIZE];//Each line in the file is a record for certain host
			while (fgets(record, BUFFERSIZE, fp) != NULL) {
				char *host;
				if (strncmp(host, p->site, strlen(host)) == 0) {
					const char* status = "  00   00   00   IN_PROGRESS\n";
					fprintf(fp,"%s", status);
					fseek(fp, 0 - sizeof(status), SEEK_CUR);
					break;
				}
			}*/
			
			int sockfd, n;
			struct sockaddr_in site_addr;
			struct hostent *site;
			//site = malloc(sizeof(struct hostent));
			site = gethostbyname(p->site);
			//puts(p->site);
			/*struct in_addr **addr_list;
			addr_list = (struct in_addr **)site->h_addr_list;
			printf("%s", inet_ntoa(*addr_list[0]));*/
			if (site == NULL|| site->h_addr_list[0]==NULL) {
				p->curStatus = NOT_FOUND;
			}
			else {
				memset(&site_addr, 0, sizeof(site_addr));
				site_addr.sin_family = AF_INET;
				memcpy(&site_addr.sin_addr, site->h_addr_list[0], site->h_length );
				site_addr.sin_port = htons(PINGPORT);
				int min_time = 9999;
				int max_time = 0;
				struct timeval begin, end, timeout;
				timeout.tv_sec = 9;
				timeout.tv_usec = 0;
				fd_set fds;
				int Num_Succ = 0;
				int Sum_Time = 0;
				int ready;
				for (int i = 0; i < CONNECTIONNO; i++) {
					sockfd = socket(AF_INET, SOCK_STREAM, 0);
					if ((fcntl(sockfd, F_SETFL, O_NONBLOCK)) < 0) {
						perror("Setting non-blocking failed.");
					}
					if (sockfd < 0) {
						perror("ERROR opening socket");
					}
					FD_ZERO(&fds);
					FD_SET(sockfd, &fds);
					gettimeofday(&begin, NULL);
					connect(sockfd, (struct sockaddr*)&site_addr, sizeof(site_addr));
					ready = select(sockfd + 1, &fds, &fds, NULL, &timeout);
					gettimeofday(&end, NULL);
					//printf("%d\n", ready);
					if (ready < 0) {
						perror("Selecting file descriptors failed");
					}
					
					else if(ready >0){
						Num_Succ++;
						int time = (end.tv_sec - begin.tv_sec) * 1000 + (end.tv_usec - begin.tv_usec) / 1000;
						//printf("%d\n", time);
						if (Num_Succ == 0) {
							min_time = time;
							max_time = time;
						}
						else {
							min_time = min_time < time ? min_time : time;
							max_time = max_time > time ? max_time : time;
						}
						Sum_Time += time;
					}
					close(sockfd);
					sleep(1);
				}
				if (Num_Succ == 0) {
					//fprintf(fp, "Connection time out         \n");
					p->curStatus = TIME_OUT;
				}
				else {
					p->avery = Sum_Time / Num_Succ;
					p->min = min_time;
					p->max = max_time;
					p->curStatus = COMPELTE;
					//fprintf(fp, "%4d %4d %4d   COMPLETE   \n",Sum_Time/Num_Succ,min_time,max_time);
				}
			}
			//fclose(fp);
		}
	}
}


int main(int argc, char *argv[]) {
	if (argc < 2) {
		perror("Please use ./server <port> to run the server program");
		exit(1);
	}
	const int port = atoi(argv[1]);
	printf("%d\n",port);
	char buffer[256];
	int server_fd,client_fd;//lsfd:file descriper of of the socket for listening
						 //cnfd:file descriper of the socket for connection
						//create the server socket
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		perror("Creating socket failed");
		return 1;
	}
	//initial request queue
	requests.head = NULL;
	requests.tail = NULL;
	requests.size = 0;
	//set the server address 
	struct sockaddr_in server_addr, client_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	
	//Inialize the Handles array
	for (int i = 0; i < MAXHANDLES; i++) {
		Handles[i] = NULL;
	}
	//char str[BUFFERSIZE];
	//inet_ntop(AF_INET, &(server_addr.sin_addr), str, INET_ADDRSTRLEN);

	//printf("%s\n", str);
	//Bind server socket with server address
	if ((bind(server_fd, (struct sockaddr*) &server_addr, sizeof(server_addr) )) < 0) {
		perror("Binding socket failed");
		exit(1);

	}
	//Listen on the server socket for connections
	listen(server_fd, QUEUESIZE);
	printf("The server is ready to be connected.\n");
	//initial the mutex lock
	if ((pthread_mutex_init(&lock, NULL)) != 0)
	{
		perror("Initailizing mutex lock failed\n");
		exit(1);

	}
	//Create the ping thread

	for (int i = 0; i < WORKSNO; i++) {
		int *NO = malloc(sizeof(int));
		*NO = i;
		if ((pthread_create(&ping_thread[i], NULL, ping_handler, NO)) < 0) {
			perror("Creating thread failed");
			exit(1);

		}
	}

	//Receive requests from clients 
	int client_size = sizeof(struct sockaddr_in);
	int *pfd;
	
	while ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_size))) {
		puts("Connection accepted.");
		pfd = &client_fd;
		pthread_t receive_thread;
		pfd = malloc(sizeof(pfd));
		*pfd = client_fd;

		if ((pthread_create(&receive_thread, NULL, recerving_handler, (void*)pfd)) < 0) {
			perror("Creating thread failed");
		}
		//pthread_join(receive_thread, NULL);
	}
	if (client_fd < 0) {
		perror("Accecpting socket failed");
		exit(1);
	}
	return 0;
}