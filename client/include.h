#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#define BUFLEN 512	// buffer length
#define CLIENT_PORT 4000

char username[30];
char permissions[3][30];

int multicast = 0;
char multicast_interface[20];
char multicast_ip[20];
int client_udp_fd;
struct sockaddr_in dest_addr, arrival_addr;
socklen_t slen = sizeof(arrival_addr);
struct hostent *hostPtr;
pthread_t UDPThreadID;

void init(char *, char *);
void sigint(int);
void error(char *);
void menu();
void authentication();
int validChoice(char *);
void clientServerFunc();
void p2pFunc();
void multicastFunc();
void* UDPWorker();
