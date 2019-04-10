#include <syslog.h>
#include "daemon.h"
#include "ipc.h"
#include "sniffer.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

void	*handle_commands(void *arg)
{
	t_ipc	*ipc;
	int		nread;

	ipc = (t_ipc *)arg;
	while (1)
	{
		ipc->client_sd = accept(ipc->server_sd, (struct sockaddr *)NULL, NULL);
		if (ipc->client_sd == -1) {
			printf("%s\n", strerror(errno));
			continue ;
		}
		printf("I have catched connection\n");
		nread = ipc_recv(ipc);
		if (nread < 0) {
			continue ;
		}
		printf("I have recevied: %s\n", ipc->recv_buf);
		close(ipc->client_sd);
	}
	return (NULL);
}

int		main(int argc, char *argv[])
{
	t_sniffer	sniffer;
	t_ipc		ipc;
	
	pthread_t thread_id;	

	openlog(argv[0], LOG_CONS, LOG_USER);
    // syslog(LOG_INFO, "%s start", argv[0]);
    printf("%s start\n", argv[0]);

	// daemon_start();
    ipc_server_init(&ipc);
    printf("test0\n");
	// sniffer_init(&sniffer);

    pthread_create(&thread_id, NULL, handle_commands, &ipc);

    while (1) {
    	printf("123\n");
    	sleep(2);
    }

	// sniffer_free(&sniffer);
	ipc_free(&ipc);

	// syslog(LOG_INFO, "%s end", argv[0]);
	printf("%s end\n", argv[0]);
	closelog();
	return (0);
}