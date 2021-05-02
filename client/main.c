#include "include.h"

int main(int argc, char *argv[]) {

}

void error(char *s) {
	perror(s);
	exit(1);
}