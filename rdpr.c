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

#define MAXBUFLEN 256

int main(int argc, char *argv[]) {
	int sockfd;
	int portno;
	char *address;
	struct sockaddr_in recv_addr, sender_addr;
  socklen_t sender_len;
  char buffer[MAXBUFLEN];   

	// Confirm command line args
  if ( argc != 4 ) {
      printf( "Usage: %s <ip> <port> <file>\n", argv[0] );
      fprintf(stderr,"REC:ERROR: Incorrect input.\n");
      return -1;
  }

  // Create the UDP socket
  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 ) {
  	fprintf(stderr, "REC:ERROR: error on socket()\n");
  	return -1;
  }

  bzero((char *) &recv_addr, sizeof(recv_addr));
  portno = atoi(argv[2]);
  address = argv[1];
  recv_addr.sin_family = AF_INET;
  recv_addr.sin_addr.s_addr = inet_addr(address);
  recv_addr.sin_port = htons(portno);

  if ( bind(sockfd, (struct sockaddr *) &recv_addr, sizeof(recv_addr)) < 0)
  {
  	close(sockfd);
  	fprintf(stderr, "REC:ERROR: error on bind()\n" );
  	return -1;
  }

  // We are now connected and serving
  printf("rdpr is running on RDP port %i.\n", portno);

  // Running loop
  while ( 1 ) {
    sender_len = sizeof(sender_addr);

    int numbytes;
    if ((numbytes = recvfrom(sockfd, buffer, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&sender_addr, &sender_len)) == -1) {
        perror("sws: error on recvfrom()!");
        return -1;
    }

    printf("%s\n", buffer);


  }

	return -1;
}