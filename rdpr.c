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

#define WINDOWSIZE 4096
#define MAXPACKETSIZE 1024
#define DATAPACKETSIZE 1000
#define MAXBUFLEN 256

#define INIT_TYPE -1
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
  char* data;
  char* dest_ip;
  int dest_port;
  char* src_ip;
  int src_port;
};

// Builds a packet to be sent over the wire
char* buildRDPHeader(int type, int seqnum, int acknum, int payloadlength, int winsize, char* destip, int destportno, char*srcip, int srcportno);

// Builds a packet based on a previous packet
void buildRDPPacket(int flag, int seqnum, int acknum, int payloadlength, int winsize, char* destip, int destportno, char*srcip, int srcportno, char *payload, int sockfd, struct sockaddr_in addr, socklen_t len);

// Parses a string packet into a struct
struct packet parsePacket(char* packet);

// Set portNo on an addr
struct sockaddr_in setAddressAndPortNo(char *addr, int portno);

// Send a string to the client
int sendPacket(int sockfd, char *buffer, struct sockaddr_in addr, socklen_t len);

// Confirm the file exists
int checkFilePath(char *loc);

// Converts a string to an int
int strToInt(char *string);

// Prints a packet to STD out
void printPacket(struct  packet p);

int main(int argc, char *argv[]) {
	int sockfd;
	int portno;
	char *address;
  char *file_path;  
  int numbytes;
  struct sockaddr_in recv_addr, sender_addr;
  socklen_t sender_len, recv_len;
  // TODO: fix this max bufflen its grabbing extra chars
  char buffer[MAXBUFLEN]; 
  char window[WINDOWSIZE];
  int optval = 1;

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

  if ( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1 ) {
    fprintf(stderr, "SEN: Error error on setsockopt()\n" );    
  }

  if ( bind(sockfd, (struct sockaddr *) &recv_addr, sizeof(recv_addr)) < 0) {
  	close(sockfd);
  	fprintf(stderr, "REC: Error on error on bind()\n" );
  	return -1;
  }
  sender_len = sizeof(sender_addr);


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
  printf("tracce %s\n", text);

  // We are now connected and serving
  printf("rdpr is running on RDP port %i.\n", portno);

  // Running loop
  while ( 1 ) {
    struct packet p;
    
    // SYN Handshake
    while ( 1 ) {

      if ((numbytes = recvfrom(sockfd, buffer, MAXBUFLEN-1 , 0,
          (struct sockaddr *)&sender_addr, &sender_len)) == -1) {
          perror("REC: error on recvfrom()!");
          return -1;
      }

      p = parsePacket(buffer);

      if ( p.type == SYN_TYPE ) {
        buildRDPPacket(SYN_TYPE, p.acknum, (p.seqnum+1), 0, sizeof(window), address, portno, p.src_ip, p.src_port, "", sockfd, sender_addr, sender_len);
        break;
      } else {
        continue;
      }    
    }

    // Begin accepting data
    while ( 1 ) {
      if ((numbytes = recvfrom(sockfd, buffer, MAXBUFLEN-1 , 0,
          (struct sockaddr *)&sender_addr, &sender_len)) == -1) {
          perror("REC: error on recvfrom()!");
          return -1;
      }
      p = parsePacket(buffer);

      buildRDPPacket(ACK_TYPE, p.acknum, (p.seqnum+1), 0, 0, p.dest_ip, p.dest_port, p.src_ip, p.src_port, "", sockfd, sender_addr, sender_len);

      // printPacket(p);
      printf("%s", p.data);
      fprintf(write_file, "%s", p.data);

      // TODO Better implement this
      if ( p.type == FIN_TYPE ) {
        break;
      }
    }
    fclose(write_file); // TODO: Move me to the end // XXX
    break;

  }

	return -1;
}

void buildRDPPacket(int flag, int seqnum, int acknum, int payloadlength, int winsize, char* destip, int destportno, char*srcip, int srcportno, char *payload, int sockfd, struct sockaddr_in addr, socklen_t len) {
  int type = -1; 
  static char packet[MAXPACKETSIZE];
  char* header = NULL;

  header = buildRDPHeader(flag, 0, 0, 0, 0, destip, destportno, srcip, srcportno);    

  if ( flag != DAT_TYPE ) {
    payload = "";
  }

  sprintf(packet, "%s%s\n\0", header, payload);
  sendPacket(sockfd, packet, addr, len);
}


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

  sprintf(header, "%s %s %i %i %i %i %s %i %s %i \n\n\0", PROTOCOL_TOKEN, typeString, seqnum, acknum, payloadlength, winsize, destip, destportno, srcip, srcportno);

  char *finishedHeader = (char*) &header;
  return finishedHeader;
}

struct packet parsePacket(char* packet) {
  int currentToken = 0;
  struct packet p;

  char* token = strtok(packet, " ");
  while ( token && currentToken < DAT_FIELD) {

    switch ( currentToken ) {
      case MAG_FIELD:
        if ( strcmp(token, PROTOCOL_TOKEN) != 0 ) {
          p.type = RST_TYPE;
        }
        break;
      case TYP_FIELD:
        if ( strcmp("DAT", token) == 0 ) {
          p.type = DAT_TYPE;
        } else if ( strcmp("ACK", token) == 0 ) {
          p.type = ACK_TYPE;
        } else if ( strcmp("SYN", token) == 0 ) {
          p.type = SYN_TYPE;
        } else if ( strcmp("FIN", token) == 0 ) {
          p.type = FIN_TYPE;
        } else if ( strcmp("RST", token) == 0 ) {
          p.type = RST_TYPE;
        } else if ( strcmp("init", token) == 0 ) {
          p.type = RST_TYPE;
        } else {
          p.type = RST_TYPE;
        }
        break;
      case SEQ_FIELD:
        p.seqnum = strToInt(token);
        break;
      case ACK_FIELD:
        p.acknum = strToInt(token);
        break;
      case LEN_FIELD:
        p.payload = strToInt(token);
        break;
      case WIN_FIELD:
        p.window = strToInt(token);
        break;
      case DIP_FIELD:
        p.dest_ip = token;
        break;
      case DPT_FIELD:
        p.dest_port = strToInt(token);
        break;
      case SIP_FIELD:
        p.src_ip = token;
        break;
      case SPT_FIELD:
        p.src_port = strToInt(token);
        break;
      case DAT_FIELD:
        // This should just be what is left if its a data type
        break;
      default:
        break;
    }
    currentToken++;
    token = strtok(NULL, " ");
  }

  if (p.type == DAT_TYPE) {
    token = strtok(NULL, "\0");
    p.data = token;
  } else {
    p.data = "";
  }

  return p;
}

struct sockaddr_in setAddressAndPortNo(char *addr, int portno) {

  struct sockaddr_in to;
  to.sin_family = AF_INET;
  to.sin_addr.s_addr = inet_addr(addr);
  to.sin_port = htons(portno);  

  return to;
}

int sendPacket(int sockfd, char *buffer, struct sockaddr_in addr, socklen_t len) {
    if ((sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&addr, len)) == -1) {
        perror("Error sending string to socket.");
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