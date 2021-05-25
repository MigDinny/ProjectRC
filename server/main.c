#include "include.h"

int main(int argc, char *argv[]) {

    if (argc != 4) {
      printf("server <porto clientes> <porto config> <ficheiro registos> \n");
      exit(0);
    }

    strcpy(file_name, argv[3]);
    
    UDP_PORT = atoi(argv[1]);
    TCP_PORT = atoi(argv[2]);

    signal(SIGINT, sigint);
    // init variables
    init();

    // start thread to accept TCP conn >> admin CLI
    //pthread_create(&TCP_thread_id, NULL, TCPWorker, NULL);
    pthread_create(&TCPThreadID, NULL, TCPWorker, NULL);

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

    // init TCP socket && user_list

    init_tcp();

}

void loopUDP() {

    int status = 0;

    while (1) {

        udp_recv_len = recvfrom(udp_fd, udp_buf, BUFLEN, 0, (struct sockaddr *) &udp_ext_socket, (socklen_t *)&udp_ext_len);
        if (udp_recv_len == -1) continue;

        udp_buf[udp_recv_len] = '\0';

        printf("buffer: %s\n", udp_buf);
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

/* »»»»»»»»»»»»»»»»»»»»»»»»»»»»»»» */

//user
int auth() {

    char answer[BUFLEN];
    answer[0] = '\0';
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

        sprintf(answer, "ACCESS DENIED");
        break;

      }

      //means username has a match
      if (strcmp(user_list[i].username, username) == 0) {

        //checks to see if the pasword matches

        if (strcmp(user_list[i].password, password) == 0) {
          char authorizations[300];
          authorizations[0] = '\0';

          if (strcmp(user_list[i].client_server,"yes") == 0)  strcat(authorizations, "|1 - Client-Server");
          if (strcmp(user_list[i].p2p,"yes") == 0) strcat(authorizations, "|2 - P2P");
          if(strcmp(user_list[i].group, "yes") == 0) strcat(authorizations, "|3 - Group");

          printf("%s\n",authorizations);
          sprintf(answer, "OPTIONS: %s",authorizations);
          break;
        }

        //due to the username being unique if the password is denied, the access is denied
        else {

          sprintf(answer, "ACCESS DENIED");

          sendto(udp_fd, answer, strlen(answer), MSG_CONFIRM, (struct sockaddr *) &udp_ext_socket, sizeof(udp_ext_socket));
          return -1;
        }


      }
    }

    // send response

	sendto(udp_fd, answer, strlen(answer), MSG_CONFIRM, (struct sockaddr *) &udp_ext_socket, sizeof(udp_ext_socket));

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


    // fetch IP of destuser
    for (int i = 0 ; i < MAX_USERS; i++) {

      if (strcmp(user_list[i].username, "") == 0){

        sprintf(answer, "INVALID DESTINATION USER");
        sendto(udp_fd, answer, BUFLEN, MSG_CONFIRM, (struct sockaddr *) &udp_ext_socket, sizeof(udp_ext_socket));
        return -1;

      }

      //means username has a match
      if (strcmp(user_list[i].username, destuser) == 0) {

        sprintf(answer, "%s", user_list[i].ip);
        break;

      }

    }

    // send response with IP

    // or send invalid destuser
	  sendto(udp_fd, answer, strlen(answer), MSG_CONFIRM, (struct sockaddr *) &udp_ext_socket, sizeof(udp_ext_socket));

    return 0;
}

