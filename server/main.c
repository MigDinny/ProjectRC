#include "include.h"

int main(int argc, char *argv[]) {

    // init variables
    pthread_t TCPThreadID;

    // start thread to accept TCP conn >> admin CLI
    pthread_create(&TCPThreadID, NULL, TCPWorker, NULL);


    // handle & process UDP packets



}

void TCPWorker() {

    // accept tcp conn

    // process it

    // loop
}


void erro(char *s) {
	perror(s);
	exit(1);
}

void sigint(int signum) {
    // pthread_cancel

    // cleanup

    // exit
}