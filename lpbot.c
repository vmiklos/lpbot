/*
 *  lpbot.c
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

#include "lpbot.h"

/** @defgroup lpbot The IRC bot
 * @{
 */

/** The main function of the bot
 * @return 0 on success, -1 on error
 */
int main()
{
	int i;
	GMainLoop *loop;
	config = g_new0(lp_config, 1);

	parseConfig("config.xml");
	parseRecords("db.xml");
	lp_serve();
	g_timeout_add(config->rss_interval*1000, lp_check_rss, NULL);

	for(i=0;i<g_list_length(config->servers);i++)
		lp_connect(g_list_nth_data(config->servers, i));

	loop = g_main_loop_new(NULL, TRUE);
	g_main_loop_run(loop);
	lp_config_free(config);
	return 0;
}
/* @} */
