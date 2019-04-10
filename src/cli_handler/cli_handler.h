#ifndef CLI_HANDLER_H
#define CLI_HANDLER_H

#include <pthread.h>

typedef struct s_sniffer	t_sniffer;
typedef struct s_ipc		t_ipc;

typedef struct	s_cli_handler
{
	pthread_t	*thread_id;
	t_sniffer	*sniffer;
	t_ipc		*ipc;
}				t_cli_handler;

int	cli_handler_init(t_cli_handler *cli_handler, t_sniffer *sniffer);
int cli_handler_start(t_cli_handler *cli_handler);
int cli_handler_free(t_cli_handler *cli_handler);

int	_handle_start(t_cli_handler *cli_handler);
int	_handle_stop(t_cli_handler *cli_handler);
int	_handle_show_ip_count(t_cli_handler *cli_handler);
int	_handle_show_ifaces(t_cli_handler *cli_handler);
int	_handle_select_iface(t_cli_handler *cli_handler);
int	_handle_stat_iface(t_cli_handler *cli_handler);

#endif