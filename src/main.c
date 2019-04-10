#include "daemon.h"
#include "cli_handler.h"
#include "sniffer.h"
#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

// void	*handle_commands(void *arg)
// {
// 	t_ipc	*ipc;
// 	int		nread;

// 	ipc = (t_ipc *)arg;
// 	while (1)
// 	{
// 		ipc->client_sd = accept(ipc->server_sd, (struct sockaddr *)NULL, NULL);
// 		if (ipc->client_sd == -1) {
// 			printf("%s\n", strerror(errno));
// 			continue ;
// 		}
// 		printf("I have catched connection\n");

// 		nread = ipc_recv(ipc);
// 		if (nread < 0) {
// 			continue ;
// 		}
// 		printf("I have recevied: %s\n", ipc->recv_buf);
// 		close(ipc->client_sd);
// 	}
// 	return (NULL);
// }

int		main(int argc, char *argv[])
{
	t_sniffer		sniffer;
	t_cli_handler	cli_handler;

	openlog(argv[0], LOG_CONS, LOG_USER);
    // syslog(LOG_INFO, "%s start", argv[0]);
    printf("%s start\n", argv[0]);

	// daemon_start();
    printf("test0\n");
	sniffer_init(&sniffer);
    cli_handler_init(&cli_handler, &sniffer);

    cli_handler_start(&cli_handler);

    while (1) {
    	if (sniffer.is_active) {
    		sniffer_sniff(&sniffer);
    	}
    	else {
	    	printf("Sniffer stoped\n");
	    	sleep(2);
    	}
    }

    cli_handler_free(&cli_handler);
	sniffer_free(&sniffer);

	// syslog(LOG_INFO, "%s end", argv[0]);
	printf("%s end\n", argv[0]);
	closelog();
	return (0);
}