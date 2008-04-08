#include <stdio.h>
#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <glib.h>

#include "lpbot.h"

/** @defgroup config Configuration file
 * @{
 */
/** Parses the general options of the bot.
 * @param doc the xml document
 * @param cur the current xml node
 * @return 0 on success, -1 on failure
 */
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
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"rss_interval")))
			config->rss_interval = atoi((char*)key);
		xmlFree(key);
		cur = cur->next;
	}
	return(0);
}
/** Parses the configuration file of the bot.
 * @param docname the filename of the config xml
 * @return 0 on success, -1 on failure
 */
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
		else if((!xmlStrcmp(cur->name, (const xmlChar *)"rss")))
		{
			if(parseRss(doc, cur)<0)
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
/** Frees a config ptr.
 * @param cfg the config ptr
 */
void lp_config_free(lp_config *cfg)
{
	int i;
	for(i=0;i<g_list_length(cfg->servers);i++)
		lp_server_free(g_list_nth_data(cfg->servers, i));
	g_list_free(cfg->servers);
	for(i=0;i<g_list_length(cfg->users);i++)
		lp_user_free(g_list_nth_data(cfg->users, i));
	g_list_free(cfg->users);
	for(i=0;i<g_list_length(cfg->records);i++)
		lp_record_free(g_list_nth_data(cfg->records, i));
	g_list_free(cfg->records);
	for(i=0;i<g_list_length(cfg->rsslist);i++)
		lp_rss_free(g_list_nth_data(cfg->rsslist, i));
	g_list_free(cfg->rsslist);
	if(cfg->ident_to)
		free(cfg->ident_to);
}
/* @} */
