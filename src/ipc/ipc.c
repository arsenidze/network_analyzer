#include "ipc.h"
// #include <sys/stat.h>
// #include <sys/types.h>
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

	connection_info = &ipc->connection_info;

	server_sd = socket(PF_INET, SOCK_STREAM, 0);
	if (server_sd == -1) {
		printf("%s\n", strerror(errno));
		return (-1);
	}

	connection_info->sin_family = AF_INET;
	connection_info->sin_port = htons(IPC_PORT);
	connection_info->sin_addr.s_addr = inet_addr(IPC_IP_ADDR);

	if (bind(server_sd, (const struct sockaddr *)connection_info, sizeof(struct sockaddr_in)) == -1) {
		printf("%s\n", strerror(errno));
		return (-1);
	}

	if (listen(server_sd, 1) == -1) {
		printf("%s\n", strerror(errno));
		return (-1);
	}

	ipc->server_sd = server_sd;
	ipc->is_server = 1;
	return (0);
}

int	ipc_client_init(t_ipc *ipc)
{
	int					client_sd;
	struct sockaddr_in	*connection_info;

	connection_info = &ipc->connection_info;

	client_sd = socket(PF_INET, SOCK_STREAM, 0);
	if (client_sd == -1) {
		printf("%s\n", strerror(errno));
		return (-1);
	}

	connection_info->sin_family = AF_INET;
	connection_info->sin_port = htons(IPC_PORT);
	connection_info->sin_addr.s_addr = inet_addr(IPC_IP_ADDR);

	if (connect(client_sd, (const struct sockaddr *)connection_info, sizeof(struct sockaddr_in)) == -1) {
		printf("%s\n", strerror(errno));
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

int ipc_recv(t_ipc *ipc)
{
	int nread;

	nread = recv(ipc->client_sd, ipc->recv_buf, sizeof(ipc->recv_buf) - 1, 0);
	if (nread < 0) {
		printf("%s\n", strerror(errno));
		return (-1);
	}
	ipc->recv_buf[nread] = '\0';
	return (nread);
}

int	ipc_send(t_ipc *ipc)
{
	int	status;

	status = send(ipc->client_sd, ipc->send_buf, strlen(ipc->send_buf), 0);
	if (status == -1) {
		printf("%s\n", strerror(errno));
		return (-1);
	}
	return (0);
}

// int ipc_try_recv(t_ipc *ipc)
// {
// 	int 			nread;
// 	fd_set			rfds;
//     struct timeval	tv;
//     int 			ret;

//     FD_ZERO(&rfds);
//     FD_SET(ipc->from, &rfds);
//     tv.tv_sec = 5;
//     tv.tv_usec = 0;

//     ret = select(ipc->from + 1, &rfds, NULL, NULL, &tv);
//     printf("ret: %d\n", ret);
//     if (ret > 0 && FD_ISSET(ipc->from, &rfds)) {
// 		nread = read(ipc->from, ipc->recv_buf, sizeof(ipc->recv_buf));
// 		if (nread < 0) {
// 			printf("%s\n", strerror(errno));
// 			return (-1);
// 		}
// 		ipc->recv_buf[nread] = '\0';
// 		return (nread);
//     }
//     else if (ret < 0) {
//     	printf("%s\n", strerror(errno));
//         return (-1);
//     }
//     else {
//     	return (0);
//     }
// }