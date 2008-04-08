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
	int is_console;
} lp_server;

int parseServer(xmlDoc *doc, xmlNode *cur);
lp_server *lp_find_server(char *chatname);
void lp_server_free(lp_server *srv);
