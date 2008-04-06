#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <glib-object.h>

#define IRC_LINE_LENGHT 512

int server_sock;

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
	char buf[IRC_LINE_LENGHT+1];
	g_snprintf(buf, IRC_LINE_LENGHT, "%s\n", msg);
	return write(sock, buf, strlen(buf));
}

int lp_handler(GIOChannel *source, GIOCondition condition, gpointer data)
{
	char c = 0, buf[IRC_LINE_LENGHT+1] = "";
	int i = 0, len;
	int sock;

	sock = (int)data;

	while(c != '\n' && i < IRC_LINE_LENGHT)
	{
		if((len = read(sock, &c, 1))<=0)
		{
			if(len<0)
				perror("read");
			return FALSE;
		}
		buf[i++] = c;
	}
	if(buf[i-2]=='\r')
		i--;
	buf[i-1] = '\0';
	if(sock != STDIN_FILENO)
		printf("%s\n", buf);
	else
		lp_send(server_sock, buf);
	fflush(stdout);
	return TRUE;
}

int main()
{
	struct hostent host;
	int sock;
	struct sockaddr_in conn;

	GIOChannel *sock_chan, *kbd_chan;
	GMainLoop *loop;

	lp_resolve("localhost", &host);
	sock = lp_create_sock();
	server_sock = sock;
	conn.sin_family = AF_INET;
	conn.sin_port = htons(6667);
	conn.sin_addr = *((struct in_addr *) host.h_addr);
	bzero(&(conn.sin_zero), 8);
	if(connect(sock, (struct sockaddr *)&conn, sizeof(struct sockaddr))<0)
	{
		perror("connect");
		return 1;
	}
	sock_chan = g_io_channel_unix_new(sock);
	g_io_add_watch(sock_chan, G_IO_IN, lp_handler, (gpointer)sock);
	kbd_chan = g_io_channel_unix_new(STDIN_FILENO);
	g_io_add_watch(kbd_chan, G_IO_IN, lp_handler, (gpointer)STDIN_FILENO);
	lp_send(sock, "nick lpbot\n");
	lp_send(sock, "user lpbot 8 * :lpbot\n");

	loop = g_main_loop_new(NULL, TRUE);
	g_main_loop_run(loop);
	return 0;
}
