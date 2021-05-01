#include "include.h"

int main(int argc, char *argv[]) {

    signal(SIGINT,sigint);
    // init variables

    init();
    // start thread to accept TCP conn >> admin CLI
    pthread_create(&TCPThreadID, NULL, TCPWorker, NULL);


    // handle & process UDP packets

    pause();

}

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


        if( strcmp(arguments[0], "LIST") == 0){

          if(counter == 1){
            listUsers();
          }

          else {
            sprintf(reply, "Invalid number of arguments \n");
            write(tcp_client_fd, reply, strlen(reply) + 1);
          }

        }

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

//Function that lists all the users in the userlist variable
void listUsers(){

  char reply[REPLY_SIZE];

  sprintf(reply, "--- User Id / IP / Password / Client-Server / P2P / Group --- \n\n");

  write(tcp_client_fd, reply, strlen(reply) + 1);

  for (int i = 0 ; i < MAX_USERS; i++ ){


  //means that a there are no more users in the array
  if (strcmp(userlist[i].username, "") == 0 ) {
    break;
  }

  sprintf(reply , "%s / %s / %s / %s / %s / %s \n", userlist[i].username, userlist[i].ip, userlist[i].password, userlist[i].client_server, userlist[i].p2p, userlist[i].group);
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
    if(strcmp(userlist[i].username, "") == 0 ) break;

    if(strcmp(userlist[i].username, arguments[1]) == 0) return 0;
  }
  return 1;

}

//Function that adds a user to the userlist and the file with all of the users if there is space to do so
void addUser(char arguments[MAX_ARGUMENTS][ARGUMENT_LEN]){

  char reply[REPLY_SIZE];

  for (int i = 0 ; i < MAX_USERS; i++) {

    //means there is space for a new user to be added
    if (strcmp(userlist[i].username, "") == 0) {

      strcpy(userlist[i].username, arguments[1]);
      strcpy(userlist[i].ip, arguments[2]);
      strcpy(userlist[i].password, arguments[3]);
      strcpy(userlist[i].client_server, arguments[4]);
      strcpy(userlist[i].p2p, arguments[5]);
      strcpy(userlist[i].group, arguments[6]);

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


//Removes a user from both the global userlist variable and the file that has all of the information about the users
void deleteUser(char arguments[MAX_ARGUMENTS][ARGUMENT_LEN]){

  char reply[REPLY_SIZE];

  for (int i = 0 ; i < MAX_USERS; i++){

    //a user was found to remove
    if(strcmp(userlist[i].username, arguments[1]) == 0){

      //we will move all the values in front of the values 1 space backwards (deleting the current value in i )
      for (int j = i; j < MAX_USERS - 1; j++ ){
        userlist[j] = userlist[j+1];
      }

      //means that the last user was no empty (it is duplicate currently in both the position MAX_USERS-1 and MAX_USERS-2)
      if (strcmp(userlist[MAX_USERS-1].username,"") != 0) {

        strcpy(userlist[MAX_USERS-1].username, "");
        strcpy(userlist[MAX_USERS-1].ip, "");
        strcpy(userlist[MAX_USERS-1].password, "");
        strcpy(userlist[MAX_USERS-1].client_server, "");
        strcpy(userlist[MAX_USERS-1].p2p, "");
        strcpy(userlist[MAX_USERS-1].group, "");

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
    if ( strcmp(userlist[i].username,"") ==0) break;

    char new_file_line[BUFLEN];

    sprintf(new_file_line, "%s %s %s %s %s %s\n", userlist[i].username,userlist[i].ip,userlist[i].password,userlist[i].client_server, userlist[i].p2p,userlist[i].group);
    fputs(new_file_line, fp);
  }

  fclose(fp);
}



//in case an error occurs
void erro(char *s) {
	perror(s);
	exit(1);
}

void init(){

  fp = fopen(file_name, "r");

  if (fp == NULL){
   erro("Error while opening the file\n");
  }

  //Initializes TCP connection
  bzero((void *) &tcp_addr, sizeof(tcp_addr));
  tcp_addr.sin_family = AF_INET;
  tcp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  tcp_addr.sin_port = htons(TCP_PORT);

  //Creates socket, bind and listen
  if ( (tcp_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  erro("na funcao socket");
  if ( bind(tcp_fd,(struct sockaddr*)&tcp_addr,sizeof(tcp_addr)) < 0)
  erro("na funcao bind");
  if( listen(tcp_fd, 5) < 0)
  erro("na funcao listen");

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
          erro("Something is wrong in the input file\n");
        }

        strcpy(arguments[arg_counter], token);
        arg_counter++;
        token  = strtok(NULL, " ");
      }

      strcpy(userlist[file_counter].username, arguments[0]);
      strcpy(userlist[file_counter].ip, arguments[1]);
      strcpy(userlist[file_counter].password, arguments[2]);
      strcpy(userlist[file_counter].client_server, arguments[3]);
      strcpy(userlist[file_counter].p2p, arguments[4]);
      strcpy(userlist[file_counter].group, arguments[5]);

      file_counter++;
  }

  fclose(fp);
}

void sigint(int signum) {
    // pthread_cancel
    pthread_cancel(TCPThreadID);
    // cleanup
    exit(0);
    // exit
}
