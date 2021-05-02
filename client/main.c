#include "include.h"




int main(int argc, char *argv[]) {

    if (argc != 3) {
      printf("cliente <host> <port> \n");
      exit(0);
    }

    init(argv[1], argv[2]);

    authentication();

    menu();

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

  //MIGUEL TO-DO verifica se isto cumpre o teu protocolo (acho que falta um d algures mas nao entendi onde )
  sprintf(message, "mode=1&%s&%s", username_input, password_input);
  sendto(client_udp_fd, request, strlen(request), MSG_CONFIRM, (struct sockaddr *) &dest_addr, sizeof(dest_addr));

  //get answer
  recvfrom(client_udp_fd, answer , BUFLEN, 0, (struct sockaddr *) &arrival_addr, (socklen_t *)&slen);

  //means that authentication failed. The program ends (MIGUEL matamos o programa ou damos outra chance? mandar status num while até acertarem)
  if(strcmp(answer, "ACCESS DENIED")){
    printf("%s\n",answer);
    exit(0);
  }

  printf("AUTHENTICATION SUCCESSFULL!\n");
  printf("%s\n", answer);
}


//initialize variables
void init(char *server_name, char *port){


	if ((hostPtr = gethostbyname(server_name)) == 0)
		error("Invalid address");

	if ((fd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
		error("socket");

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port   = htons((short) atoi(port));
	dest_addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;


}


void menu(){

  do{
    printf("")
    scanf("%s",)




  }while(1);

}

void error(char *s) {
	perror(s);
	exit(1);
}
