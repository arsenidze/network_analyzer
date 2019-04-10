#include "ipc.h"
#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int	ipc_server_init(t_ipc *ipc)
{
	int					server_sd;
	struct sockaddr_in	*connection_info;
	int					status;
	int					option;

	connection_info = &ipc->connection_info;

	server_sd = socket(AF_INET, SOCK_STREAM, 0);
	option = 1;
	setsockopt(server_sd, SOL_SOCKET, SO_REUSEADDR, (const void *)&option, sizeof(option));
	if (server_sd == -1) {
		syslog(LOG_ERR, "%s\n", strerror(errno));
		printf("%s\n", strerror(errno));
		return (-1);
	}

	connection_info->sin_family = AF_INET;
	connection_info->sin_port = htons(IPC_PORT);
	connection_info->sin_addr.s_addr = inet_addr(IPC_IP_ADDR);

	status = bind(server_sd, (const struct sockaddr *)connection_info, sizeof(struct sockaddr_in));
	if (status == -1) {
		syslog(LOG_ERR, "%s\n", strerror(errno));
		printf("%s\n", strerror(errno));
		return (-1);
	}

	status = listen(server_sd, 1);
	if (status == -1) {
		syslog(LOG_ERR, "%s\n", strerror(errno));
		printf("%s\n", strerror(errno));
		return (-1);
	}

	ipc->server_sd = server_sd;
	ipc->is_server = 1;
	printf("Server inited");
	return (0);
}

int	ipc_client_init(t_ipc *ipc)
{
	int					client_sd;
	struct sockaddr_in	*connection_info;
	int					status;

	connection_info = &ipc->connection_info;

	client_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (client_sd == -1) {
		syslog(LOG_ERR, "%s\n", strerror(errno));
		// printf("%s\n", strerror(errno));
		return (-1);
	}

	connection_info->sin_family = AF_INET;
	connection_info->sin_port = htons(IPC_PORT);
	connection_info->sin_addr.s_addr = inet_addr(IPC_IP_ADDR);

	status = connect(client_sd, (const struct sockaddr *)connection_info, sizeof(struct sockaddr_in));
	if (status == -1) {
		syslog(LOG_ERR, "%s\n", strerror(errno));
		// printf("%s\n", strerror(errno));
		return (-1);
	}

	ipc->client_sd = client_sd;
	ipc->is_server = 0;
	return (0);
}


int	ipc_free(t_ipc *ipc)
{
	if (ipc->is_server) {
		close(ipc->server_sd);
	}
	else {
		close(ipc->client_sd);
	}
	return (0);
}

int	ipc_recv_size_and_msg(t_ipc *ipc)
{
	int 			nread;
	unsigned int	size;

	nread = recv(ipc->client_sd, (void *)&size, sizeof(size), 0);
	if (nread < 0 || nread != sizeof(size)) {
		syslog(LOG_ERR, "%s\n", strerror(errno));
		// printf("%s\n", strerror(errno));
		return (-1);
	}
	nread = recv(ipc->client_sd, ipc->recv_buf, size, 0);
	if (nread < 0) {
		syslog(LOG_ERR, "%s\n", strerror(errno));
		// printf("%s\n", strerror(errno));
		return (-1);
	}
	ipc->recv_buf[nread] = '\0';
	return (nread);	
}

int	ipc_send_size_and_msg(t_ipc *ipc)
{
	int				status;
	unsigned int	len;

	len = strlen(ipc->send_buf);
	status = send((ipc->client_sd), (void *)&len, sizeof(len), MSG_NOSIGNAL);
	if (status == -1) {
		syslog(LOG_ERR, "%s\n", strerror(errno));
		// printf("%s\n", strerror(errno));
		return (-1);
	}

	status = send(ipc->client_sd, ipc->send_buf, len, MSG_NOSIGNAL);
	if (status == -1) {
		syslog(LOG_ERR, "%s\n", strerror(errno));
		// printf("%s\n", strerror(errno));
		return (-1);
	}
	return (0);	
}