#define LP_RIGHT_DB 0x01
#define LP_RIGHT_OP 0x02

typedef struct __lp_user
{
	char *login;
	char *email;
	char *pass;
	unsigned int rights;
	int identified;
} lp_user;

int parseUser(xmlDoc *doc, xmlNode *cur);
