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

    for (int i = 0; i < MAX_USERS; i++) {

      if (strcmp(user_list[i].username, "") == 0){

        sprintf(answer, "ACCESS DENIED\n");
        break;

      }

      //means username has a match
      if (strcmp(user_list[i].username, username) == 0) {

        //checks to see if the pasword matches

        if (strcmp(user_list[i].password, password) == 0) {
          char authorizations[300];

          if (strcmp(user_list[i].client_server,"yes") == 0)  strcat(authorizations, "|Client-Server");
          if (strcmp(user_list[i].p2p,"yes") == 0) strcat(authorizations, "|P2P");
          if(strcmp(user_list[i].group, "yes") == 0) strcat(authorizations, "|Group");

          sprintf(answer, "OPTIONS%s",authorizations);
          break;
        }

        //due to the username being unique if the password is denied, the access is denied
        else {

          sprintf(answer, "ACCESS DENIED\n");
          sendto(udp_fd, answer, BUFLEN, MSG_CONFIRM, (struct sockaddr *) &udp_ext_socket, sizeof(udp_ext_socket));
          return -1;
        }


      }
    }

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
    for (int i = 0 ; i < MAX_USERS; i++) {

      if (strcmp(user_list[i].username, "") == 0){

        sprintf(answer, "INVALID DESTINATION USER\n");
        sendto(udp_fd, answer, BUFLEN, MSG_CONFIRM, (struct sockaddr *) &udp_ext_socket, sizeof(udp_ext_socket));
        return -1;

      }

      //means username has a match
      if (strcmp(user_list[i].username, username) == 0) {

        sprintf(answer, "%s", user_list[i].ip);
        break;

      }

    }

    // send response with IP

    // or send invalid destuser
	  sendto(udp_fd, answer, BUFLEN, MSG_CONFIRM, (struct sockaddr *) &udp_ext_socket, sizeof(udp_ext_socket));

    return 0;
}

int reqMulticast() {

    // blank for now
        return 0;
}

int sendMSG() {

    char answer[BUFLEN];
    char endClient[100];
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
    for (int i = 0 ; i < MAX_USERS; i++) {

      if (strcmp(user_list[i].username, "") == 0){

        sprintf(answer, "INVALID DESTINATION USER\n");
        sendto(udp_fd, answer, BUFLEN, MSG_CONFIRM, (struct sockaddr *) &udp_ext_socket, sizeof(udp_ext_socket));
        return -1;

      }

      //means username has a match
      if (strcmp(user_list[i].username, username) == 0) {

        strcpy(endClient,  user_list[i].ip);

        break;

      }

    }

    // build socket to send message to destuser


    int dest_fd;
    struct sockaddr_in dest_addr, arrival_addr;
    socklen_t slen = sizeof(arrival_addr);
    struct hostent *hostPtr;

	  if ((hostPtr = gethostbyname(endClient)) == 0)
		  error("Unreachable address");

	  if ((dest_fd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
		  error("Problem creating the socket");

	  dest_addr.sin_family = AF_INET;
    // TO -DO METER PORTO !!!!!!!!!!!!!!! (nao sabia como! )
	  dest_addr.sin_port   = htons((short) atoi(CLIENT_PORT));
	  dest_addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;


    // send message like this "<username>: message"
    sprintf(answer, "%s",message);
    sendto(dest_fd, answer, BUFLEN, MSG_CONFIRM, (struct sockaddr *) &dest_addr, sizeof(dest_addr));

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
