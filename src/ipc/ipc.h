#ifndef IPC_H
#define IPC_H

#define MAX_MSG_LEN 2048

#define FIFO_SERVER "fifo_server"
#define FIFO_CLIENT "fifo_client"

typedef struct	s_ipc
{
	int		is_server;
	int		client_to_server;
	int		server_to_client;
	char	recv_buf[MAX_MSG_LEN + 1];
	char	send_buf[MAX_MSG_LEN + 1];
}				t_ipc;

int	ipc_server_init(t_ipc *ipc);
int	ipc_client_init(t_ipc *ipc);
int	ipc_server_free(t_ipc *ipc);
int	ipc_client_free(t_ipc *ipc);
int	ipc_recv(t_ipc *ipc);
int	ipc_send(t_ipc *ipc);

#endif