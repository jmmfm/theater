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
    int s;
    int ret;
    char *buf;
    struct sockaddr_in addr;
 
    if (argc != 4)
	{
		puts("usage: sendUDP ipaddr port data");
		exit(EXIT_FAILURE);
	}
	
	addr.sin_family = AF_INET;
	
	// Assign remote address
	ret = inet_aton(argv[1], &addr.sin_addr);
	if (ret == 0)
	{
		perror("inet_aton");
		exit(EXIT_FAILURE);
	}

	// Assign remote port	
	addr.sin_port = htons(atoi(argv[2]));
	buf = argv[3];
	
	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s < 0)
	{
		perror("Failed to create socket");
		exit(EXIT_FAILURE);
	}
	
	ret = sendto(s, buf, strlen(buf), 0, (struct sockaddr *)&addr, sizeof(addr));
	if (ret == -1)
	{
		perror("sendto");
		exit(EXIT_FAILURE);
	}



	// Receive part ------------------------------------------------------------
	struct sockaddr_in myaddr;
	int sock;
	
	// Construct the server sockaddr_ structure	
//	memset (&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	// Bind to any addr on local machine
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
//	ret = inet_aton("10.0.2.15", &myaddr.sin_addr);
//	if (ret == 0)
//	{
//		perror("inet_aton");
//		exit(EXIT_FAILURE);
//	}
	// Bind to any port
	myaddr.sin_port=htons(0);

	// Create the socket
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		perror("Failed to create socket");
		exit(EXIT_FAILURE);
	}

	ret = bind(sock,( struct sockaddr *) &myaddr, sizeof(myaddr));
	if(ret < 0)
	{
		perror("return bind failed");
		exit(EXIT_FAILURE);
	}

	// Receive the datagram
	//Receive the datagram back from server
	int addrLength(sizeof(myaddr)),received(0);
	char buffer[256] = {0};
	if((received=recvfrom(sock, buffer, 256, 0, (sockaddr *)&myaddr, (socklen_t*)&addrLength)) < 0)
	{
		perror("Mismatch in number of bytes received");
		exit(EXIT_FAILURE);
	}
	buffer[received]='\0';
	std::cout << "Server (" << inet_ntoa(myaddr.sin_addr) << ") echoed: " << buffer << std::endl;
	close(sock);
		
	return 0;
}
