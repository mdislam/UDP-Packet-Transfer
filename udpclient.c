#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#define BUFLEN 512
#define NPACK 2
#define PORT 9930
#define MAX 255
#define SERVER_IP "127.0.0.1"

struct UdpPacket {
	int pck_id;
	long send_time;
	long sleep_time;
};

/* this function used for error handling */
void diep(char *s) {
	perror(s);
	exit(1);
}

// command line parameter should be num of packets and sleep time
int main(int argc, char **argv) {
	int sfd, n, i;
	socklen_t len;
	int pck_count;
	long sleep_time, proc_time;
	struct timeval start, end, val1;
	
	/* 
	 * socketaddr_in is a structure containing an Internet socket address.
	 * It contains: an address family, a port number, an IP address
	 */
	struct sockaddr_in caddr;

	
	if(argc != 4) {
		printf("Usage: %s ipaddress numOfPackets sleeptime(microsec)\n", argv[0]);
		return -1;
	}
	
	// assigning the second argument as number of packets
	pck_count = atoi(argv[2]);
	sleep_time = atol(argv[3]);
	
	/* 
	 * Create a socket.
	 * AF_INET says that it will be an Internet socket.
	 * SOCK_DGRAM says that it will use datagram delivery instead of virtual circuits.
	 * IPPROTO_UDP says that it will use the UDP protocol
	 */
	if((sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		diep("socket");
	
	// Initialize saddr strucure, filling with binary zeros
	bzero(&caddr, sizeof(caddr));
	
	caddr.sin_family = AF_INET;
	
	caddr.sin_port = htons(PORT); // htons() ensures that the byte order is correct (Host TO Network order/Short integer)
	//inet_pton(AF_INET, argv[1], &caddr.sin_addr);

	if (inet_aton(argv[1], &caddr.sin_addr) == 0) {
		fprintf(stderr, "inet_aton() failed.\n");
		exit(1);
	}

	//printf("Client Running... ...\n");
	
	for(i = 0; i < pck_count; i++)
	{		
		struct UdpPacket pck;
		bzero(&pck, sizeof(pck));	
		
		len = sizeof(caddr);	
		
		gettimeofday(&val1, NULL); 

		usleep(sleep_time);		
				
		//get wakeuptime ~ sendtime
		gettimeofday(&start, NULL);
		
//		printf("sleep overhead %ld, ", (start.tv_sec * 1000000 + start.tv_usec) - (val1.tv_sec * 1000000 + val1.tv_usec));
		printf("%ld\n", (start.tv_sec * 1000000 + start.tv_usec) - (val1.tv_sec * 1000000 + val1.tv_usec));
		
		pck.pck_id = i;
		pck.send_time = start.tv_sec * 1000000 + start.tv_usec;
		pck.sleep_time = sleep_time;

		/*
		 * Send sizeof(pck) bytes from pck to sfd, with no flags (0).
		 * The receiver is specified in caddr, which contains len byte. 
		 */
		if(sendto(sfd, (const char *)&pck, sizeof(pck), 0, (struct sockaddr *)&caddr, len) == -1)
			diep("sendto()");
			
		gettimeofday(&end, NULL);
		
		//printf("Sending packet %d from %s:%d, ", pck.pck_id, inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
		
		// calculating the processing time
		proc_time = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
		//printf("Send Processing time: %ld\n", proc_time);
	}
	
	
	close(sfd);	
	return 0;
}
