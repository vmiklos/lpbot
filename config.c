#include <stdio.h>
#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <glib.h>

#include "lpbot.h"

static int parseOptions(xmlDoc *doc, xmlNode *cur)
{
	xmlChar *key;
	cur = cur->xmlChildrenNode;


	while (cur)
	{
		key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"ident_method")))
		{
			if(!strcmp((char*)key, "pass"))
				config->ident_method = LP_IDENT_PASS;
			else if(!strcmp((char*)key, "services"))
				config->ident_method = LP_IDENT_SERVICES;
		}
		xmlFree(key);
		cur = cur->next;
	}
	return(0);
}
int parseConfig(char *docname)
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

	if(xmlStrcmp(cur->name, (const xmlChar *)"config"))
	{
		fprintf(stderr, "document of the wrong type, root node != config");
		xmlFreeDoc(doc);
		return(1);
	}

	cur = cur->xmlChildrenNode;
	while(cur)
	{
		key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if((!xmlStrcmp(cur->name, (const xmlChar *)"server")))
		{
			if(parseServer(doc, cur)<0)
				return -1;
		}
		else if((!xmlStrcmp(cur->name, (const xmlChar *)"user")))
		{
			if(parseUser(doc, cur)<0)
				return -1;
		}
		else if((!xmlStrcmp(cur->name, (const xmlChar *)"options")))
		{
			if(parseOptions(doc, cur)<0)
				return -1;
		}
		xmlFree(key);
		cur = cur->next;
	}
	xmlFreeDoc(doc);
	return(0);
}