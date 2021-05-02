#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

/* CONSTANTS AND STRUCTS */

#define BUFLEN 512	// buffer length
#define TCP_PORT 9000	// TCP port to admin CLI
#define UDP_PORT 5000   // UDP port 
#define MAX_USERS 20    // max users 

typedef struct user {
    char username[20];
    char password[20];

} user;


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
int sendMSG();

// TCP
void *TCPWorker();



/* GLOBALS */

pthread_t TCP_thread_id;
user user_list[MAX_USERS];

struct sockaddr_in udp_int_socket, udp_ext_socket;
int udp_fd, udp_recv_len;
socklen_t udp_ext_len = sizeof(udp_ext_socket);
char udp_buf[BUFLEN];
char udp_answer[BUFLEN];
char udp_pairs[5][300];