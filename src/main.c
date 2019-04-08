#include <syslog.h>
#include "daemon.h"
#include "sniffer.h"

int		main(int argc, char *argv[])
{
	t_sniffer	sniffer;

	openlog(argv[0], LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "%s start", argv[0]);
    // printf("%s start", argv[0]);

	daemon_start();
	sniffer_init(&sniffer);

	sniffer_start(&sniffer);

	sniffer_free(&sniffer);

	syslog(LOG_INFO, "%s end", argv[0]);
	// printf("%s end", argv[0]);
	closelog();
	return (0);
}