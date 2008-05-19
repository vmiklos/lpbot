/*
 *  rss.c
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
#include <mrss.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "lpbot.h"

/** @defgroup rss The RSS parser
 * @{
 */

/** Parses the RSS part of the configuration file.
 * @param doc the xml document
 * @param cur the current xml node
 * @return 0 on success, -1 on failure
 */
int parseRss(xmlDoc *doc, xmlNode *cur)
{
	xmlChar *key;
	cur = cur->xmlChildrenNode;
	lp_rss *rss = g_new0(lp_rss, 1);


	while (cur)
	{
		key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
			rss->name = strdup((char*)key);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"url")))
			rss->url = strdup((char*)key);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"server")))
			rss->server = lp_find_server((char*)key);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"channel")))
			rss->channel = strdup((char*)key);
		xmlFree(key);
		cur = cur->next;
	}
	config->rsslist = g_list_append(config->rsslist, rss);
	return 0;
}

/*
 * Chesk an RSS feed for new entries.
 * @param rss the feed
 * @return -1 on error, 0 on success
 */
int check_rss(lp_rss *rss)
{
	mrss_t *data;
	mrss_error_t ret;
	mrss_item_t *item;

	if ((ret=mrss_parse_url(rss->url, &data)))
	{
		fprintf (stderr, "MRSS return error: %s\n", mrss_strerror (ret));
		return -1;
	}

	item = data->item;
	while (item)
	{
		// don't print any item for the first time
		if(rss->lastupd==0)
			break;
		if(get_date(item->pubDate, NULL)>rss->lastupd)
		{
			lp_send(rss->server, "privmsg %s :14%s7 %s3 %s\n",
					rss->channel, rss->name, item->title, item->link);
		}
		item = item->next;
	}
	rss->lastupd=get_date(data->item->pubDate, NULL);
	mrss_free (data);
	return 0;
}

/** Frees an rss feed.
 * @param rss the rss feed to free
 */
void lp_rss_free(lp_rss *rss)
{
	free(rss->name);
	free(rss->url);
	free(rss->channel);
}
/* @} */
