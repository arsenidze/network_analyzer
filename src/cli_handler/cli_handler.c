#include <pthread.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
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
	int				status;

	cli_handler = (t_cli_handler *)arg;
	printf("hello\n");
	while (1) {
		status = ipc_server_init(&ipc);
		if (status < 0) {
			sleep(3);
			printf("hello\n");
		}
		else {
			break ;
		}
	}
	cli_handler->ipc = &ipc;
	while (cli_handler->is_active)
	{
		ipc.client_sd = accept(ipc.server_sd, (struct sockaddr *)NULL, NULL);
		if (ipc.client_sd == -1) {
			syslog(LOG_ERR, "%s\n", strerror(errno));
			continue ;
		}
		syslog(LOG_DEBUG, "I have catched connection\n");
		
		size = ipc_recv_size_and_msg(&ipc);
		if (size < 0) {
			continue ;
		}
		syslog(LOG_DEBUG, "I have recevied: %s\n", ipc.recv_buf);

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

	cli_handler->is_active = 1;
	status = pthread_create(&thread_id, NULL, _handle_cli_requests, cli_handler);
	if (status != 0) {
		syslog(LOG_ERR, "Problem with creating cli handler\n");
		return (-1);
	}
	cli_handler->thread_id = &thread_id;
	return (0);
}

int	_cli_handler_free(t_cli_handler *cli_handler)
{
	cli_handler->is_active = 0;
	pthread_join(*(cli_handler->thread_id), NULL);
	return (0);
}

int	_handle_start(t_cli_handler *cli_handler)
{
	if (cli_handler->sniffer->is_active) {
		strcpy(cli_handler->ipc->send_buf, "Sniffer already started");
		ipc_send_size_and_msg(cli_handler->ipc);
	}
	else {
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

	nstat_lookup_ip_times(cli_handler->sniffer->nstats[cli_handler->sniffer->interface_idx], 
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
	int		size;
	char	*ret;
	char	**all_stat;


	size = ipc_recv_size_and_msg(cli_handler->ipc);
	if (size < 0) {
		strcpy(cli_handler->ipc->send_buf, "Problem with getting iface");
		ipc_send_size_and_msg(cli_handler->ipc);
		return (-1);
	}
	ret = sniffer_get_interface_stat(cli_handler->sniffer, cli_handler->ipc->recv_buf);
	if (ret != NULL) {
		strcpy(cli_handler->ipc->send_buf, "1");
		ipc_send_size_and_msg(cli_handler->ipc);

		strcpy(cli_handler->ipc->send_buf, ret);
		ipc_send_size_and_msg(cli_handler->ipc);
		free(ret);
		return (0);
	}
	all_stat = sniffer_get_all_stat(cli_handler->sniffer);
	if (all_stat == NULL) {
		strcpy(cli_handler->ipc->send_buf, "Problem with getting statistic");
		ipc_send_size_and_msg(cli_handler->ipc);
		return (-1);
	}
	sprintf(cli_handler->ipc->send_buf, "%d", cli_handler->sniffer->num_active_interfaces);
	ipc_send_size_and_msg(cli_handler->ipc);
	for (int i = 0; i < cli_handler->sniffer->num_active_interfaces; ++i)
	{
		printf("LLLLLLLLL: %s\n", all_stat[i]);
		strcpy(cli_handler->ipc->send_buf, all_stat[i]);
		ipc_send_size_and_msg(cli_handler->ipc);
		free(all_stat[i]);
	}
	return (0);
}