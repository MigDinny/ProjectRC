#include "include.h"



int main(int argc, char *argv[]) {

	if (argc != 3) {
	  printf("cliente <host> <port> \n");
	  exit(0);
	}

	init(argv[1], argv[2]);

	authentication();

	// start thread to receive incoming UDP messages
	pthread_create(&UDPThreadID, NULL, UDPWorker, NULL);

	menu();
	pause();
}

//
void authentication(){

  char username_input[30];
  char password_input[30];
  char request[BUFLEN];
  char answer[BUFLEN];

  printf("Username: ");
  scanf("%s",username_input);

  printf("Password: ");
  scanf("%s",password_input);

  sprintf(request, "mode=1&username=%s&password=%s", username_input, password_input);
  sendto(client_udp_fd, request, strlen(request), MSG_CONFIRM, (struct sockaddr *) &dest_addr, sizeof(dest_addr));

  //get answer
  recvfrom(client_udp_fd, answer , BUFLEN, 0, (struct sockaddr *) &arrival_addr, (socklen_t *)&slen);

  //means that authentication failed. The program ends (MIGUEL matamos o programa ou damos outra chance? mandar status num while até acertarem) // matamos o programa bro, nao merece viver
  if(strcmp(answer, "ACCESS DENIED") == 0){
	printf("%s\n",answer);
	exit(0);
  }

  strcpy(username , username_input);

  printf("AUTHENTICATION SUCCESSFULL!\n");

  int counter = 0;
  char *token  = strtok(answer, "|");

  //The first strtok gives always the string OPTIONS which is useless
  token = strtok(NULL, "|");

  //while there are more spaces left we keep adding the arguments
  while( token != NULL){

	strcpy( permissions[counter++], token);
	token  = strtok(NULL, "|");
  }

  printf("%s\n", answer);
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

  if ((strcmp(choice, "Client-Server") == 0)) {
	if (validChoice("Client-Server") == 0) {
	  clientServerFunc();
	} else {
	  printf("You do not have authorization for this feature!\n");
	}
  } else if ((strcmp(choice, "P2P") == 0)) {
	if (validChoice("P2P") == 0) {
	  p2pFunc();
	} else {
	  printf("You do not have authorization for this feature!\n");
	}
  } else if ((strcmp(choice, "Group") == 0)) {
	if (validChoice("Group") == 0) {
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


void clientServerFunc(){

	//TO-DO :O
	
}

void p2pFunc(){

  //TO-DO :P
}

void multicastFunc(){

  //TO-DO ;(

}

void error(char *s) {
	perror(s);
	exit(1);
}

void UDPWorker() {

	int udp_recv_len, udp_fd;
	struct sockaddr_in udp_ext_socket, udp_int_socket;
	socklen_t slen = sizeof(arrival_addr);
	char udp_buf[BUFLEN];
	socklen_t udp_ext_len = sizeof(udp_ext_socket);

	udp_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_fd == -1) error("Error creating UDP socket");

    udp_int_socket.sin_family = AF_INET;
    udp_int_socket.sin_port = htons(CLIENT_PORT);
    udp_int_socket.sin_addr.s_addr = htonl(INADDR_ANY);

    int status = bind(udp_fd, (struct sockaddr*) &udp_int_socket, sizeof(udp_int_socket));
    if (status == -1) error("Error binding UDP socket");

    while (1) {

        udp_recv_len = recvfrom(udp_fd, udp_buf, BUFLEN, 0, (struct sockaddr *) &udp_ext_socket, (socklen_t *)&udp_ext_len);
        if (udp_recv_len == -1) continue;

        udp_buf[udp_recv_len] = '\0';

        printf("%s\n", udp_buf); // @TODO raw data, beautify this in the end. use this to debug before beautifying
    }

}