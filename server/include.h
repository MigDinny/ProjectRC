#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define BUFLEN 512	// buffer length
#define TCP_PORT 9000	// TCP port to admin CLI
#define UDP_PORT 5000   // UDP port 
#define MAX_USERS 20    // max users 

typedef struct user {
    char username[20];
    char password[20];

} user;

user userlist[MAX_USERS];

void erro(char *);

void TCPWorker();

void sigint(int);