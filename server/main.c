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

        printf("here2");
        fflush(stdout);

        udp_recv_len = recvfrom(udp_fd, udp_buf, BUFLEN, 0, (struct sockaddr *) &udp_ext_socket, (socklen_t *)&udp_ext_len);
        printf("%d", udp_recv_len);
        fflush(stdout);
        if (udp_recv_len == -1) continue;

        udp_buf[udp_recv_len] = '\0';

        status = switcher(); // detects which kind of request is and answers accordingly
        printf("status = %d\n", status);
        fflush(stdout);
    }

}

int switcher() {

    char *token;

    char pairs[5][300];
    for (int u = 0; u < 5; u++)
        pairs[u][0] = '\0';

    int i = 0;

    token = strtok(udp_buf, "&");
    while (token != NULL && i < 5) {
        if (token[0] == 'd') break;

        strcpy(pairs[i++], token);
        token = strtok(NULL, "&");
    }

    token = strtok(pairs[0], "=");
    if (strcmp(token, "mode") != 0) return -1;

    token = strtok(NULL, "=");
    if (token == NULL) return -2;
    
    switch (token[0]) {
        case '1':
            return -6;
        case '2':
            return -7;
        case '3':
            return -3;
        case '4':
            return -4;
        case '5':
            return -5;
        default:
            return -2;
    }

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