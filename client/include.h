#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFLEN 512	// buffer length

char username[30];
char password[30];


int client_udp_fd;
struct sockaddr_in dest_addr, arrival_addr;
socklen_t slen = sizeof(arrival_addr);
struct hostent *hostPtr;

void init(char *, char *);
void error(char *);
