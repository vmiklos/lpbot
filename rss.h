typedef struct __lp_rss
{
	char *name;
	char *url;
	lp_server* server;
	char *channel;
	int lastupd;
} lp_rss;

int check_rss(lp_rss *rss);
int parseRss(xmlDoc *doc, xmlNode *cur);
