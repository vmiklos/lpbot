#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <glib.h>

#include "lpbot.h"

static int parseUser(xmlDoc *doc, xmlNode *cur)
{
	xmlChar *key;
	cur = cur->xmlChildrenNode;
	lp_user *user = g_new0(lp_user, 1);


	while (cur)
	{
		key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"login")))
			user->login = strdup((char*)key);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pass")))
			user->pass = strdup((char*)key);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mask")))
			user->mask = strdup((char*)key);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"right_db")))
		{
			if(atoi((char*)key))
				user->rights |= LP_RIGHT_DB;
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"right_op")))
		{
			if(atoi((char*)key))
				user->rights |= LP_RIGHT_OP;
		}
		xmlFree(key);
		cur = cur->next;
	}
	config->users = g_list_append(config->users, user);
	return(0);
}

int parseUsers(char *docname)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlChar *key;

	doc = xmlParseFile(docname);

	if(doc == NULL)
	{
		fprintf(stderr, "document not parsed successfully\n");
		return(1);
	}

	cur = xmlDocGetRootElement(doc);

	if(cur == NULL)
	{
		fprintf(stderr, "empty document\n");
		xmlFreeDoc(doc);
		return(1);
	}

	if(xmlStrcmp(cur->name, (const xmlChar *)"users"))
	{
		fprintf(stderr, "document of the wrong type, root node != users");
		xmlFreeDoc(doc);
		return(1);
	}

	cur = cur->xmlChildrenNode;
	while(cur)
	{
		key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if((!xmlStrcmp(cur->name, (const xmlChar *)"user")))
			if(parseUser(doc, cur)<0)
				return -1;
		xmlFree(key);
		cur = cur->next;
	}
	xmlFreeDoc(doc);
	return(0);
}
