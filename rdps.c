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

#define MAXPACKETSIZE 1024
#define MAXBUFLEN 256

#define DAT_TYPE 0
#define ACK_TYPE 1
#define SYN_TYPE 2
#define FIN_TYPE 3
#define RST_TYPE 4

#define MAG_FIELD 0
#define TYP_FIELD 1
#define SEQ_FIELD 2
#define ACK_FIELD 3
#define LEN_FIELD 4
#define WIN_FIELD 5
#define DIP_FIELD 6
#define DPT_FIELD 7
#define SIP_FIELD 8
#define SPT_FIELD 9 
#define DAT_FIELD 10

#define PROTOCOL_TOKEN "UVicCSc361"

struct packet {
  int type;
  int seqnum;
  int acknum;
  int payload;
  int window;
  int data;
  char* dest_ip;
  int dest_port;
  char* src_ip;
  int src_port;
};

// Builds a packet to be sent over the wire
char* buildRDPHeader(int type, int seqnum, int acknum, int payloadlength, int winsize, char* destip, int destportno, char*srcip, int srcportno);

// Builds a packet based on a previous packet
void buildRDPPacket(char *previous_flag, int seqnum, int acknum, int payloadlength, int winsize, char* destip, int destportno, char*srcip, int srcportno, char *payload, int sockfd, struct sockaddr_in addr, socklen_t len);

// Set portNo on an addr
struct sockaddr_in setAddressAndPortNo(char *addr, int portno);

// Send a string to the client
int sendPacket(int sockfd, char *packet, struct sockaddr_in addr, socklen_t len);

// Confirm the file exists
int checkFilePath(char *loc);

// Converts a string to an int
int strToInt(char *string);

// Prints a packet to STD out
void printPacket(struct  packet p);

int main(int argc, char *argv[]) {
	int sockfd;
	int portno;
	int dest_portno;
	char *address;
	char *dest_address;
  char *file_path;
	struct sockaddr_in recv_addr, sender_addr;
  char buffer[MAXBUFLEN]; 
  socklen_t recv_len, sender_len;

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

  // Parse the params
  bzero((char *) &recv_addr, sizeof(recv_addr));
  address = argv[1];
  dest_address = argv[3];
  portno = atoi(argv[2]);
  dest_portno = atoi(argv[4]);
  file_path = argv[5];

  // Connect and bind
  recv_addr = setAddressAndPortNo(dest_address, dest_portno);
  sender_addr = setAddressAndPortNo(address, portno);

  // TODO: change this to use setsockopt with SO_REUSEADDR
  // refrence: http://www.beej.us/guide/bgnet/output/html/multipage/setsockoptman.html
  if ( bind(sockfd, (struct sockaddr *) &sender_addr, sizeof(sender_addr)) < 0) {
    close(sockfd);
    fprintf(stderr, "SEN: Error error on bind()\n" );
    return -1;
  }

  recv_len = sizeof(recv_addr);
  sender_len = sizeof(sender_addr);

  // Read file into buffer
  if ( checkFilePath(file_path) < 0 ) {
    fprintf(stderr, "SEN: Error- no file found to send\n");
  }

  char *payload = NULL;
  FILE *fp = fopen(file_path, "r");
  if (fp != NULL) {

    if (fseek(fp, 0L, SEEK_END) == 0) {

      long bufsize = ftell(fp);
      if (bufsize == -1) { 
        fprintf(stderr, "SEN: Error error on bufsize\n" );
      }

      payload = malloc(sizeof(char) * (bufsize + 1));

      if (fseek(fp, 0L, SEEK_SET) != 0) { 
        fprintf(stderr, "SEN: Error error fseek\n" );
      }

      size_t newLen = fread(payload, sizeof(char), bufsize, fp);
      if (newLen == 0) {
        fprintf(stderr, "SEN: Error reading file");
      } else {
        payload[++newLen] = '\0'; 
      }
    }
    fclose(fp);
  }

  printf(">>>%s\n", buildRDPHeader(DAT_TYPE, 0, 0, 0, 0, dest_address, dest_portno, address, portno));

  // We are now connected and serving
  printf("rdps is running on RDP port %i.\n", portno);

  // Initialize protocol
  buildRDPPacket("init", 0, 0, 0, 0, dest_address, dest_portno, address, portno, payload, sockfd, recv_addr, recv_len);

  // Running loop
  while ( 1 ) {

    int numbytes;
    if ((numbytes = recvfrom(sockfd, buffer, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&recv_addr, &recv_len)) == -1) {
        perror("SEN: Error on recvfrom()!");
        return -1;
    }

  }
 
  free(payload);
	return -1;
}

