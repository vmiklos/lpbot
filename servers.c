/*
 *  servers.c
 *
 *  Copyright (c) 2008 by Miklos Vajna <vmiklos@vmiklos.hu>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 *  USA.
 */

#include <stdio.h>
#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <glib.h>

#include "lpbot.h"

/** @defgroup servers Handling or servers
 * @{
 */

/** Searches for a user
 * @param chatname the id of the server
 * @return the server pointer on success, NULL on error
 */
lp_server *lp_find_server(char *chatname)
{
	int i;

	for(i=0;i<g_list_length(config->servers);i++)
	{
		lp_server *server = g_list_nth_data(config->servers, i);
		if(!strcmp(server->chatname, chatname))
			return server;
	}
	return NULL;
}

/** Parses an entry of a server in the config
 * @param doc the xml document
 * @param cur the current xml node
 * @return 0 on success, -1 on failure
 */
int parseServer(xmlDoc *doc, xmlNode *cur)
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
	config->servers = g_list_append(config->servers, server);
	return(0);
}

/** Frees a server.
 * @param srv the server to free
 */
void lp_server_free(lp_server *srv)
{
	free(srv->chatname);
	free(srv->address);
	free(srv->nick);
	free(srv->username);
	free(srv->realname);
	g_list_free(srv->channels);
}
/* @} */
