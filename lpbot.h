typedef struct __lp_server
{
	char *chatname;
	char *address;
	int port;
	char *nick;
	char *username;
	char *realname;
	GList *channels;

	GIOChannel *chan;
	int sock;

	int lastpong;
} lp_server;

typedef struct __lp_msg
{
	char *raw;
	char *from;
	char *to;
	char *cmd;
	GList *params;
} lp_msg;

extern GList *servers, *users;
int lp_disconnect(lp_server *server, char *msg);
int lp_reconnect(lp_server *server, char *msg);

#define IRC_LINE_LENGHT 512
#define sockerr_again() (errno == EINPROGRESS || errno == EINTR)

#include "config.h"
#include "users.h"
