#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>


#define BUFLEN 512	// buffer length
#define TCP_PORT 9000	// TCP port to admin CLI
#define UDP_PORT 5000   // UDP port
#define MAX_USERS 20    // max users
#define MAX_ARGUMENTS 7  //max number of arguments given to the admin CLI
#define ARGUMENT_LEN 20  //length of an argument given to the admin CLI
#define REPLY_SIZE 256

typedef struct user {
    char username[20];
    char ip[20];
    char password[20];
    char client_server[4];
    char p2p[4];
    char group[4];

} user;

user userlist[MAX_USERS];

void erro(char *);

void *TCPWorker(void*);

void listUsers();

int valueVerification(char arguments[MAX_ARGUMENTS][ARGUMENT_LEN]);

void addUser(char arguments[MAX_ARGUMENTS][ARGUMENT_LEN]);

void deleteUser(char arguments[MAX_ARGUMENTS][ARGUMENT_LEN]);

void sigint(int);

void init();

void writeToFile();

pthread_t TCPThreadID;

int tcp_fd, tcp_client_fd;
struct sockaddr_in tcp_addr, tcp_client_addr;
int tcp_client_addr_size;

FILE* fp;
char *file_name = "ficheiro.txt";
