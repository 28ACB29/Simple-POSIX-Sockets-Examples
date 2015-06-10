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
const int LISTENQ = 1024;

void server(int sockfd)
{
	int read_status;
	char buf[MAX_LINE];
	int write_status;

	/*Echo client message*/
	while((read_status = read(sockfd, buf, MAX_LINE)) > 0)
	{

		/*Print received message*/
		printf("read message is: %s\n", buf);

		write_status = write(sockfd, buf, read_status);
		if(write_status != read_status)
		{
			printf("write error\n");
			return(errno);
		}
	}
	if(read_status != strlen(buf))
	{
		printf("read error\n");
		return(errno);
	}
	else if(read_status == 0)
	{
		return;
	}

	/*Clear buffer*/
	bzero(buf, sizeof(buf));
}

void server2(int sockfd, SA* pcliaddr, socklen_t len)
{
	int recvfrom_status;
	char buf[MAX_LINE];
	int sendto_status;

	/*Echo client message*/
	while((recvfrom_status = recvfrom(sockfd, buf, MAX_LINE, 0, pcliaddr, &len)) > 0)
	{

		/*Print received message*/
		printf("recvfrom message is: %s\n", buf);

		sendto_status = sendto(sockfd, buf, recvfrom_status, 0, pcliaddr, len);
		if(sendto_status != recvfrom_status)
		{
			printf("sendto error\n");
			return(errno);
		}

		/*Clear buffer*/
		bzero(buf, sizeof(buf));
	}
	if(recvfrom_status != strlen(buf))
	{
		printf("recvfrom error\n");
		return(errno);
	}
	else if(recvfrom_status == 0)
	{
		return;
	}
}

void server3(int sockfd)
{
	struct iovec serv_iov[1];
	struct msghdr serv_msghdr;
	int serv_flags;
	int recvmsg_status;
	char mesg[MAX_LINE];
	int sendmsg_status;

	/*Echo client message*/
	for ( ; ; )
	{

		serv_iov[0].iov_base = mesg;
		serv_iov[0].iov_len = sizeof(mesg);

		bzero(&serv_msghdr, sizeof(serv_msghdr));
		serv_msghdr.msg_name = NULL;
		serv_msghdr.msg_namelen = 0;
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
		else if(recvmsg_status == 0)
		{
			return;
		}

		/*Print received message*/
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
	int listenfd;
	struct sockaddr_in servaddr;
	int bind_status;
	int listen_status;
	struct sockaddr_in cliaddr;
	socklen_t clilen;
	int connfd;
	pid_t childpid;
	char buffer[MAX_LINE + 1];
	time_t current_time;
	int write_status;
	int read_status;
	int close_status;

	/*Check for correct argument length*/
	if (argc != 1)
	{
		printf("usage: tcpserver\n");
		return(errno);
	}

	/*Define socket file descriptor*/
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd < 0)
	{
		printf("socket error\n");
		return(errno);
	}

	/*Initialize server socket*/
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	/*Bind address to socket*/
	bind_status = bind(listenfd, (SA*) &servaddr, sizeof(servaddr));
	if(bind_status < 0)
	{
		printf("bind error\n");
		return(errno);
	}

	/*Listen for clients*/
	listen_status = listen(listenfd, LISTENQ);
	if(listen_status < 0)
	{
		printf("listen error\n");
		return(errno);
	}
	
	for( ; ; )
	{

		/*Accept a client connection*/
		connfd = accept(listenfd, (SA*) &cliaddr, &clilen);
		
		childpid = fork();
		if(childpid < 0)
		{
			printf("fork error\n");
			return(errno);
		}
		
		if(childpid == 0)
		{

			/*Close listening connection*/
			close_status = close(listenfd);
			if(close_status < 0)
			{
				printf("close error\n");
			}

			/* process the request */
			/*server(connfd);*/
			/*server2(connfd, (SA*) &cliaddr, clilen);*/
			server3(connfd);
			exit(0);
		}

		/*Close client connection*/
		close_status = close(connfd);
		if(close_status < 0)
		{
			printf("close error\n");
		}

		/*Clear buffer*/
		bzero(buffer, sizeof(buffer));
	}
	
	return(0);
}
