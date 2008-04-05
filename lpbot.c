#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

#define IRC_LINE_LENGHT 512

int lp_resolve(char *server, struct hostent *host)
{
	struct hostent *ptr;
	if (!(ptr = gethostbyname(server)))
	{
		perror("gethostbyname");
		return -1;
	}
	*host = *ptr;
	return 0;
}

int lp_create_sock()
{
	int sock;

	if((sock = socket(AF_INET, SOCK_STREAM, 0))<0)
		perror("socket");
	return sock;
}

int lp_send(int sock, char *msg)
{
	return write(sock, msg, strlen(msg));
}

int lp_recv(int sock)
{
	char buf[IRC_LINE_LENGHT+1];
	read(sock, &buf, IRC_LINE_LENGHT);
	printf("got from server: '%s'\n", buf);
}

int main()
{
	struct hostent host;
	int sock;
	struct sockaddr_in conn;

	lp_resolve("localhost", &host);
	sock = lp_create_sock();
	conn.sin_family = AF_INET;
	conn.sin_port = htons(6667);
	conn.sin_addr = *((struct in_addr *) host.h_addr);
	bzero(&(conn.sin_zero), 8);
	if(connect(sock, (struct sockaddr *)&conn, sizeof(struct sockaddr))<0)
	{
		perror("connect");
		return 1;
	}
	lp_send(sock, "pass lpbot\n");
	lp_send(sock, "nick lpbot\n");
	lp_send(sock, "user lpbot 8 * :lpbot\n");
	lp_send(sock, "join :#lpbot\n");
	while(1)
		lp_recv(sock);
}
