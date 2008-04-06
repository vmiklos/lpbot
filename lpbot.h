#include <libxml/parser.h>
#include "servers.h"
#include "users.h"
#include "config.h"

typedef struct __lp_msg
{
	char *raw;
	char *from;
	char *to;
	char *cmd;
	GList *params;
} lp_msg;

typedef struct __lp_config
{
	GList *servers;
	GList *users;
} lp_config;

int lp_disconnect(lp_server *server, char *msg);
int lp_reconnect(lp_server *server, char *msg);

#define IRC_LINE_LENGHT 512
#define sockerr_again() (errno == EINPROGRESS || errno == EINTR)

extern lp_config *config;
