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

#define MAXPACKETSIZE 1024
#define MAXBUFLEN 256

// Set portNo on an addr
struct sockaddr_in setAddressAndPortNo(char *addr, int portno);

// Send a string to the client
int sendString(int sockfd, char *buffer, struct sockaddr_in addr, socklen_t len);

// Confirm the file exists
int checkFilePath(char *loc);

int main(int argc, char *argv[]) {
	int sockfd;
	int portno;
	char *address;
  char *file_path;  
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

  // Parse the params
  bzero((char *) &recv_addr, sizeof(recv_addr));
  portno = atoi(argv[2]);
  address = argv[1];
  file_path = argv[3];

  // Connect and bind
  recv_addr = setAddressAndPortNo(address, portno);

  if ( bind(sockfd, (struct sockaddr *) &recv_addr, sizeof(recv_addr)) < 0) {
  	close(sockfd);
  	fprintf(stderr, "REC: Error on error on bind()\n" );
  	return -1;
  }

  // Open the file for writing
  if ( checkFilePath(file_path) < 0 ) {
    fprintf(stderr, "REC: Error- no file found to write too\n");
  }

  FILE *write_file = fopen(file_path, "w");
  if (write_file == NULL) {
      printf("REC: Error opening file!\n");
      exit(1);
  }

  // TODO: place this better
  char *text = "Write this to the file";
  fprintf(write_file, "TEST: Some text: %s\n", text);
  fclose(write_file); // TODO: Move me to the end

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

// void buildRDPHeader(char *header, int type, int seqnum, int acknum, int payloadlength, int winsize) {

//   char *typeString = NULL;

//   switch ( type ) {
//     case DAT_TYPE:
//       typeString = "DAT";
//       break;
//     case ACK_TYPE:
//       typeString = "ACK";
//       break;
//     case SYN_TYPE:
//       typeString = "SYN";
//       break;
//     case FIN_TYPE:
//       typeString = "FIN";
//       break;
//     case RST_TYPE:
//       typeString = "RST";
//       break;
//     default:
//       typeString = "RST";
//       break;
//   }

//   fprintf(header, "UVicCSc361 %s %i %i %i %i\n\n", seqnum, acknum, payloadlength, winsize);
// }

struct sockaddr_in setAddressAndPortNo(char *addr, int portno) {

  struct sockaddr_in to;
  to.sin_family = AF_INET;
  to.sin_addr.s_addr = inet_addr(addr);
  to.sin_port = htons(portno);  

  return to;
}

int sendString(int sockfd, char *buffer, struct sockaddr_in addr, socklen_t len) {
    if ((sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&addr, len)) == -1) {
        perror("Error sending string to socket.");
        return -1;
    }   
    return 1;
}

int checkFilePath(char *loc) {
    return access(loc, F_OK);
}