#include <syslog.h>
#include "daemon.h"
#include "ipc.h"
#include "sniffer.h"

int		main(int argc, char *argv[])
{
	t_sniffer	sniffer;
	t_ipc		ipc;

	openlog(argv[0], LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "%s start", argv[0]);
    // printf("%s start", argv[0]);

	daemon_start();
    ipc_server_init(&ipc);
	sniffer_init(&sniffer);

	// sniffer_start(&sniffer);

	//main_loop(&sniffer, &ipc);

	sniffer_free(&sniffer);
	ipc_server_free(&ipc);

	syslog(LOG_INFO, "%s end", argv[0]);
	// printf("%s end", argv[0]);
	closelog();
	return (0);
}