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
//#define TCP_PORT 9000	// TCP port to admin CLI
//#define UDP_PORT 5000   // UDP port
int TCP_PORT = 9000;
int UDP_PORT = 5000;
#define MAX_USERS 20    // max users
#define CLIENT_PORT 4000
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


/* GLOBALS */

pthread_t TCP_thread_id;
user user_list[MAX_USERS];

struct sockaddr_in udp_int_socket, udp_ext_socket;
int udp_fd, udp_recv_len;
socklen_t udp_ext_len = sizeof(udp_ext_socket);
char udp_buf[BUFLEN];
char udp_answer[BUFLEN];
char udp_pairs[5][300];


/* FUNCTIONS */
void init();
void sigint(int);
void error(char *);

// UDP
void loopUDP();
int switcher();
int auth();
int listModes();
int reqP2P();
int reqMulticast();
int sendMSG();

// TCP

void *TCPWorker(void*);
void listUsers();
int valueVerification(char arguments[MAX_ARGUMENTS][ARGUMENT_LEN]);
void addUser(char arguments[MAX_ARGUMENTS][ARGUMENT_LEN]);
void deleteUser(char arguments[MAX_ARGUMENTS][ARGUMENT_LEN]);
void sigint(int);
void init_tcp();
void writeToFile();

pthread_t TCPThreadID;
int tcp_fd, tcp_client_fd;
struct sockaddr_in tcp_addr, tcp_client_addr;
int tcp_client_addr_size;
FILE* fp;
char *file_name = "ficheiro.txt";

int address_counter = 0;
char addresses[4][20] = {"226.0.0.2","226.0.0.3","226.0.0.4","226.0.0.5"};
