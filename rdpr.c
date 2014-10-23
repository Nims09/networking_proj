/*
 * ---------------
 * Nathan Mots V00718293
 * CSC361 Assign: P2
 * Reciever program
 * ---------------
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h> 

int main(int argc, char *argv[]) {
	int sockfd;
	int portno;
	char *address;
	struct sockaddr_in serv_addr, cli_addr;

	// Confirm command line args
  if ( argc != 4 ) {
      printf( "Usage: %s <ip> <port> <file>\n", argv[0] );
      fprintf(stderr,"ERROR: Incorrect input.\n");
      return -1;
  }

  // Create the UDP socket
  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 ) {
  	fprintf(stderr, "ERROR: error on socket()\n");
  	return -1;
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi(argv[2]);
  address = argv[1];
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(address);
  serv_addr.sin_port = htons(portno);

  if ( bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
  {
  	close(sockfd);
  	fprintf(stderr, "ERROR: error on bind()\n" );
  	return -1;
  }

  // We are now connected and serving
  printf("rdpr is running on RDP port %i.\n", portno);

	return -1;
}