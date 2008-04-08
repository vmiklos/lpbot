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
	GList *rsslist;
	int ident_method;
	char* ident_to;
	int rss_interval;
} lp_config;

int parseConfig(char *docname);
void lp_config_free(lp_config *cfg);
