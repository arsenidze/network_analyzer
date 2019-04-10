#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
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
		
		size = ipc_recv_size_and_msg(&ipc);
		if (size < 0) {
			continue ;
		}
		printf("I have recevied: %s\n", ipc.recv_buf);

		request_idx = _find_request_prototype(ipc.recv_buf);
		if (request_idx < 0) {
			strcpy(ipc.send_buf, "Unknown command");
			ipc_send_size_and_msg(&ipc);
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
		ipc_send_size_and_msg(cli_handler->ipc);
	}
	else {
		// sniffer_start(cli_handler->sniffer);
		cli_handler->sniffer->is_active = 1;
		strcpy(cli_handler->ipc->send_buf, "Sniffer was started");
		ipc_send_size_and_msg(cli_handler->ipc);
	}
	return (0);
}

int	_handle_stop(t_cli_handler *cli_handler)
{
	if (!cli_handler->sniffer->is_active) {
		strcpy(cli_handler->ipc->send_buf, "Sniffer already stoped");
		ipc_send_size_and_msg(cli_handler->ipc);
	}
	else {
		// sniffer_start(cli_handler->sniffer);
		cli_handler->sniffer->is_active = 0;
		strcpy(cli_handler->ipc->send_buf, "Sniffer was stoped");
		ipc_send_size_and_msg(cli_handler->ipc);
	}
	return (0);
}

static int	_check_ip_prototype(char *str)
{
	int	offset;
	int	i;

	offset = 0;
	for (int j = 0; j < 3; ++j)
	{
		for (i = 0; i < 3 && str[offset + i] != '.'; ++i)
		{
			if (!isdigit(str[offset + i])) {
				return (-1);
			}
		}
		if (i == 0) {
			return (-1);
		}
		if (str[offset + i] != '.') {
			return (-1);
		}
		offset += i + 1;
	}
	for (i = 0; i < 3 && str[offset + i] != '\0'; ++i)
	{
		if (!isdigit(str[offset + i])) {
			return (-1);
		}
	}
	if (str[offset + i] != '\0') {
		return (-1);
	}
	return (0);
}

#define MIN_IP_LEN	7
#define MAX_IP_LEN	15

int	_handle_show_ip_count(t_cli_handler *cli_handler)
{
	int				size;
	int				status;
	unsigned int	times[2];

	size = ipc_recv_size_and_msg(cli_handler->ipc);
	if (size < MIN_IP_LEN || size > MAX_IP_LEN) {
		strcpy(cli_handler->ipc->send_buf, "Error: Wrong ip format");
		ipc_send_size_and_msg(cli_handler->ipc);
		return (-1);
	}
	status = _check_ip_prototype(cli_handler->ipc->recv_buf);
	if (status < 0) {
		strcpy(cli_handler->ipc->send_buf, "Error: Wrong ip format");
		ipc_send_size_and_msg(cli_handler->ipc);
		return (-1);
	}

	nstat_lookup_ip_times(cli_handler->sniffer->nstat, 
		cli_handler->ipc->recv_buf, times);
	
	sprintf(cli_handler->ipc->send_buf, "%u", times[0]);
	ipc_send_size_and_msg(cli_handler->ipc);
	return (0);
}

#define MAX_ERROR_LEN 256

int	_handle_select_iface(t_cli_handler *cli_handler)
{
	int				size;
	int				status;
	char			errbuf[MAX_ERROR_LEN];

	size = ipc_recv_size_and_msg(cli_handler->ipc);
	if (size < 0) {
		strcpy(cli_handler->ipc->send_buf, "Problem with command");
		ipc_send_size_and_msg(cli_handler->ipc);
		return (-1);
	}
	status = sniffer_try_set_interface(cli_handler->sniffer, cli_handler->ipc->recv_buf, errbuf);
	if (status < 0) {
		strcpy(cli_handler->ipc->send_buf, errbuf);
		ipc_send_size_and_msg(cli_handler->ipc);
		return (-1);
	}
	else {
		strcpy(cli_handler->ipc->send_buf, "Interface selected");
		ipc_send_size_and_msg(cli_handler->ipc);
		return (0);
	}
}

int	_handle_stat_iface(t_cli_handler *cli_handler)
{
	return (0);
}