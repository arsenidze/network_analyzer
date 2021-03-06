#ifndef IPC_H
#define IPC_H

#include <arpa/inet.h>

#define IPC_MAX_MSG_LEN 2048

#define IPC_PORT	5050
#define IPC_IP_ADDR	"127.0.0.1"

typedef struct	s_ipc
{
	int					is_server;
	int					server_sd;
	int					client_sd;
	struct sockaddr_in	connection_info;
	char				recv_buf[IPC_MAX_MSG_LEN];
	char				send_buf[IPC_MAX_MSG_LEN];
}				t_ipc;

int	ipc_server_init(t_ipc *ipc);
int	ipc_client_init(t_ipc *ipc);
int	ipc_free(t_ipc *ipc);
int	ipc_recv_size_and_msg(t_ipc *ipc);
int	ipc_send_size_and_msg(t_ipc *ipc);

#endif