void buildRDPPacket(char *previous_flag, int seqnum, int acknum, int payloadlength, int winsize, char* destip, int destportno, char*srcip, int srcportno, char *payload, int sockfd, struct sockaddr_in addr, socklen_t len) {
  int type = -1; 
  char* packet = NULL;

  if ( strcmp("DAT", previous_flag) == 0 ) {

  } else if ( strcmp("ACK", previous_flag) == 0 ) {

  } else if ( strcmp("SYN", previous_flag) == 0 ) {
    
  } else if ( strcmp("FIN", previous_flag) == 0 ) {
    
  } else if ( strcmp("RST", previous_flag) == 0 ) {
    
  } else if ( strcmp("init", previous_flag) == 0 ) {
    packet = buildRDPHeader(SYN_TYPE, 0, 0, 0, 0, destip, destportno, srcip, srcportno);
  } else {

  }
  sendPacket(sockfd, packet, addr, len);
}

// TODO: ADD SOURCE AND DEST PORT NUM HERE
char* buildRDPHeader(int type, int seqnum, int acknum, int payloadlength, int winsize, char* destip, int destportno, char*srcip, int srcportno) {

  static char header[MAXBUFLEN];
  char *typeString = NULL;

  switch ( type ) {
    case DAT_TYPE:
      typeString = "DAT";
      break;
    case ACK_TYPE:
      typeString = "ACK";
      break;
    case SYN_TYPE:
      typeString = "SYN";
      break;
    case FIN_TYPE:
      typeString = "FIN";
      break;
    case RST_TYPE:
      typeString = "RST";
      break;
    default:
      typeString = "RST";
      break;
  }

  sprintf(header, "%s %s %i %i %i %i %s %i %s %i\n\n\0", PROTOCOL_TOKEN, typeString, seqnum, acknum, payloadlength, winsize, destip, destportno, srcip, srcportno);

  char *finishedHeader = (char*) &header;
  return finishedHeader;
}

struct sockaddr_in setAddressAndPortNo(char *addr, int portno) {

  struct sockaddr_in to;
  to.sin_family = AF_INET;
  to.sin_addr.s_addr = inet_addr(addr);
  to.sin_port = htons(portno);  

  return to;
}

int sendPacket(int sockfd, char *packet, struct sockaddr_in addr, socklen_t len) {
    if ((sendto(sockfd, packet, strlen(packet), 0, (struct sockaddr *)&addr, len)) == -1) {
        perror("Error sending packet to socket.");
        return -1;
    }   
    return 1;
}

int checkFilePath(char *loc) {
    return access(loc, F_OK);
}

// TODO: make this function more robust
int strToInt(char *string) {
  return atoi(string);
}

void printPacket(struct packet p) {
  printf("Packet: \n\t  type: %d      \n\t seqnum: %d    \n\t acknum: %d    \n\t payload: %d   \n\t window: %d    \n\t data: %s      \n\t dest_ip: %s   \n\t dest_port: %d \n\t src_ip: %s    \n\t src_port: %d  \n\t \n", p.type, p.seqnum, p.acknum, p.payload, p.window, p.data, p.dest_ip, p.dest_port, p.src_ip, p.src_port);
}

 /*
  The buffer is the buffer you're working with, you can clear space in it as you write things to the file.
 */

 /*
  Reciever initiates a file, thats where all its stored data is saved
  Sender specifies a file to send to the reciever
  */

  /*
  Sender intitiates by having the recievers IP the packets from then on containt where the IP:PoRTNO of the next packet will come from 
  */

  /*
  Packets items and flags are seperated by a space 
  */

  /*
  int size = strlen(payload);
  int firstCount = 0;

  while (firstCount < (size-9)) {
    char str[10];
    strncpy ( str, payload+firstCount, 9);
    str[9] = '\0';
    sendString(sockfd, str, recv_addr, recv_len);
    firstCount = firstCount + 9;
  }
  printf("sz %d\n", size);
  printf("cnt %d\n", firstCount);
  */

  /*
  For assign: Reciever has some buffer (window) as the buffer gets filled, window size changes, on the reciever end, it can read its buffer into the file and create more space in its window.

  - The packet on SYN will send a packet to the receiver with its IP/Port/ Dst/port this goes to the NAT which will make some changes according to translation.
  - The router keeps a table of src_ip, src_port, dst_ip, dst_port, when it gets the first packet it makes an entry
  - Then does a network address translation on the entry

  NAT and router destinations:
  - dst_ip/port and src_ip/port are fields in the header
  - we pretend the NAT is translating these addresses and take them as they are recieved by either client
  */