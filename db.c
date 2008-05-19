/*
 *  db.c
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
#include <sys/utsname.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <glib.h>

#include "lpbot.h"

/** @defgroup db Database handling
 * @{
 */
/** Parses a given version of a database record
 * @param doc the xml document
 * @param cur the current xml node
 * @return 0 on success, -1 on failure
 */
static lp_record_ver *parseVersion(xmlDoc *doc, xmlNode *cur)
{
	xmlChar *key;
	cur = cur->xmlChildrenNode;
	lp_record_ver *ver = g_new0(lp_record_ver, 1);

	while (cur != NULL)
	{
		key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"date")))
			ver->date = atoi((char*)key);
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"author")))
			ver->author = strdup((char*)key);
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"content")))
			ver->content = g_string_new((char*)key);
		xmlFree(key);
		cur = cur->next;
	}
	return ver;
}

/** Parses a given record of the database
 * @param doc the xml document
 * @param cur the current xml node
 * @return 0 on success, -1 on failure
 */
static int parseRecord(xmlDoc *doc, xmlNode *cur)
{
	xmlChar *key;
	cur = cur->xmlChildrenNode;
	lp_record *record = g_new0(lp_record, 1);

	while (cur != NULL)
	{
		key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
			record->name = strdup((char*)key);
		if((!xmlStrcmp(cur->name, (const xmlChar *)"version")))
			record->versions = g_list_append(record->versions, parseVersion(doc, cur));
		xmlFree(key);
		cur = cur->next;
	}
	config->records = g_list_append(config->records, record);
	return(0);
}

/** Parses the database file of the bot.
 * @param docname the filename of the db xml
 * @return 0 on success, -1 on failure
 */
int parseRecords(char *docname)
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

	if(xmlStrcmp(cur->name, (const xmlChar *)"records"))
	{
		fprintf(stderr, "document of the wrong type, root node != records");
		xmlFreeDoc(doc);
		return(1);
	}

	cur = cur->xmlChildrenNode;
	while(cur != NULL)
	{
		key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if((!xmlStrcmp(cur->name, (const xmlChar *)"record")))
			if(parseRecord(doc, cur))
				return(1);
		xmlFree(key);
		cur = cur->next;
	}
	xmlFreeDoc(doc);
	return(0);
}

/** Writes the database of the bot to a file.
 * @param docname the filename of the db xml
 * @return 0 on success, -1 on failure
 */
int saveRecords(char *docname)
{
	int i, j;
	xmlTextWriterPtr writer;

	writer = xmlNewTextWriterFilename("db.xml", 0);
	xmlTextWriterSetIndentString(writer, BAD_CAST "\t");
	xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);
	xmlTextWriterStartElement(writer, BAD_CAST "records");
	for(i=0;i<g_list_length(config->records);i++)
	{
		lp_record *record = g_list_nth_data(config->records, i);

		xmlTextWriterSetIndent(writer, 1);
		xmlTextWriterStartElement(writer, BAD_CAST "record");
		xmlTextWriterSetIndent(writer, 2);
		xmlTextWriterWriteFormatElement(writer, BAD_CAST "name", "%s", record->name);
		for(j=0;j<g_list_length(record->versions);j++)
		{
			lp_record_ver *ver = g_list_nth_data(record->versions, j);

			xmlTextWriterStartElement(writer, BAD_CAST "version");
			xmlTextWriterSetIndent(writer, 3);
			xmlTextWriterWriteFormatElement(writer, BAD_CAST "date", "%d", ver->date);
			xmlTextWriterWriteFormatElement(writer, BAD_CAST "author", "%s", ver->author);
			xmlTextWriterWriteFormatElement(writer, BAD_CAST "content", "%s", ver->content->str);
			xmlTextWriterSetIndent(writer, 2);
			xmlTextWriterEndElement(writer);
		}
		xmlTextWriterSetIndent(writer, 1);
		xmlTextWriterEndElement(writer);
	}
	xmlTextWriterSetIndent(writer, 0);
	xmlTextWriterEndElement(writer);
	xmlTextWriterEndDocument(writer);
	xmlFreeTextWriter(writer);
	return 0;
}

/** Frees a record.
 * @param rec the record to free
 */
void lp_record_free(lp_record *rec)
{
	int i;
	free(rec->name);
	for(i=0;i<g_list_length(rec->versions);i++)
		lp_record_ver_free(g_list_nth_data(rec->versions, i));
	g_list_free(rec->versions);
}

/** Frees a record version.
 * @param ver the record version to free
 */
void lp_record_ver_free(lp_record_ver *ver)
{
	free(ver->author);
	g_string_free(ver->content, TRUE);
}
/* @} */
