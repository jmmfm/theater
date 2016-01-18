// http://www.cplusplus.com/forum/unices/116919/

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
 
/* for EXIT_FAILURE and EXIT_SUCCESS */
#include <stdlib.h>
 
/* network functions */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>



int main(int argc, char **argv)
{
    int sockfd;
    int ret;
    char *buf;
    struct sockaddr_in servaddr;

 
    if (argc != 4)
	{
		puts("usage: sendUDP ipaddr port data");
		exit(EXIT_FAILURE);
	}
	
	servaddr.sin_family = AF_INET;
	
	// Assign remote address	/*aton = address to network*/
	ret = inet_aton(argv[1], &servaddr.sin_addr);
	if (ret == 0)
	{
		perror("inet_aton");
		exit(EXIT_FAILURE);
	}

	// Assign remote port	
	servaddr.sin_port = htons(atoi(argv[2]));
	// std::cout << "Remote port = " << ntohs(servaddr.sin_port) << std::endl;

	// String to send
	buf = argv[3];
	
	sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		perror("Failed to create socket");
		exit(EXIT_FAILURE);
	}
	
	ret = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if (ret == -1)
	{
		perror("sendto");
		exit(EXIT_FAILURE);
	}


	// Receive part ------------------------------------------------------------
	struct sockaddr_in cliaddr;
	
	// Construct the server sockaddr_ structure	
//	memset (&myaddr, 0, sizeof(myaddr));
	cliaddr.sin_family = AF_INET;
	// Bind to any addr on local machine /* puts server's IP automatically */
	cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
//	ret = inet_aton("192.168.56.102", &myaddr.sin_addr);
//	if (ret == 0)
//	{
//		perror("inet_aton");
//		exit(EXIT_FAILURE);
//	}
	// Bind to any port
	cliaddr.sin_port=htons(9999);

	// --- This is just for debug ---
	// std::cout << "addr = " << inet_ntoa(cliaddr.sin_addr) << std::endl;
	// std::cout << "port = " << ntohs(cliaddr.sin_port) << std::endl;
	
	// No need to create the socket because socket will be the same
//	sock = socket(AF_INET, SOCK_DGRAM, 0);
//	if (sock < 0)
//	{
//		perror("Failed to create socket");
//		exit(EXIT_FAILURE);
//	}


	// This struct is used to define one timeout in the used socket
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 00000; // 1 second
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) 
	{
		perror("Error");
	}

	// ret val from the binded socket is always -1
	ret = bind(sockfd,( struct sockaddr *) &cliaddr, sizeof(cliaddr));
	// std::cout << "return value from bind command = " <<  ret << std::endl;
//	if(ret < 0)
//	{
//		perror("return bind failed");
//		exit(EXIT_FAILURE);
//	}

	// Receive the datagram back from server
	int addrLength(sizeof(cliaddr)),received(0);
	char buffer[256] = {0};
	if((received=recvfrom(sockfd, buffer, 256, 0, (sockaddr *)&cliaddr, (socklen_t*)&addrLength)) < 0)
	{
		perror("No return message");
		exit(EXIT_FAILURE);
	}
	buffer[received]='\0';
	std::cout << "Server (" << inet_ntoa(cliaddr.sin_addr) << ") echoed: " << buffer << std::endl;
	close(sockfd);
		
	return 0;
}
