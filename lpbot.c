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

lp_config *config;

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
		{
			*p = '\0';
			p++;
		}
		while(p)
		{
			*p = '\0';
			msg->params = g_list_append(msg->params, ++p);
			p = strchr(p, ' ');
		}
	}
	p = strchr(msg->from, '!');
	if(p)
		*p = '\0';
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

int lp_check_rss(gpointer data)
{
	int i;

	for(i=0;i<g_list_length(config->rsslist);i++)
	{
		lp_rss *rss = g_list_nth_data(config->rsslist, i);
		check_rss(rss);
	}
	return TRUE;
}

char *lp_to(lp_server *server, lp_msg *msg)
{
	// if the msg is from a channel, then returns the channel, if
	// it's from a query, then return the user
	char *ret;

	if(!strcmp(server->nick, msg->to))
		ret = msg->from;
	else
		ret = msg->to;
	return ret;
}

int lp_identified(char *who)
{
	int i;

	for(i=0;i<g_list_length(config->users);i++)
	{
		lp_user *user = g_list_nth_data(config->users, i);
		if(!strcmp(user->login, who) && user->identified)
			return 1;
	}
	return 0;
}

lp_user *lp_find_user(char *who)
{
	int i;

	for(i=0;i<g_list_length(config->users);i++)
	{
		lp_user *user = g_list_nth_data(config->users, i);
		if(!strcmp(user->login, who) && user->identified)
			return user;
	}
	return NULL;
}

