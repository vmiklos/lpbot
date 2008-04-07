#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <glib.h>

#include "lpbot.h"

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
			ver->content = strdup((char*)key);
		xmlFree(key);
		cur = cur->next;
	}
	return ver;
}

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
			lp_record_ver *ver = g_list_nth_data(record->versions, i);

			xmlTextWriterStartElement(writer, BAD_CAST "version");
			xmlTextWriterSetIndent(writer, 3);
			xmlTextWriterWriteFormatElement(writer, BAD_CAST "date", "%d", ver->date);
			xmlTextWriterWriteFormatElement(writer, BAD_CAST "author", "%s", ver->author);
			xmlTextWriterWriteFormatElement(writer, BAD_CAST "content", "%d", ver->content);
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
