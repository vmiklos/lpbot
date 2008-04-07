#include <stdio.h>
#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <glib.h>

#include "lpbot.h"

int parseUser(xmlDoc *doc, xmlNode *cur)
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
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"email")))
			user->email = strdup((char*)key);
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
