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
} lp_server;

extern GList *servers;
