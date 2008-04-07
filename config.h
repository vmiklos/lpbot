enum
{
	LP_IDENT_PASS = 1,
	LP_IDENT_SERVICES
};

typedef struct __lp_config
{
	GList *servers;
	GList *users;
	GList *records;
	int ident_method;
	char* ident_to;
} lp_config;

int parseConfig(char *docname);
