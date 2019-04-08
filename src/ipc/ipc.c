#include "ipc.h"
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

int	ipc_server_init(t_ipc *ipc)
{
	int		status;
	int		client_to_server;
	int		server_to_client;

	status = mkfifo(FIFO_SERVER, 0666);
	if (status < 0) {
		printf("%s\n", strerror(errno));
		return (-1);
	}
	status = mkfifo(FIFO_CLIENT, 0666);
	if (status < 0) {
		printf("%s\n", strerror(errno));
		return (-1);
	}

	client_to_server = open(FIFO_SERVER, O_RDONLY);
	if (client_to_server < 0) {
		printf("%s\n", strerror(errno));
		return (-1);
	}
	server_to_client = open(FIFO_CLIENT, O_WRONLY);
	if (server_to_client < 0) {
		printf("%s\n", strerror(errno));
		return (-1);
	}

	ipc->client_to_server = client_to_server;
	ipc->server_to_client = server_to_client;
	ipc->is_server = 1;
	return (0);
}

int	ipc_client_init(t_ipc *ipc)
{
	int		client_to_server;
	int		server_to_client;

	client_to_server = open(FIFO_SERVER, O_WRONLY);
	if (client_to_server < 0) {
		printf("%s\n", strerror(errno));
		return (-1);
	}
	server_to_client = open(FIFO_CLIENT, O_RDONLY);
	if (server_to_client < 0) {
		printf("%s\n", strerror(errno));
		return (-1);
	}

	ipc->client_to_server = client_to_server;
	ipc->server_to_client = server_to_client;
	ipc->is_server = 0;
	return (0);
}


int	ipc_server_free(t_ipc *ipc)
{
	close(ipc->client_to_server);
	close(ipc->server_to_client);

	unlink(FIFO_CLIENT);
	unlink(FIFO_SERVER);
	return (0);
}

int	ipc_client_free(t_ipc *ipc)
{
	close(ipc->client_to_server);
	close(ipc->server_to_client);
	return (0);
}

int ipc_recv(t_ipc *ipc)
{
	int	from;
	int nread;

	from = ipc->is_server ? ipc->client_to_server : ipc->server_to_client;
	nread = read(from, ipc->recv_buf, sizeof(ipc->recv_buf));
	if (nread < 0) {
		printf("%s\n", strerror(errno));
		return (-1);
	}
	ipc->recv_buf[nread] = '\0';
	return (nread);
}

int	ipc_send(t_ipc *ipc)
{
	int	where;
	int	status;

	where = ipc->is_server ? ipc->server_to_client : ipc->client_to_server;
	status = write(where, ipc->send_buf, strlen(ipc->send_buf));
	if (status < 0) {
		printf("%s\n", strerror(errno));
		return (-1);
	}
	return (0);
}