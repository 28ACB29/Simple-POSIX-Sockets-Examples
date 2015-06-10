#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

const int MAX_LINE = 4096;
const char character = 'X';
const int PORT = 16384;
typedef struct sockaddr SA;

void server(int sockfd, SA *pcliaddr, socklen_t clilen)
{
	int recvfrom_status;
	int number;
	int ioctl_status;
	socklen_t len;
	char mesg[MAX_LINE];
	int sendto_status;

	/*Echo client message*/
	for ( ; ; )
	{
		len = clilen;
		recvfrom_status = recvfrom(sockfd, mesg, MAX_LINE, 0, pcliaddr, &len);
		if(recvfrom_status != strlen(mesg))
		{
			printf("recvfrom error\n");
			return(errno);
		}

		/*Print received message*/
		printf("recvfrom message is: %s\n", mesg);

		sendto_status = sendto(sockfd, mesg, recvfrom_status, 0, pcliaddr, len);
		if(sendto_status != recvfrom_status)
		{
			printf("sendto error\n");
			return(errno);
		}

		/*Clear buffer*/
		bzero(mesg, sizeof(mesg));
	}
}

void server2(int sockfd, SA* pcliaddr, socklen_t clilen)
{
	struct iovec serv_iov[1];
	struct msghdr serv_msghdr;
	int serv_flags;
	int recvmsg_status;
	socklen_t len;
	char mesg[MAX_LINE];
	int sendmsg_status;

	/*Echo client message*/
	for ( ; ; )
	{
		len = clilen;

		serv_iov[0].iov_base = mesg;
		serv_iov[0].iov_len = sizeof(mesg);

		bzero(&serv_msghdr, sizeof(serv_msghdr));
		serv_msghdr.msg_name = pcliaddr;
		serv_msghdr.msg_namelen = clilen;
		serv_msghdr.msg_iov = serv_iov;
		serv_msghdr.msg_iovlen = 1;
		serv_msghdr.msg_flags = 0;

		serv_flags = 0;

		recvmsg_status = recvmsg(sockfd, &serv_msghdr, serv_flags);
		if(recvmsg_status != strlen(mesg))
		{
			printf("recvmsg error\n");
			return(errno);
		}

		printf("recvmsg message is: %s\n", mesg);

		serv_iov[0].iov_len = strlen(mesg);

		sendmsg_status = sendmsg(sockfd, &serv_msghdr, serv_flags);
		if(sendmsg_status != recvmsg_status)
		{
			printf("sendmsg error\n");
			return(errno);
		}

		/*Clear buffer*/
		bzero(mesg, sizeof(mesg));
	}
}

int main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_in servaddr;
	int bind_status;
	struct sockaddr_in cliaddr;

	/*Check for correct argument length*/
	if (argc != 1)
	{
		printf("usage: tcpserver\n");
		return(errno);
	}

	/*Define socket file descriptor*/
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0)
	{
		printf("socket error");
		return(errno);
	}

	/*Initialize server socket*/
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	/*Bind address to socket*/
	bind_status = bind(sockfd, (SA*) &servaddr, sizeof(servaddr));
	if(bind_status < 0)
	{
		printf("bind error\n");
		return(errno);
	}

	/* process the request */
	/*server(sockfd, (SA*) &cliaddr, sizeof(cliaddr));*/
	server2(sockfd, (SA*) &cliaddr, sizeof(cliaddr));
}
