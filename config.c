#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <glib.h>

#include "lpbot.h"

static int parseServer(xmlDoc *doc, xmlNode *cur)
{
	xmlChar *key;
	cur = cur->xmlChildrenNode;
	lp_server *server = g_new0(lp_server, 1);


	while (cur)
	{
		key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"chatname")))
			server->chatname = strdup((char*)key);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"address")))
			server->address = strdup((char*)key);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"port")))
			server->port = atoi((char*)key);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"nick")))
			server->nick = strdup((char*)key);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"username")))
			server->username = strdup((char*)key);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"realname")))
			server->realname = strdup((char*)key);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"channel")))
			server->channels = g_list_append(server->channels, strdup((char*)key));
		xmlFree(key);
		cur = cur->next;
	}
	servers = g_list_append(servers, server);
	return(0);
}

int parseServers(char *docname)
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

	if(xmlStrcmp(cur->name, (const xmlChar *)"servers"))
	{
		fprintf(stderr, "document of the wrong type, root node != servers");
		xmlFreeDoc(doc);
		return(1);
	}

	cur = cur->xmlChildrenNode;
	while(cur)
	{
		key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if((!xmlStrcmp(cur->name, (const xmlChar *)"server")))
			if(parseServer(doc, cur)<0)
				return -1;
		xmlFree(key);
		cur = cur->next;
	}
	xmlFreeDoc(doc);
	return(0);
}
