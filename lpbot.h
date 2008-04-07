#include <glib.h>
#include <libxml/parser.h>
#include <netdb.h>
#include "servers.h"
#include "users.h"
#include "config.h"
#include "db.h"
#include "getdate.h"
#include "rss.h"
#include "remind.h"

typedef struct __lp_msg
{
	char *raw;
	char *from;
	char *to;
	char *cmd;
	GList *params;
} lp_msg;

int lp_disconnect(lp_server *server, char *msg);
int lp_reconnect(lp_server *server, char *msg);
int lp_connect(lp_server *server);
int lp_send(lp_server* server, char *fmt, ...);
int lp_resolve(char *server, struct hostent *host);
int lp_create_sock();
lp_user *lp_find_user(char *who);

#define IRC_LINE_LENGHT 512
#define sockerr_again() (errno == EINPROGRESS || errno == EINTR)

extern lp_config *config;
