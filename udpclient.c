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

void client(FILE* fp, int sockfd, const SA* pservaddr, socklen_t servlen)
{
	int fgets_status;
	int sendto_status;
	int	recvfrom_status;
	char sendline[MAX_LINE];
	char recvline[MAX_LINE + 1];
	int fputs_status;

	while((fgets_status = fgets(sendline, MAX_LINE, fp)) != NULL)
	{

		/*Send message*/
		sendto_status = sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
		if(sendto_status != strlen(sendline))
		{
			printf("sendto error\n");
			return(errno);
		}

		/*Read message*/
		recvfrom_status = recvfrom(sockfd, recvline, MAX_LINE, 0, NULL, NULL);
		if(recvfrom_status != sendto_status)
		{
			printf("recvfrom error\n");
			return(errno);
		}

		/*Print received message if successful*/
		else
		{

			/* null terminate */
			recvline[recvfrom_status] = 0;
			fputs_status = fputs(recvline, stdout);
			if(fputs_status == EOF)
			{
				printf("fputs error\n");
				return(errno);
			}
		}

		/*Clear buffers*/
		bzero(sendline, sizeof(sendline));
		bzero(recvline, sizeof(recvline));
	}
	if(fgets_status == NULL && ferror(fp))
	{
		printf("fgets error");
		return(errno);
	}
}

void client2(FILE* fp, int sockfd, const SA* pservaddr, socklen_t servlen)
{
	int fgets_status;
	struct iovec send_iov[1];
	struct msghdr send_msghdr;
	int send_flags;
	int sendmsg_status;
	struct iovec recv_iov[1];
	struct msghdr recv_msghdr;
	int recv_flags;
	int	recvmsg_status;
	char sendline[MAX_LINE];
	char recvline[MAX_LINE + 1];
	int fputs_status;

	while((fgets_status = fgets(sendline, MAX_LINE, fp)) != NULL)
	{

		send_iov[0].iov_base = sendline;
		send_iov[0].iov_len = strlen(sendline);

		bzero(&send_msghdr, sizeof(send_msghdr));
		send_msghdr.msg_name = pservaddr;
		send_msghdr.msg_namelen = servlen;
		send_msghdr.msg_iov = send_iov;
		send_msghdr.msg_iovlen = 1;
		send_msghdr.msg_flags = 0;

		send_flags = 0;

		/*Send message*/
		sendmsg_status = sendmsg(sockfd, &send_msghdr, send_flags);
		if(sendmsg_status != strlen(sendline))
		{
			printf("sendmsg error\n");
			return(errno);
		}

		recv_iov[0].iov_base = recvline;
		recv_iov[0].iov_len = MAX_LINE;

		bzero(&recv_msghdr, sizeof(recv_msghdr));
		recv_msghdr.msg_name = pservaddr;
		recv_msghdr.msg_namelen = servlen;
		recv_msghdr.msg_iov = recv_iov;
		recv_msghdr.msg_iovlen = 1;
		recv_msghdr.msg_flags = 0;

		recv_flags = 0;

		/*Read message*/
		recvmsg_status = recvmsg(sockfd, &recv_msghdr, recv_flags);
		if(recvmsg_status != sendmsg_status)
		{
			printf("recvmsg error\n");
			return(errno);
		}

		/*Print received message if successful*/
		else
		{

			/* null terminate */
			recvline[recvmsg_status] = 0;
			fputs_status = fputs(recvline, stdout);
			if(fputs_status == EOF)
			{
				printf("fputs error\n");
				return(errno);
			}
		}

		/*Clear buffers*/
		bzero(sendline, sizeof(sendline));
		bzero(recvline, sizeof(recvline));
	}
	if(fgets_status == NULL && ferror(fp))
	{
		printf("fgets error");
		return(errno);
	}
}

int main(int argc, char **argv)
{
	struct sockaddr_in servaddr;
	int inet_pton_status;
	int sockfd;

	/*Check for correct argument length*/
	if(argc != 2)
	{
		printf("usage: client <IPaddress>");
		return(errno);
	}

	/*Initialize server socket*/
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);

	/*Convert input to address*/
	inet_pton_status = inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	if(inet_pton_status <= 0)
	{
		printf("inet_pton error for %s\n", argv[1]);
		return(errno);
	}

	/*Define socket file descriptor*/
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0)
	{
		printf("socket error");
		return(errno);
	}

	/* do it all */
	/*client(stdin, sockfd, (SA*) &servaddr, sizeof(servaddr));*/
	client2(stdin, sockfd, (SA*) &servaddr, sizeof(servaddr));

	exit(0);
}
