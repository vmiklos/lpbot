#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include "lpbot.h"

/*
 * Sends a password reminder to a given user.
 * @param user the user pointer
 * @return 0 on success, -1 on error
 */
int remind(lp_user *user)
{
	struct hostent host;
	struct sockaddr_in conn;
	lp_server *server = g_new0(lp_server, 1);

	lp_resolve("localhost", &host);
	server->sock = lp_create_sock();
	conn.sin_family = AF_INET;
	conn.sin_port = htons(25);
	conn.sin_addr = *((struct in_addr *) host.h_addr);
	memset(&(conn.sin_zero), 0, 8);
	if(connect(server->sock, (struct sockaddr *)&conn, sizeof(struct sockaddr))<0)
	{
		perror("connect");
		return -1;
	}
	lp_send(server, "HELO localhost");
	lp_send(server, "MAIL FROM: lpbot@vmiklos.hu");
	lp_send(server, "RCPT TO: %s", user->email);
	lp_send(server, "DATA");
	lp_send(server, "From: lpbot@vmiklos.hu");
	lp_send(server, "To: %s", user->email);
	lp_send(server, "Subject: password reminder from lpbot");
	lp_send(server, "");
	lp_send(server, "Your password is '%s'.", user->pass);
	lp_send(server, ".");
	lp_send(server, "QUIT");
	shutdown(server->sock, SHUT_RDWR);
	close(server->sock);
	return 0;
}
