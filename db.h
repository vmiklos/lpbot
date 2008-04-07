typedef struct __lp_record_ver
{
	int date;
	char *author;
	GString *content;
} lp_record_ver;

typedef struct __lp_record
{
	char *name;
	GList *versions;
} lp_record;

int parseRecords(char *docname);
int saveRecords(char *docname);
