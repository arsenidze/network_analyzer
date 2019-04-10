#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "cli_handler.h"
#include "ipc.h"
#include "sniffer.h"

typedef struct	s_request
{
	char	*word;
	int		(*process)(t_cli_handler *);

}				t_request;

#define NREQUESTS 6

static t_request	requests[NREQUESTS] = {
	{"start", _handle_start},
	{"stop", _handle_stop},
	{"show_ip_count", _handle_show_ip_count},
	{"show_ifaces", _handle_show_ifaces},
	{"select_iface", _handle_select_iface},
	{"stat_iface", _handle_stat_iface}
};

int	cli_handler_init(t_cli_handler *cli_handler, t_sniffer *sniffer)
{
	cli_handler->sniffer = sniffer;
	return (0);
}
	
static int	_find_request_prototype(char *request)
{
	for (int i = 0; i < NREQUESTS; ++i)
	{
		if (strcmp(requests[i].word, request) == 0) {
			return (i);
		}
	}
	return (-1);
}

static void	*_handle_cli_requests(void *arg)
{
	t_cli_handler	*cli_handler;
	t_ipc			ipc;
	int				size;
	int				request_idx;

	cli_handler = (t_cli_handler *)arg;
	ipc_server_init(&ipc);
	cli_handler->ipc = &ipc;
	while (1)
	{
		ipc.client_sd = accept(ipc.server_sd, (struct sockaddr *)NULL, NULL);
		if (ipc.client_sd == -1) {
			printf("%s\n", strerror(errno));
			continue ;
		}
		printf("I have catched connection\n");
		
		size = ipc_recv(&ipc);
		if (size < 0) {
			continue ;
		}
		printf("I have recevied: %s\n", ipc.recv_buf);

		request_idx = _find_request_prototype(ipc.recv_buf);
		if (request_idx < 0) {
			strcpy(ipc.send_buf, "Unknown command");
			ipc_send(&ipc);
		}
		else {
			requests[request_idx].process(cli_handler);
		}

		close(ipc.client_sd);
	}
	ipc_free(&ipc);
	return (NULL);
}

int cli_handler_start(t_cli_handler *cli_handler)
{
	pthread_t	thread_id;
	int			status;

	status = pthread_create(&thread_id, NULL, _handle_cli_requests, cli_handler);
	if (status != 0) {
		fprintf(stderr, "Problem with creating cli handler\n");
		return (-1);
	}
	cli_handler->thread_id = &thread_id;
	return (0);
}

int	_cli_handler_free(t_cli_handler *cli_handler)
{
	(void)cli_handler;
	// pthread_join(_handle->thread_id);
	return (0);
}

int	_handle_start(t_cli_handler *cli_handler)
{
	if (cli_handler->sniffer->is_active) {
		strcpy(cli_handler->ipc->send_buf, "Sniffer already started");
		ipc_send(cli_handler->ipc);
	}
	else {
		// sniffer_start(cli_handler->sniffer);
		cli_handler->sniffer->is_active = 1;
		strcpy(cli_handler->ipc->send_buf, "Sniffer was started");
		ipc_send(cli_handler->ipc);
	}
	return (0);
}

int	_handle_stop(t_cli_handler *cli_handler)
{
	if (!cli_handler->sniffer->is_active) {
		strcpy(cli_handler->ipc->send_buf, "Sniffer already stoped");
		ipc_send(cli_handler->ipc);
	}
	else {
		// sniffer_start(cli_handler->sniffer);
		cli_handler->sniffer->is_active = 0;
		strcpy(cli_handler->ipc->send_buf, "Sniffer was stoped");
		ipc_send(cli_handler->ipc);
	}
	return (0);
}

int	_handle_show_ip_count(t_cli_handler *cli_handler)
{
	return (0);
}

int	_handle_show_ifaces(t_cli_handler *cli_handler)
{
	return (0);
}

int	_handle_select_iface(t_cli_handler *cli_handler)
{
	return (0);
}

int	_handle_stat_iface(t_cli_handler *cli_handler)
{
	return (0);
}