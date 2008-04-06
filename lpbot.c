#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <errno.h>
#include <glib-object.h>

#include "lpbot.h"
#include "config.h"

GList *servers, *users;

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

int lp_send(lp_server* server, char *fmt, ...)
{
	va_list ap;
	char buf[IRC_LINE_LENGHT+1];
	struct pollfd pfd[1];

	if(!server)
		server = g_list_nth_data(servers, 0);
	if(!server || server->sock<0)
		return -1;

	va_start(ap, fmt);
	vsnprintf(buf, IRC_LINE_LENGHT-1, fmt, ap);
	va_end(ap);
	printf("sending message '%s'\n", buf);
	buf[strlen(buf)+1] = '\0';
	buf[strlen(buf)] = '\n';

	// check if we can write to avoid a sigpipe
	pfd[0].fd = server->sock;
	pfd[0].events = POLLOUT;

	poll(pfd, 1, 1000);
	if(pfd[0].revents & POLLHUP)
	{
		lp_reconnect(server, NULL);
		return FALSE;
	}
	return write(server->sock, buf, strlen(buf));
}

void lp_dump_msg(lp_msg *msg)
{
	int i;

	printf("lp_dump_msg() from: '%s', cmd: '%s', to: '%s'", msg->from, msg->cmd, msg->to);
	if(g_list_length(msg->params))
		printf(", params: ");
	for(i=0;i<g_list_length(msg->params);i++)
	{
		printf("'%s' ", (char*)g_list_nth_data(msg->params, i));
	}
	printf("\n");
}

lp_msg *lp_parse(char *str)
{
	char *p;
	lp_msg *msg = g_new0(lp_msg, 1);
	msg->raw = g_strdup(str);
	msg->from = msg->raw;
	if(msg->from[0] == ':')
		msg->from++;

	printf("parsing message '%s'\n", msg->raw);

	p = strchr(msg->raw, ' ');
	// every irc msg should have a from/to/cmd
	if(!p)
		return NULL;
	*p = '\0';
	msg->cmd = ++p;
	p = strchr(msg->cmd, ' ');
	if(!p)
		return NULL;
	*p = '\0';
	msg->to = ++p;
	p = strchr(msg->to, ' ');
	// params are optional
	if(p)
	{
		if(*(p+1)==':')
			p++;
		while(p)
		{
			*p = '\0';
			msg->params = g_list_append(msg->params, ++p);
			p = strchr(p, ' ');
		}
	}
	lp_dump_msg(msg);
	return msg;
}

void lp_msg_free(lp_msg *msg)
{
	free(msg->raw);
	free(msg);
}

int lp_ping(gpointer data)
{
	lp_server *server = (lp_server*)data;
	int lag = time(NULL) - server->lastpong;

	if(server->lastpong != 0 && lag > 300)
	{
		lp_reconnect(server, "Connection timed out");
		return FALSE;
	}
	lp_send(server, "PING %s", server->nick);
	return TRUE;
}

int lp_handler(GIOChannel *source, GIOCondition condition, gpointer data)
{
	char c = 0, buf[IRC_LINE_LENGHT+1] = "";
	int i = 0, len;
	lp_server *server = (lp_server*)data;
	lp_msg *msg;

	while(c != '\n' && i < IRC_LINE_LENGHT)
	{
		len = read(server->sock, &c, 1);
		if(len == 0 || ( len < 0 && !sockerr_again() ))
		{
			lp_reconnect(server, "Read error");
			return FALSE;
		}
		buf[i++] = c;
	}
	if(buf[i-2]=='\r')
		i--;
	buf[i-1] = '\0';
	if(server->sock == STDIN_FILENO)
	{
		lp_send(NULL, buf);
		return TRUE;
	}
	msg = lp_parse(buf);
	// probably a ping which is handled by the idle handler or so
	if(!msg)
		return TRUE;
	if(!strcmp(msg->cmd, "001"))
	{
		// welcome
		for(i=0;i<g_list_length(server->channels); i++)
			lp_send(server, "join :%s", (char*)g_list_nth_data(server->channels, i));
		g_timeout_add(10000, lp_ping, (gpointer)server);
	}
	else if(!strcmp(msg->cmd, "PONG"))
	{
		server->lastpong = time(NULL);
	}
	lp_msg_free(msg);
	return TRUE;
}

int lp_connect(lp_server *server)
{
	struct hostent host;
	struct sockaddr_in conn;

	lp_resolve(server->address, &host);
	server->sock = lp_create_sock();
	conn.sin_family = AF_INET;
	conn.sin_port = htons(server->port);
	conn.sin_addr = *((struct in_addr *) host.h_addr);
	memset(&(conn.sin_zero), 0, 8);
	if(connect(server->sock, (struct sockaddr *)&conn, sizeof(struct sockaddr))<0)
	{
		perror("connect");
		return -1;
	}
	server->chan = g_io_channel_unix_new(server->sock);
	g_io_add_watch(server->chan, G_IO_IN, lp_handler, (gpointer)server);
	lp_send(server, "nick %s", server->nick);
	lp_send(server, "user %s 8 * :%s", server->username, server->realname);
	return 0;
}

int lp_disconnect(lp_server *server, char *msg)
{
	if(msg)
		lp_send(server, "quit :%s", msg);
	close(server->sock);
	server->sock = 0;
	servers = g_list_remove(servers, server);
	return 0;
}

int lp_reconnect(lp_server *server, char *msg)
{
	lp_disconnect(server, msg);
	lp_connect(server);
	return 0;
}

int main()
{
	int i;
	GMainLoop *loop;

	parseConfig("config.xml");
	parseUsers("users.xml");

	for(i=0;i<g_list_length(servers);i++)
		lp_connect(g_list_nth_data(servers, i));
	// debug
	lp_server *kbd = g_new0(lp_server, 1);
	kbd->chan = g_io_channel_unix_new(STDIN_FILENO);
	kbd->sock = STDIN_FILENO;
	g_io_add_watch(kbd->chan, G_IO_IN, lp_handler, (gpointer)kbd);

	loop = g_main_loop_new(NULL, TRUE);
	g_main_loop_run(loop);
	return 0;
}
