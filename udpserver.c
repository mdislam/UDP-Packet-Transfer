#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFLEN 51
#define NPACK 2
#define PORT 9930
#define MAX 255
#define SERVER_IP "127.0.0.1"

// packet structure
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

int main() {
	int sfd, n, i;
	socklen_t len;
	char line[BUFLEN];
	struct UdpPacket *pck;
	struct timeval end;
	
	long curr_rx_ts, prev_rx_ts, pck_tx_time, sleep_t_server_cal;
	
	FILE *file; // file for recording packet information
	
	/* 
	 * socketaddr_in is a structure containing an Internet socket address.
	 * It contains: an address family, a port number, an IP address
	 * server will listen in 'saddr' socket and client socket is 'caddr'
	 */
	struct sockaddr_in saddr, caddr;
	
	/* 
	 * Create a socket.
	 * AF_INET says that it will be an Internet socket.
	 * SOCK_DGRAM says that it will use datagram delivery instead of virtual circuits.
	 * IPPROTO_UDP says that it will use the UDP protocol
	 */
	if((sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		diep("socket");
	
	// Initialize saddr strucure, filling with binary zeros
	bzero(&saddr, sizeof(saddr));
	
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(PORT); // htons() ensures that the byte order is correct (Host TO Network order/Short integer)
	saddr.sin_addr.s_addr = htonl(INADDR_ANY); // ??
	
	// the socket sfd should be bound to the address in saddr.
	if(bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
		diep("bind");

	printf("Server Running... ...\n");
	
	file = fopen("log_data.txt", "w");
	fprintf(file, "ID\tSlp_C\tSlp_S\tTX_Time\tSend_TS\n");
	fclose(file);

	len = sizeof(caddr);
	curr_rx_ts = prev_rx_ts = 0;
	
	file = fopen("log_data.txt", "a");
	
	pck  = malloc(sizeof(struct UdpPacket));
	
	while(1){
		/*
		 * Receive a packet from sfd, that the data should be put into line
		 * line can store at most BUFLEN characters
		 * The zero parameter says that no special flags should be used
		 * Data about the sender should be stored in caddr, which has room for len byte
		 */
	    int ret = recvfrom(sfd, line, sizeof(struct UdpPacket) + 10, 0, (struct sockaddr *)&caddr, &len);
	    if(ret == -1)
			diep("recvfrom()");
		// getting the receiveing timestamp
		gettimeofday(&end, NULL);
		
		curr_rx_ts = end.tv_sec * 1000000 + end.tv_usec;

		
		pck = (struct UdpPacket *)line;	
		
		// calculating the transfer time of a packet
		pck_tx_time = ((end.tv_sec * 1000000 + end.tv_usec) - pck->send_time);
		if(prev_rx_ts == 0)
		  sleep_t_server_cal = pck->sleep_time;
		else
		  sleep_t_server_cal = curr_rx_ts - prev_rx_ts - pck_tx_time;
		
		printf("Received packet %d from %s:%d\n", pck->pck_id, inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
		//printf("Packet ID: %d, Sending TS: %ld, Transfer time: %ld, sleep time: %ld\n", pck->pck_id, pck->send_time, pck_tx_time, sleep_t_server_cal);
		
		prev_rx_ts = curr_rx_ts;
		
		
		fprintf(file, "%d\t%ld\t%ld\t%ld\t\t%ld\n", pck->pck_id, pck->sleep_time, sleep_t_server_cal, pck_tx_time, pck->send_time);
	}
	
	free(pck);
	
	fclose(file);
	close(sfd);
	return 0;
}
