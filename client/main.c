#include "include.h"


int main(int argc, char *argv[]) {

	if (argc != 3) {
	  printf("cliente <host> <port> \n");
	  exit(0);
	}


	init(argv[1], argv[2]);


	authentication();
	signal(SIGINT, sigint);
	menu();

}

//
void authentication(){

  char username_input[30];
  char password_input[30];
  char request[BUFLEN];
  char answer[BUFLEN];
  answer[0] = '\0';

  printf("Username: ");
  scanf("%s",username_input);

  printf("Password: ");
  scanf("%s",password_input);

  sprintf(request, "mode=1&username=%s&password=%s", username_input, password_input);
  sendto(client_udp_fd, request, strlen(request), MSG_CONFIRM, (struct sockaddr *) &dest_addr, sizeof(dest_addr));

  //get answer
  int nread = recvfrom(client_udp_fd, answer , BUFLEN, 0, (struct sockaddr *) &arrival_addr, (socklen_t *)&slen);

  answer[nread] = '\0';

  if(strcmp(answer, "ACCESS DENIED") == 0){
	printf("%s\n",answer);
	exit(0);
  }

  strcpy(username , username_input);

  printf("AUTHENTICATION SUCCESSFULL!\n");
  printf("%s\n", answer);

  int counter = 0;
  char *token  = strtok(answer, "|");

  //The first strtok gives always the string OPTIONS which is useless
  token = strtok(NULL, "|");

  //while there are more spaces left we keep adding the arguments
  while( token != NULL){

	strcpy( permissions[counter++], token);
	token  = strtok(NULL, "|");
  }

}


