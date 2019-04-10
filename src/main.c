#include "daemon.h"
#include "cli_handler.h"
#include "sniffer.h"
#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

int		main(int argc, char *argv[])
{
	t_sniffer		sniffer;
	t_cli_handler	cli_handler;

	(void)argc;
	openlog(argv[0], LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "%s start", argv[0]);

	daemon_start();
	if (sniffer_init(&sniffer) < 0) {
		return (-1);
	}
    if (cli_handler_init(&cli_handler, &sniffer) < 0) {
    	return (-1);
    }

    cli_handler_start(&cli_handler);

    while (1) {
    	if (sniffer.is_active) {
    		sniffer_sniff(&sniffer);
    	}
    	else {
	    	syslog(LOG_DEBUG, "Sniffer stoped");
	    	sleep(2);
    	}
    }

    cli_handler_free(&cli_handler);
	sniffer_free(&sniffer);

	syslog(LOG_INFO, "%s end", argv[0]);
	closelog();
	return (0);
}