int reqMulticast() {

    char answer[BUFLEN];
    answer[0] = '\0';
    char *group_mode = udp_pairs[1];
	printf("1231 %s", answer);
    strtok(group_mode, "=");
    group_mode = strtok(NULL, "=");

    //Create new group
    if ( strcmp(group_mode,"1") == 0 ) {

        if (address_counter < 3) address_counter++;

        //Means that the limit of amount of groups has been reached.
        else{
            strcpy(answer, "No space to create a new group");
            sendto(udp_fd, answer, strlen(answer), MSG_CONFIRM, (struct sockaddr *) &udp_ext_socket, sizeof(udp_ext_socket));
        }

    }
    //Join group
    else{

        //show possible ip groups
        if (address_counter == 0) {
            strcpy(answer, "No groups created");
        }

        //Means that atleast 1 group has been created. Returns options to the user
        else {
            char aux[200];
            aux[0] = '\0';
            for (int i = 0; i < address_counter; i++) {

                if(i == 0) sprintf(aux, "%s", addresses[i]);
                else sprintf(aux, "-%s", addresses[i]);

                strcat(answer, aux);
            }
        }
		printf("STRING %s\n", answer);
        sendto(udp_fd, answer, strlen(answer), MSG_CONFIRM, (struct sockaddr *) &udp_ext_socket, sizeof(udp_ext_socket));
    }

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


    // fetch IP from destuser
    for (int i = 0 ; i < MAX_USERS; i++) {

      if (strcmp(user_list[i].username, "") == 0){

        sprintf(answer, "INVALID DESTINATION USER");
        sendto(udp_fd, answer, strlen(answer), MSG_CONFIRM, (struct sockaddr *) &udp_ext_socket, sizeof(udp_ext_socket));
        return -1;

      }

      //means username has a match
      if (strcmp(user_list[i].username, destuser) == 0) {

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

	  if ((dest_fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) == -1)
		  error("Problem creating the socket");

	  dest_addr.sin_family = AF_INET;
	  dest_addr.sin_port   = htons((short) CLIENT_PORT);
	  dest_addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;


    // send message like this "<username>: message"
    sprintf(answer, "%s",message);
    sendto(dest_fd, answer, strlen(answer), MSG_CONFIRM, (struct sockaddr *) &dest_addr, sizeof(dest_addr));

    return 0;
}



/* »»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»» */


void *TCPWorker(void *id) {


  //Waits for a connection
  while (1) {

    tcp_client_fd = accept(tcp_fd,(struct sockaddr *)&tcp_client_addr,(socklen_t *)&tcp_client_addr_size);

    //sucessfull

    if (tcp_client_fd > 0) {

      int nread = 0;
      char buffer[BUFLEN];
      char reply[REPLY_SIZE];

      while (1) {

        nread = read(tcp_client_fd, buffer, BUFLEN-1);

        if(nread <=0) break;

        //remove \n from netcat input
        buffer[nread-1] = '\0';

        //split the buffer where it has a space (" ") so that we can extract all of the argument given
        char arguments[MAX_ARGUMENTS][ARGUMENT_LEN];
        int counter = 0;
        char *token  = strtok(buffer, " ");

        //while there are more spaces left we keep adding the arguments
        while( token != NULL){

          if(counter == MAX_ARGUMENTS){
            counter++;
            sprintf(reply, "Too many arguments \n");
            write(tcp_client_fd, reply, strlen(reply) + 1);

            break;
          }

          strcpy(arguments[counter], token);
          counter++;
          token  = strtok(NULL, " ");
        }

        //LIST functionality
        if( strcmp(arguments[0], "LIST") == 0){

          if(counter == 1){
            listUsers();
          }

          else {
            sprintf(reply, "Invalid number of arguments \n");
            write(tcp_client_fd, reply, strlen(reply) + 1);
          }

        }

        //ADD funcionality
        else if (strcmp(arguments[0], "ADD") == 0) {

          //make sure there are 8 arguments
          if ( counter == 7 && (valueVerification(arguments) == 1)) {
            addUser(arguments);

          }

          else {

            sprintf(reply, "Invalid number of arguments or invalid information \n");
            write(tcp_client_fd, reply, strlen(reply) + 1);

          }

        }
        //Delete functionality
        else if (strcmp(arguments[0], "DEL") == 0){

          if( counter == 2 ){
            deleteUser(arguments);
          }

          else{

            sprintf(reply, "Invalid number of arguments\n");
            write(tcp_client_fd, reply, strlen(reply) + 1);

          }
        }

        else if( strcmp(arguments[0], "QUIT") == 0){

          break;
        }

        else{
          sprintf(reply, "Invalid input \n");
          write(tcp_client_fd, reply, strlen(reply) + 1);
        }


      }

    close(tcp_client_fd);
    }

  }

}

//Function that lists all the users in the user_list variable
void listUsers(){

  char reply[REPLY_SIZE];

  sprintf(reply, "--- User Id / IP / Password / Client-Server / P2P / Group --- \n\n");

  write(tcp_client_fd, reply, strlen(reply) + 1);

  for (int i = 0 ; i < MAX_USERS; i++ ){


  //means that a there are no more users in the array
  if (strcmp(user_list[i].username, "") == 0 ) {
    break;
  }

  sprintf(reply , "%s / %s / %s / %s / %s / %s \n", user_list[i].username, user_list[i].ip, user_list[i].password, user_list[i].client_server, user_list[i].p2p, user_list[i].group);
  write(tcp_client_fd, reply, strlen(reply) + 1);

  }

}

//Function that verifies if the argments variable is suitable for the addUser function
int valueVerification(char arguments[MAX_ARGUMENTS][ARGUMENT_LEN]){

  if ( (strcmp(arguments[4], "yes") != 0 && strcmp(arguments[4], "no") !=0)   ||
      (strcmp(arguments[5], "yes") != 0 && strcmp(arguments[5], "no") !=0) ||
      (strcmp(arguments[6], "yes") != 0 && strcmp(arguments[6], "no") !=0)){

        return  0;
      }

  //checks if ID is unique
  for(int i = 0; i < MAX_USERS; i++){
    if(strcmp(user_list[i].username, "") == 0 ) break;

    if(strcmp(user_list[i].username, arguments[1]) == 0) return 0;
  }
  return 1;

}

//Function that adds a user to the user_list and the file with all of the users if there is space to do so
void addUser(char arguments[MAX_ARGUMENTS][ARGUMENT_LEN]){

  char reply[REPLY_SIZE];

  for (int i = 0 ; i < MAX_USERS; i++) {

    //means there is space for a new user to be added
    if (strcmp(user_list[i].username, "") == 0) {

      strcpy(user_list[i].username, arguments[1]);
      strcpy(user_list[i].ip, arguments[2]);
      strcpy(user_list[i].password, arguments[3]);
      strcpy(user_list[i].client_server, arguments[4]);
      strcpy(user_list[i].p2p, arguments[5]);
      strcpy(user_list[i].group, arguments[6]);

      sprintf(reply, "SUCCESS\n");
      write(tcp_client_fd, reply, strlen(reply) + 1);

      //TO-DO write new user to file
      fp = fopen( file_name, "a");

      char new_file_line[BUFLEN];

      sprintf(new_file_line, "%s %s %s %s %s %s\n", arguments[1],arguments[2],arguments[3],arguments[4],arguments[5],arguments[6]);
      fputs(new_file_line, fp);

      fclose(fp);
      return;
    }
  }

  sprintf(reply, "There is no more space for new users\n");
  write(tcp_client_fd, reply, strlen(reply) + 1);

}

//Removes a user from both the global user_list variable and the file that has all of the information about the users
void deleteUser(char arguments[MAX_ARGUMENTS][ARGUMENT_LEN]){

  char reply[REPLY_SIZE];

  for (int i = 0 ; i < MAX_USERS; i++){

    //a user was found to remove
    if(strcmp(user_list[i].username, arguments[1]) == 0){

      //we will move all the values in front of the values 1 space backwards (deleting the current value in i )
      for (int j = i; j < MAX_USERS - 1; j++ ){
        user_list[j] = user_list[j+1];
      }

      //means that the last user was no empty (it is duplicate currently in both the position MAX_USERS-1 and MAX_USERS-2)
      if (strcmp(user_list[MAX_USERS-1].username,"") != 0) {

        strcpy(user_list[MAX_USERS-1].username, "");
        strcpy(user_list[MAX_USERS-1].ip, "");
        strcpy(user_list[MAX_USERS-1].password, "");
        strcpy(user_list[MAX_USERS-1].client_server, "");
        strcpy(user_list[MAX_USERS-1].p2p, "");
        strcpy(user_list[MAX_USERS-1].group, "");

      }

      //re write the entire file

      writeToFile();

      sprintf(reply, "SUCCESS\n");
      write(tcp_client_fd, reply, strlen(reply) + 1);
      return;
    }
  }


  sprintf(reply, "User does not exist\n");
  write(tcp_client_fd, reply, strlen(reply) + 1);
}

//Fucntion that writes the current information to the entry file
void writeToFile(){

  fp = fopen(file_name, "w");

  for ( int i = 0 ; i < MAX_USERS; i++){
    if ( strcmp(user_list[i].username,"") ==0) break;

    char new_file_line[BUFLEN];

    sprintf(new_file_line, "%s %s %s %s %s %s\n", user_list[i].username,user_list[i].ip,user_list[i].password,user_list[i].client_server, user_list[i].p2p,user_list[i].group);
    fputs(new_file_line, fp);
  }

  fclose(fp);
}

void init_tcp(){

  fp = fopen(file_name, "r");

  if (fp == NULL){
   error("Error while opening the file\n");
  }

  //Initializes TCP connection
  bzero((void *) &tcp_addr, sizeof(tcp_addr));
  tcp_addr.sin_family = AF_INET;
  tcp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  tcp_addr.sin_port = htons(TCP_PORT);

  //Creates socket, bind and listen
  if ( (tcp_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  error("na funcao socket");
  if ( bind(tcp_fd,(struct sockaddr*)&tcp_addr,sizeof(tcp_addr)) < 0)
  error("na funcao bind");
  if( listen(tcp_fd, 5) < 0)
  error("na funcao listen");

  tcp_client_addr_size = sizeof(tcp_client_addr);

  //read fiple
  char buffer[BUFLEN];
  int file_counter = 0;

  while (fgets(buffer, BUFLEN - 1, fp)) {
      // Remove trailing newline
      buffer[strcspn(buffer, "\n")] = 0;

      char arguments[5][20];
      int arg_counter = 0;
      char *token  = strtok(buffer, " ");


      //while there are more spaces left we keep adding the arguments
      while( token != NULL){

        if(arg_counter >= 6){
          error("Something is wrong in the input file\n");
        }

        strcpy(arguments[arg_counter], token);
        arg_counter++;
        token  = strtok(NULL, " ");
      }

      strcpy(user_list[file_counter].username, arguments[0]);
      strcpy(user_list[file_counter].ip, arguments[1]);
      strcpy(user_list[file_counter].password, arguments[2]);
      strcpy(user_list[file_counter].client_server, arguments[3]);
      strcpy(user_list[file_counter].p2p, arguments[4]);
      strcpy(user_list[file_counter].group, arguments[5]);

      file_counter++;
  }

  fclose(fp);
}


/* »»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»» */

void error(char *s) {
	perror(s);
	exit(1);
}

void sigint(int signum) {
    // pthread_cancel
   //pthread_cancel(TCP_thread_id);
    pthread_cancel(TCPThreadID);

    // cleanup
    close(udp_fd);

    // exit
    exit(0);
}
