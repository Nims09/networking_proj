/*
 * ---------------
 * Nathan Mots V00718293
 * CSC361 Assign: P2
 * Sender program
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
	int dest_portno;
	char *address;
	char *dest_address;
	struct sockaddr_in recv_addr, sender_addr;
  char buffer[MAXBUFLEN]; 
  socklen_t cli_len, serv_len;

	// Confirm command line args
  if ( argc != 6 ) {
      printf( "Usage: %s <ip> <port> <dest:ip> <dest:port> <file>\n", argv[0] );
      fprintf(stderr,"SEN:ERROR: Incorrect input.\n");
      return -1;
  }

  // Create the UDP socket
  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 ) {
  	fprintf(stderr, "SEN:ERROR: error on socket()\n");
  	return -1;
  }

  bzero((char *) &recv_addr, sizeof(recv_addr));
  address = argv[1];
  dest_address = argv[3];
  portno = atoi(argv[2]);
  dest_portno = atoi(argv[4]);

  recv_addr.sin_family = AF_INET;
  recv_addr.sin_addr.s_addr = inet_addr(dest_address);
  recv_addr.sin_port = htons(dest_portno);

  sender_addr.sin_family = AF_INET;
  sender_addr.sin_addr.s_addr = inet_addr(address);
  sender_addr.sin_port = htons(portno);

  if ( bind(sockfd, (struct sockaddr *) &sender_addr, sizeof(sender_addr)) < 0)
  {
  	close(sockfd);
  	fprintf(stderr, "SEN: Error error on bind()\n" );
  	return -1;
  }

  // We are now connected and serving
  printf("rdps is running on RDP port %i.\n", portno);

  // Running loop
  while ( 1 ) {
    cli_len = sizeof(sender_addr);
    serv_len = sizeof(recv_addr);

    if ((sendto(sockfd, ">test<", strlen(">test<"), 0, (struct sockaddr *)&recv_addr, serv_len)) == -1) {
        perror("SEN: Error sending string to socket.");
        return -1;
    }   

    int numbytes;
    if ((numbytes = recvfrom(sockfd, buffer, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&sender_addr, &cli_len)) == -1) {
        perror("SEN: Error on recvfrom()!");
        return -1;
    }

  }

	return -1;
}