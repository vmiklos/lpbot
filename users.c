/*
 *  users.c
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

/** @defgroup users Handling of users
 * @{
 */

/** Parses an entry of a user in the config
 * @param doc the xml document
 * @param cur the current xml node
 * @return 0 on success, -1 on failure
 */
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
/** Frees a user.
 * @param usr the user to free
 */
void lp_user_free(lp_user *usr)
{
	free(usr->login);
	free(usr->email);
	free(usr->pass);
}
/* @} */