//initialize variables
void init(char *server_name, char *port){


	if ((hostPtr = gethostbyname(server_name)) == 0)
		error("Invalid address");

	if ((client_udp_fd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
		error("socket");

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port   = htons((short) atoi(port));
	dest_addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;

}


void menu(){

  char choice[30];

  printf("Option: ");
  scanf("%s", choice);

  if ((strcmp(choice, "1") == 0)) {
	if (validChoice("1 - Client-Server") == 0) {
	  clientServerFunc();
	} else {
	  printf("You do not have authorization for this feature!\n");
	}
} else if ((strcmp(choice, "2") == 0)) {
	if (validChoice("2 - P2P") == 0) {
	  p2pFunc();
	} else {
	  printf("You do not have authorization for this feature!\n");
	}
} else if ((strcmp(choice, "3") == 0)) {
	if (validChoice("3 - Group") == 0) {
	  multicastFunc();
	} else {
	  printf("You do not have authorization for this feature!\n");
	}
  } else {
	printf("Not a valid option\n");
  }

}

//this fuction sees if the user has authorization to use a certain feature
int validChoice(char *choice){

  for (int i = 0; i < 3; i++) {

	if (strcmp(permissions[i], "") == 0) return -1;

	//if this is true, then it means that the user does have authorization to use the feature
	if(strcmp(permissions[i], choice) == 0) return 0;
  }

  return -1;
}


//connects to server  :)
void clientServerFunc(){

	char dest_user[30];
	char message[200];
	char request[BUFLEN];

	dest_user[0] = '\0';
	message[0] = '\0';
	request[0] = '\0';
	printf("Username to send information to: ");
	scanf("%s", dest_user);


	// start thread to receive incoming UDP messages
	pthread_create(&UDPThreadID, NULL, UDPWorker, NULL);

	printf("[Welcome to chat]");

	fgets(message, 200, stdin);

	message[0] = '\0';

	while(1){

		printf("\n>>");


		fgets(message, 200, stdin);

		//message[strlen(message)- 1] = '\0';

		sprintf(request, "mode=4&user=%s&destuser=%s&data=%s",username, dest_user, message);
		sendto(client_udp_fd, request, strlen(request), MSG_CONFIRM, (struct sockaddr *) &dest_addr, sizeof(dest_addr));

		}

}

void p2pFunc(){

	// ask user for the destuser
	char destuser[30];
	destuser[0] = '\0';

	printf("Destination user: ");
	scanf("%s", destuser);

	// ask server for P2P IP address
	char request[BUFLEN];
	char answer[BUFLEN];
	answer[0] = '\0';
	request[0] = '\0';
	sprintf(request, "mode=2&user=%s&destuser=%s", username, destuser);
	sendto(client_udp_fd, request, strlen(request), MSG_CONFIRM, (struct sockaddr *) &dest_addr, sizeof(dest_addr));

	//get answer
	recvfrom(client_udp_fd, answer, BUFLEN, 0, (struct sockaddr *) &arrival_addr, (socklen_t *)&slen);

	if(strcmp(answer, "INVALID DESTINATION USER") == 0){
		printf("%s\n",answer);
		exit(0);
	}


	struct sockaddr_in p2p_client_addr;
	struct hostent *hostPtrP2P;
	int p2p_fd;

	printf("ans = %s\n",answer);

	//if ((hostPtrP2P = gethostbyname(answer)) == 0)
		//error("Unreachable address");

	if ((p2p_fd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
		error("socket");

	p2p_client_addr.sin_family = AF_INET;
	p2p_client_addr.sin_port   = htons((short) CLIENT_PORT);
	p2p_client_addr.sin_addr.s_addr = inet_addr(answer);

	// start thread to receive incoming UDP messages
	pthread_create(&UDPThreadID, NULL, UDPWorker, NULL);

	char buf[BUFLEN];
	fgets(buf, BUFLEN, stdin);
	buf[0] = '\0';

    while (1) {

		printf(">> ");
		fgets(buf, BUFLEN, stdin);

    	sendto(p2p_fd, buf, strlen(buf), MSG_CONFIRM, (struct sockaddr *) &p2p_client_addr, sizeof(p2p_client_addr));

	}


}

void multicastFunc(){

    char request[BUFLEN];
    int group = 0;
    int choice = 0;
	char answer[BUFLEN];
	answer[0] = '\0';
	multicast_ip[0] = '\0';
	struct in_addr localInterface;

	while(1){

		printf("Options : 1 - Create a group | 2- Join a group");
	    scanf("%d",&choice);

		// Create new group
    	if(choice == 1){
			// groupmode=1, creates group server-side
        	sprintf(request, "mode=3&groupmode=1");
        	sendto(client_udp_fd, request, strlen(request), MSG_CONFIRM, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
			continue;
    	}

		// Join group
    	if(choice == 2){
			// groupmode=2, asks for ips
			sprintf(request, "mode=3&groupmode=2");
        	sendto(client_udp_fd, request, strlen(request), MSG_CONFIRM, (struct sockaddr *) &dest_addr, sizeof(dest_addr));

			// print ips from server
			int nread = recvfrom(client_udp_fd, answer, BUFLEN, 0, (struct sockaddr *) &arrival_addr, (socklen_t *)&slen);


			answer[nread] = '\0';



			// if there were no groups created yet
			if(strcmp(answer, "No groups created") == 0){
				printf("%s\n", answer);
				continue;
			}

			printf("Answer: %s\n",answer);
			// ask for multicast ip
			char validIps[4][20];

			//We now store all the ips avaiable sent by the server. We wait for the user to choose one and if it is correct we add the client to the new mode
			char *token;

		    for (int u = 0; u < 5; u++)
		        validIps[u][0] = '\0';

		    int i = 0;
		    token = strtok(answer, "-");
		    while (token != NULL) {
		        strcpy(validIps[i++], token);
		        token = strtok(NULL, "-");
		    }

			printf("---Groups---\n");

			for(int j = 0 ; j < 4; j++){
				if(strcmp(validIps[j], "") == 0){
					break;
				}
				printf("- %s\n",validIps[j]);
			}

			printf("Give a valid multicast ip: ");
			scanf("%s", multicast_ip);

			int correct = 0;

			//Check if the input is valid
			for(int j = 0 ; j < 4; j++){

				if(strcmp(validIps[j], "")==0){
					break;
				}

				if(strcmp(validIps[j], multicast_ip) == 0){
				correct = 1;
					break;
				}

			}
			//Means there was no match
			if(correct == 0){
				printf("Invalid group\n");
				continue;
			}

			break;
    	}

	}

	multicast_interface[0] = '\0';
	printf("Local interface address : " );
	scanf("%s", multicast_interface);

	// prepare to sent packets to multicast group
	if ((hostPtr = gethostbyname(multicast_ip)) == 0)
		error("Unreachable address");

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port   = htons((short) CLIENT_PORT);
	dest_addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;

	// disable loopback
	char loopch = 0;
	if(setsockopt(client_udp_fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch, sizeof(loopch)) < 0) {
		perror("Setting IP_MULTICAST_LOOP error");
		exit(1);
	}

	//increase ttl
	int ttl = 60;
	if(setsockopt(client_udp_fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
	  	perror("Setting local interface error");
  		exit(1);
	}

	// set multicast interface
	localInterface.s_addr = inet_addr(multicast_interface); // try INADDR_ANY if this doesnt work
	if(setsockopt(client_udp_fd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0) {
  		perror("Setting local interface error");
  		exit(1);
	}

	multicast = 1;
	// start thread to receive incoming UDP messages
	pthread_create(&UDPThreadID, NULL, UDPWorker, NULL);

	// loop to send messages
	char buf[BUFLEN];
	fgets(buf, BUFLEN, stdin);
	buf[0] = '\0';

    while (1) {

		printf(">> ");
		fgets(buf, BUFLEN, stdin);

    	sendto(client_udp_fd, buf, BUFLEN, MSG_CONFIRM, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
	}

}

void error(char *s) {
	perror(s);
	exit(1);
}

void* UDPWorker() {

	// init variables and create socket
	int udp_recv_len, udp_fd;
	struct sockaddr_in udp_ext_socket, udp_int_socket;
	socklen_t slen = sizeof(arrival_addr);
	char udp_buf[BUFLEN];
	socklen_t udp_ext_len = sizeof(udp_ext_socket);
	struct ip_mreq group;

	udp_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_fd == -1) error("Error creating UDP socket");

    udp_int_socket.sin_family = AF_INET;
    udp_int_socket.sin_port = htons(CLIENT_PORT);
    udp_int_socket.sin_addr.s_addr = htonl(INADDR_ANY);

	// set reuse opt
	if (multicast == 1) {
		int reuse = 1;

		if (setsockopt(client_udp_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0) {
			perror("Setting SO_REUSEADDR error");
			exit(1);
		}
	}

	// bind
    int status = bind(udp_fd, (struct sockaddr*) &udp_int_socket, sizeof(udp_int_socket));
    if (status == -1) error("Error binding UDP socket");

	// add membership to the group
	if (multicast == 1) {
		group.imr_multiaddr.s_addr = inet_addr(multicast_ip);

		group.imr_interface.s_addr = inet_addr(multicast_interface); // try INADDR_ANY if this doesnt work

		if (setsockopt(client_udp_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0) {
			perror("Adding multicast group error");
			exit(1);
		}
	}

	// loop to receive messages and print them out
    while (1) {

        udp_recv_len = recvfrom(udp_fd, udp_buf, BUFLEN, 0, (struct sockaddr *) &udp_ext_socket, (socklen_t *)&udp_ext_len);
        if (udp_recv_len == -1) continue;

        udp_buf[udp_recv_len] = '\0';

        printf(">%s\n>>", udp_buf); // @TODO raw data, beautify this in the end. use this to debug before beautifying
        fflush(stdout);
    }

}

void sigint(int signum) {
    // pthread_cancel
   //pthread_cancel(TCP_thread_id);
    pthread_cancel(UDPThreadID);

    // cleanup
    close(client_udp_fd);

    // exit
    exit(0);
}