int lp_handle_command(lp_server *server, lp_msg *msg, GList *params)
{
	int i;
	char *to = lp_to(server, msg);

	// FIXME: this could be more generic to provide
	// help, etc
	if(!strcmp("ping", g_list_nth_data(params, 0)))
		lp_send(server, "privmsg %s :pong", to);
	if(!strcmp("eval", g_list_nth_data(params, 0)))
	{
		if(lp_identified(msg->from))
		{
			lp_user *user = lp_find_user(msg->from);
			if(user->rights & LP_RIGHT_OP && g_list_length(params) >1)
			{
				GString *cmd = g_string_new(g_list_nth_data(params, 1));
				for(i=2;i<g_list_length(params);i++)
					g_string_append_printf(cmd, " %s", (char*)g_list_nth_data(params, i));
				lp_send(server, cmd->str);
			}
			else
				lp_send(server, "privmsg %s :you don't have rights to alter the db", to);
		}
		else
			lp_send(server, "privmsg %s :identify first to control the bot", to);
	}
	if(!strcmp("db", g_list_nth_data(params, 0)))
	{
		int found = 0;
		if(!strcmp("get", g_list_nth_data(params, 1)) && g_list_length(params) == 3)
		{
			for(i=0;i<g_list_length(config->records);i++)
			{
				lp_record *record = g_list_nth_data(config->records, i);
				if(!strcmp(record->name, g_list_nth_data(params, 2)))
				{
					lp_record_ver *ver = g_list_nth_data(record->versions, 0);
					lp_send(server, "privmsg %s :%s", to, ver->content->str);
					found = 1;
					break;
				}
			}
			if(!found)
				lp_send(server, "privmsg %s :no such record", to);
		}
		else if(!strcmp("put", g_list_nth_data(params, 1)) && g_list_length(params) > 3)
		{
			if(lp_identified(msg->from))
			{
				lp_user *user = lp_find_user(msg->from);
				if(user->rights & LP_RIGHT_DB)
				{
					lp_record *record;
					for(i=0;i<g_list_length(config->records);i++)
					{
						record = g_list_nth_data(config->records, i);
						if(!strcmp(record->name, g_list_nth_data(params, 2)))
						{
							found = 1;
							break;
						}
					}
					if(!found)
					{
						record = g_new0(lp_record, 1);
						record->name = g_strdup(g_list_nth_data(params, 2));
						config->records = g_list_append(config->records, record);
					}
					lp_record_ver *ver = g_new0(lp_record_ver, 1);
					ver->date = time(NULL);
					ver->author = g_strdup(msg->from);
					ver->content = g_string_new(g_strdup(g_list_nth_data(params, 3)));
					for(i=4;i<g_list_length(params);i++)
						g_string_append_printf(ver->content, " %s",
								(char*)g_list_nth_data(params, i));
					record->versions = g_list_insert(record->versions, ver, 0);
					saveRecords("db.xml");
					if(found)
						lp_send(server, "privmsg %s :okay, updated", to);
					else
						lp_send(server, "privmsg %s :okay, inserted", to);
				}
				else
					lp_send(server, "privmsg %s :you don't have rights to alter the db", to);
			}
			else
				lp_send(server, "privmsg %s :identify first to alter the db", to);
		}
		else if(!strcmp("del", g_list_nth_data(params, 1)) && g_list_length(params) == 3)
		{
			if(lp_identified(msg->from))
			{
				lp_user *user = lp_find_user(msg->from);
				if(user->rights & LP_RIGHT_DB)
				{
					lp_record *record;
					for(i=0;i<g_list_length(config->records);i++)
					{
						record = g_list_nth_data(config->records, i);
						if(!strcmp(record->name, g_list_nth_data(params, 2)))
						{
							found = 1;
							break;
						}
					}
					if(!found)
						lp_send(server, "privmsg %s :no such record", to);
					else
					{
						config->records = g_list_remove(config->records, record);
						saveRecords("db.xml");
						lp_send(server, "privmsg %s :okay, deleted", to);
					}
				}
				else
					lp_send(server, "privmsg %s :you don't have rights to alter the db", to);
			}
			else
				lp_send(server, "privmsg %s :identify first to alter the db", to);
		}
	}
	if(!strcmp("whoami", g_list_nth_data(params, 0)))
	{
		if(lp_identified(msg->from))
			lp_send(server, "privmsg %s :i know who you are, %s.",
					to, msg->from);
	}
	if(!strcmp("identify", g_list_nth_data(params, 0)))
	{
		if(config->ident_method == LP_IDENT_PASS)
		{
			for(i=0;i<g_list_length(config->users);i++)
			{
				lp_user *user = g_list_nth_data(config->users, i);
				if(!strcmp(user->login, msg->from) &&
						g_list_length(params) == 2 &&
						!strcmp(user->pass, g_list_nth_data(params, 1)))
				{
					user->identified = 1;
					lp_send(server, "privmsg %s :ok, now i know you, %s.",
							to, user->login);
					break;
				}
			}
		}
		if(config->ident_method == LP_IDENT_SERVICES)
		{
			if(config->ident_to)
				free(config->ident_to);
			config->ident_to = g_strdup(to);
			lp_send(server, "whois %s", msg->from);
		}
	}
	if(!strcmp("quit", g_list_nth_data(params, 0)) && server->is_console)
	{
		lp_user *user = lp_find_user(msg->from);
		if(user)
			user->identified = 0;
		lp_disconnect(server, "bye");
	}
	if(!strcmp("connect", g_list_nth_data(params, 0)))
	{
		lp_user *user = lp_find_user(msg->from);
		if(user)
		{
			if(user->rights & LP_RIGHT_OP && g_list_length(params) >5)
			{
				lp_server *new = g_new0(lp_server, 1);
				new->chatname = g_strdup(g_list_nth_data(params, 1));
				new->address = g_strdup(g_list_nth_data(params, 2));
				new->port = atoi(g_list_nth_data(params, 3));
				new->nick = g_strdup(g_list_nth_data(params, 4));
				new->username = g_strdup(g_list_nth_data(params, 4));
				new->realname = g_strdup(g_list_nth_data(params, 4));
				for(i=5;i<g_list_length(params);i++)
					new->channels = g_list_append(new->channels, g_strdup(g_list_nth_data(params, i)));
				config->servers = g_list_append(config->servers, new);
				lp_connect(new);
				lp_send(server, "privmsg %s :ok, connected", to);
			}
			else
				lp_send(server, "privmsg %s :you don't have rights to alter the db", to);
		}
		else
			lp_send(server, "privmsg %s :identify first to alter the db", to);
	}
	return 0;
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
	else if(!strcmp(msg->cmd, "320"))
	{
		// identified
		if(config->ident_method == LP_IDENT_SERVICES)
		{
			for(i=0;i<g_list_length(config->users);i++)
			{
				lp_user *user = g_list_nth_data(config->users, i);
				if(g_list_length(msg->params) > 0 &&
						!strcmp(user->login, g_list_nth_data(msg->params, 0)))
				{
					user->identified = 1;
					lp_send(server, "privmsg %s :ok, now i know you, %s.",
							config->ident_to, user->login);
					break;
				}
			}
		}
	}
	else if(!strcmp(msg->cmd, "PONG"))
	{
		server->lastpong = time(NULL);
	}
	else if(!strcmp(msg->cmd, "PRIVMSG"))
	{
		if(!strncmp(server->nick, g_list_nth_data(msg->params, 0), strlen(server->nick)))
			// highlight
			lp_handle_command(server, msg, g_list_next(msg->params));
		else if(!strncmp(server->nick, msg->to, strlen(server->nick)))
			// query
			lp_handle_command(server, msg, msg->params);
	}
	else if(!strcmp(msg->cmd, "QUIT") || !strcmp(msg->cmd, "PART"))
	{
		for(i=0;i<g_list_length(config->users);i++)
		{
			lp_user *user = g_list_nth_data(config->users, i);
			if(!strcmp(user->login, msg->from))
			{
				user->identified = 0;
				break;
			}
		}
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
	if(server->is_console)
		shutdown(server->sock, SHUT_RDWR);
	close(server->sock);
	server->sock = 0;
	return 0;
}

int lp_reconnect(lp_server *server, char *msg)
{
	lp_disconnect(server, msg);
	lp_connect(server);
	return 0;
}

int lp_listen(GIOChannel *source, GIOCondition condition, gpointer data)
{
	int sock = (int)data;
	lp_server *server = g_new0(lp_server, 1);
	server->sock = accept(sock, NULL, NULL);
	server->chan = g_io_channel_unix_new(server->sock);
	server->nick = g_strdup("lpbot");
	server->is_console = 1;
	g_io_add_watch(server->chan, G_IO_IN, lp_handler, (gpointer)server);
	return TRUE;
}

int lp_serve()
{
	int sock = lp_create_sock();
	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	struct sockaddr_in conn;
	memset(&conn, 0, sizeof(conn));
	conn.sin_family = AF_INET;
	conn.sin_port = htons(1100);
	conn.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(sock,(struct sockaddr*) &conn, sizeof(conn));
	listen(sock, 1);
	GIOChannel *chan = g_io_channel_unix_new(sock);
	g_io_add_watch(chan, G_IO_IN, lp_listen, (gpointer)sock);
	return 0;
}

int main()
{
	int i;
	GMainLoop *loop;
	config = g_new0(lp_config, 1);

	parseConfig("config.xml");
	parseRecords("db.xml");
	lp_serve();
	g_timeout_add(config->rss_interval*1000, lp_check_rss, NULL);

	for(i=0;i<g_list_length(config->servers);i++)
		lp_connect(g_list_nth_data(config->servers, i));

	loop = g_main_loop_new(NULL, TRUE);
	g_main_loop_run(loop);
	return 0;
}
