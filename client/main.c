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
  answer[0] = '\0';

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

  //TO-DO :P
}

void multicastFunc(){


    char request[BUFLEN];
    int group = 0;
    int choice;

    printf("Options : 1 - Create a group | 2- Join a group");
    scanf("%d",&choice);

    if(choice == 1){
        //sprintf(request, "mode=3&from=%s&group_mode=1&group_join=0");
        sendto(client_udp_fd, request, strlen(request), MSG_CONFIRM, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
    }
    else if(choice == 2){
        //sprintf(request, "mode=3&from=%s&group_mode=2&group_join=0");
        printf("wetf uipwe tui uioer uioer\n");
    }
    else{
        printf("Invalid choice!\n");
    }


}

void error(char *s) {
	perror(s);
	exit(1);
}

void* UDPWorker() {

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

        printf(">%s\n>>", udp_buf); // @TODO raw data, beautify this in the end. use this to debug before beautifying
        fflush(stdout);
    }

}
