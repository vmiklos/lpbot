#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <pwd.h>
#include <glib.h>

#include "lpbot.h"

#define LP_HOST "localhost"
#define LP_PORT 1100

lp_server *server, *kbd;

/** @defgroup lptbotctl The bot controller
 * @{
 */

/** Handles the incoming messages from the server
 * @param source not used
 * @param condition not used
 * @param data not used
 * @return TRUE on success, FALSE on error
 */
int srv_handler(GIOChannel *source, GIOCondition condition, gpointer data)
{
	char c = 0, buf[IRC_LINE_LENGHT+1] = "", *ptr;
	int i = 0, len;

	while(c != '\n' && i < IRC_LINE_LENGHT)
	{
		len = read(server->sock, &c, 1);
		if(len == 0 || ( len < 0 && !sockerr_again() ))
		{
			if(errno)
				perror("read");
			exit(0);
		}
		buf[i++] = c;
	}
	if(buf[i-2]=='\r')
		i--;
	buf[i-1] = '\0';
	ptr = strchr(buf, ':');
	if(ptr)
		ptr++;
	else
		ptr = buf;
	printf("%s\n", ptr);
	return TRUE;
}

/** Handles the input from the standard input
 * @param source not used
 * @param condition not used
 * @param data not used
 * @return TRUE on success, FALSE on error
 */
int kbd_handler(GIOChannel *source, GIOCondition condition, gpointer data)
{
	char c = 0, buf[IRC_LINE_LENGHT+1] = "", *user = (char*)data;
	int i = 0, len;

	while(c != '\n' && i < IRC_LINE_LENGHT)
	{
		len = read(kbd->sock, &c, 1);
		if(len == 0 || ( len < 0 && !sockerr_again() ))
		{
			perror("read");
			return FALSE;
		}
		buf[i++] = c;
	}
	if(buf[i-2]=='\r')
		i--;
	buf[i-1] = '\0';
	lp_send(server, "%s PRIVMSG lpbot %s", user, buf);
	return TRUE;
}

/** The main function of the control client.
 * @param argv number of parameters
 * @param argc the parameters
 * @return 0 on success, -1 on error
 */
int main(int argv, char **argc)
{
	GMainLoop *loop;
	struct passwd *p;
	char *user;
	server = g_new0(lp_server, 1);
	kbd = g_new0(lp_server, 1);

	// option parsing
	if(argv<2)
	{
		p = getpwuid(getuid());
		user = g_strdup(p->pw_name);
	}
	else
		user = g_strdup(argc[1]);
	if(argv<3)
		server->address = g_strdup(LP_HOST);
	else
		server->address = g_strdup(argc[2]);
	if(argv<4)
		server->port = LP_PORT;
	else
		server->port = atoi(argc[3]);

	kbd->sock = STDIN_FILENO;
	kbd->chan = g_io_channel_unix_new(kbd->sock);

	printf("connecting to %s:%d...", server->address, server->port);
	if(lp_connect(server)==-1)
	{
		printf("failed!\n");
		return 1;
	}
	else
		printf("done!\n");
	g_io_add_watch(server->chan, G_IO_IN, srv_handler, NULL);
	g_io_add_watch(kbd->chan, G_IO_IN, kbd_handler, user);

	loop = g_main_loop_new(NULL, TRUE);
	g_main_loop_run(loop);
	return 0;
}
/* @} */
