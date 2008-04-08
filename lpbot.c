#include "lpbot.h"

/*
 * The main function of the bot
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
	return 0;
}
