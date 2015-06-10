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

void client(FILE *fp, int sockfd)
{
	int fgets_status;
	int write_status;
	int	read_status;
	char sendline[MAX_LINE];
	char recvline[MAX_LINE + 1];
	int fputs_status;

	/*While there is something to read*/
	while((fgets_status = fgets(sendline, MAX_LINE, fp)) != NULL)
	{

		/*Send message*/
		write_status = write(sockfd, sendline, strlen(sendline));
		if(write_status != strlen(sendline))
		{
			printf("write error\n");
		}

		/*Read message*/
		read_status = read(sockfd, recvline, MAX_LINE);
		if(read_status != write_status)
		{
			printf("read error\n");
		}

		/*Print received message if successful*/
		else
		{

			/* null terminate */
			recvline[read_status] = 0;
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

void client2(FILE* fp, int sockfd, struct sockaddr_in servaddr, int servlen)
{
	int fgets_status;
	int sendto_status;
	char sendline[MAX_LINE];
	int recvfrom_status;
	char recvline[MAX_LINE];
	int fputs_status;

	/*While there is something to read*/
	while((fgets_status = fgets(sendline, MAX_LINE, fp)) != NULL)
	{

		/*Send message*/
		sendto_status = sendto(sockfd, sendline, strlen(sendline), 0, (SA*) &servaddr, servlen);
		if(sendto_status != strlen(sendline))
		{
			printf("sendto error\n");
			return(errno);
		}

		/*Read message*/
		recvfrom_status = recvfrom(sockfd, recvline, MAX_LINE, 0, (SA*) NULL, NULL);
		if(recvfrom_status != sendto_status)
		{
			printf("recvfrom error\n");
			if(recvfrom_status == 0)
			{
				printf("client: server terminated prematurely\n");
			}
			return(errno);
		}

		/*Print received message if successful*/
		fputs_status = fputs(recvline, stdout);
		if(fputs_status == EOF)
		{
			printf("fputs error\n");
			return(errno);
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

void client3(FILE* fp, int sockfd)
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
		send_msghdr.msg_name = NULL;
		send_msghdr.msg_namelen = 0;
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
		recv_msghdr.msg_name = NULL;
		recv_msghdr.msg_namelen = 0;
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
	int sockfd;
	struct sockaddr_in servaddr;
	int inet_pton_status;
	int connect_status;
	char sendline[MAX_LINE + 1];
	int write_status;
	char recvline[MAX_LINE + 1];
	int read_status;
	int close_status;

	/*Check for correct argument length*/
	if (argc != 2)
	{
		printf("usage: tcpclient <IPaddress>\n");
		return(errno);
	}

	/*Define socket file descriptor*/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		printf("socket error");
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

	/*Establish connection with server*/
	connect_status = connect(sockfd, (SA*) &servaddr, sizeof(servaddr));
	if(connect_status < 0)
	{
		printf("connect error\n");
		return(errno);
	}

	/* do it all */
	/*client(stdin, sockfd);*/
	/*client2(stdin, sockfd, servaddr, sizeof(servaddr));*/
	client3(stdin, sockfd);

	/*Close socket*/
	close_status = close(sockfd);
	if(close_status < 0)
	{
		printf("close error\n");
	}
	return(0);
}
