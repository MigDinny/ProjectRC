#include "include.h"

int main(int argc, char *argv[]) {

    // init variables
    init();

    // start thread to accept TCP conn >> admin CLI
    //pthread_create(&TCP_thread_id, NULL, TCPWorker, NULL);

    // handle & process UDP packets
    loopUDP();

    return 0;
}

void init() {
    int status = 0;

    // init UDP Socket

    udp_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_fd == -1) error("Error creating UDP socket");

    udp_int_socket.sin_family = AF_INET;
    udp_int_socket.sin_port = htons(UDP_PORT);
    udp_int_socket.sin_addr.s_addr = htonl(INADDR_ANY);

    status = bind(udp_fd, (struct sockaddr*) &udp_int_socket, sizeof(udp_int_socket));
    if (status == -1) error("Error binding UDP socket");

    // init TCP socket

    // init userlist

}

void loopUDP() {

    int status = 0;

    while (1) {

        udp_recv_len = recvfrom(udp_fd, udp_buf, BUFLEN, 0, (struct sockaddr *) &udp_ext_socket, (socklen_t *)&udp_ext_len);
        if (udp_recv_len == -1) continue;

        udp_buf[udp_recv_len] = '\0';

        status = switcher(); // detects which kind of request is and answers accordingly
        printf("status = %d\n", status);
    }

}

int switcher() {

    char *token;

    for (int u = 0; u < 5; u++)
        udp_pairs[u][0] = '\0';

    int i = 0;

    token = strtok(udp_buf, "&");
    while (token != NULL && i < 5) {
        if (token[0] == 'd') break;

        strcpy(udp_pairs[i++], token);
        token = strtok(NULL, "&");
    }

    token = strtok(udp_pairs[0], "=");
    if (strcmp(token, "mode") != 0) return -1;

    token = strtok(NULL, "=");
    if (token == NULL) return -1;
    
    switch (token[0]) {
        case '1':
            return auth();
        case '2':
            return reqP2P();
        case '3':
            return reqMulticast();
        case '4':
            return sendMSG();
        default:
            return -1;
    }

    return -1;
}

int auth() {

    char answer[BUFLEN];
    char *userpair = udp_pairs[1];
    char *passpair = udp_pairs[2];

    char *username;
    char *password;

    strtok(userpair, "=");
    username = strtok(NULL, "=");

    strtok(passpair, "=");
    password = strtok(NULL, "=");

    // auth

    // @TODO EDGAR

    // build response
    sprintf(answer, "1 - Normal chat\n2 - P2P chat\n3 - Group chat\n");

    // if auth invalid
    sprintf(answer, "ACCESS DENIED\n");

    // send response
	sendto(udp_fd, answer, BUFLEN, MSG_CONFIRM, (struct sockaddr *) &udp_ext_socket, sizeof(udp_ext_socket));

    return 0;
}

int reqP2P() {

    char answer[BUFLEN];
    char *userpair = udp_pairs[1];
    char *destuserpair = udp_pairs[2];
    char *username;
    char *destuser;

    strtok(userpair, "=");
    username = strtok(NULL, "=");

    strtok(destuserpair, "=");
    destuser = strtok(NULL, "=");
    
    // @TODO EDGAR
    // fetch IP of destuser


    // send response with IP 

    // or send invalid destuser


    return 0;
}

int reqMulticast() {

    // blank for now
        return 0;
}

int sendMSG() {

    char answer[BUFLEN];
    char *userpair = udp_pairs[1];
    char *destuserpair = udp_pairs[2];
    char *messagepair = udp_pairs[3];
    char *username;
    char *destuser;
    char *message;
    
    strtok(userpair, "=");
    username = strtok(NULL, "=");

    strtok(destuserpair, "=");
    destuser = strtok(NULL, "=");

    strtok(messagepair, "=");
    message = strtok(NULL, "=");


    // @TODO EDGAR
    // fetch IP from destuser

    // build socket to send message to destuser

    // send message like this "<username>: message"

    return 0;
}

void *TCPWorker() {

    // accept tcp conn

    // process it

    // loop
    return 0;
}

void error(char *s) {
	perror(s);
	exit(1);
}

void sigint(int signum) {
    // pthread_cancel
   //pthread_cancel(TCP_thread_id);

    // cleanup
    close(udp_fd);

    // exit
    exit(0);
}