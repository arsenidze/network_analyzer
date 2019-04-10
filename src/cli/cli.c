#include "cli.h"
#include "ipc.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define	MAX_NWORDS 3

typedef struct	s_command
{
	char	*words[MAX_NWORDS];
	int		nwords;
	int		(*execute)(int argc, char *argv[]);
	char	*help_msg;

}				t_command;

#define NCOMMANDS 7

static t_command	commands[NCOMMANDS] = {
	{{"start"}, 1, cli_start, "start"},
	{{"stop"}, 1, cli_stop, "stop"},
	{{"show", NULL, "count"}, 3, cli_show_ip_count, "show [ip] count"},
	{{"show", "ifaces"}, 2, cli_show_ifaces, "show ifaces"},
	{{"select", "iface", NULL}, 3, cli_select_iface, "select iface [iface]"},
	{{"stat", NULL}, 2, cli_stat_iface, "stat [iface]"},
	{{"--help"}, 1, cli_help, "--help"}
};

int	cli_help(int argc, char *argv[])
{
	(void)argc;
	printf("Usage: %s\t%s\n", argv[0], commands[0].help_msg);
	for (int i = 1; i < NCOMMANDS; ++i)
	{
		printf("\t\t%s\n", commands[i].help_msg);
	}
	return (0);
}

static int	_find_command_prototype(int argc, char *argv[])
{
	int	i;
	int	j;

	for (i = 0; i < NCOMMANDS; ++i)
	{
		if (commands[i].nwords != argc - 1) {
			continue ;
		}
		for (j = 0; j < commands[i].nwords; ++j)
		{
			if (commands[i].words[j] == NULL) {
				continue ;
			}
			if (strcmp(commands[i].words[j], argv[j + 1]) != 0) {
				break ;
			}
		}
		if (j == commands[i].nwords) {
			return (i);
		}
	}
	if (i == NCOMMANDS) {
		return (-1);
	}
}

static int	_execute_command(int argc, char *argv[], int command_idx)
{
	int	status;

	status = commands[command_idx].execute(argc, argv);
	return (status);
}

int	cli_handle_command(int argc, char *argv[])
{
	int	command_idx;
	int	status;

	command_idx = _find_command_prototype(argc, argv);
	if (command_idx < 0) {
		fprintf(stderr, "Command doesn't match to any prototype\n");
		return (-1);
	}

	status = _execute_command(argc, argv, command_idx);
	if (status < 0) {
		fprintf(stderr, "Failed to execute command\n");
	}
	return (status);
}

int	cli_start(int argc, char *argv[])
{
	t_ipc	ipc;
	int		status;
	int		size;

	ipc_client_init(&ipc);
	strcpy(ipc.send_buf, "start");
	status = ipc_send(&ipc);
	if (status < 0) {
		fprintf(stderr, "Problem with command\n");
		return (-1);
	}
	size = ipc_recv(&ipc);
	if (size < 0) {
		fprintf(stderr, "Problem with start sniffing\n");
		return (-1);
	}
	printf("%s\n", ipc.recv_buf);
	ipc_free(&ipc);
	return (0);
}

int	cli_stop(int argc, char *argv[])
{
	t_ipc	ipc;
	int		status;
	int		size;

	ipc_client_init(&ipc);
	strcpy(ipc.send_buf, "stop");
	status = ipc_send(&ipc);
	if (status < 0) {
		fprintf(stderr, "Problem with command\n");
		return (-1);
	}	
	size = ipc_recv(&ipc);
	if (size < 0) {
		fprintf(stderr, "Problem with stop sniffing\n");
		return (-1);
	}
	printf("%s\n", ipc.recv_buf);
	ipc_free(&ipc);
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
int	cli_show_ip_count(int argc, char *argv[])
{
	t_ipc			ipc;
	int				status;
	int				size;
	unsigned int	ip_count;

	status = _check_ip_prototype(argv[2]);
	if (status < 0) {
		fprintf(stderr, "Error: Wrong ip format\n");
	}
	ipc_client_init(&ipc);
	strcpy(ipc.send_buf, "show_ip_count");
	status = ipc_send(&ipc);
	if (status < 0) {
		fprintf(stderr, "Problem with command\n");
		return (-1);
	}
	strcpy(ipc.send_buf, argv[2]);
	status = ipc_send(&ipc);
	if (status < 0) {
		fprintf(stderr, "Problem with command\n");
		return (-1);
	}
	size = ipc_recv(&ipc); 
	if (size < 0 || size != sizeof(unsigned int)) {
		fprintf(stderr, "Problem with command\n");
		return (-1);
	}
	ip_count = *(unsigned int *)ipc.recv_buf;
	printf("IP: %s | count: %u\n", ipc.send_buf, ip_count);
	ipc_free(&ipc);
	return (status);
}

int	cli_show_ifaces(int argc, char *argv[])
{
	// t_ipc	ipc;
	// int		status;
	// int		size;

	// ipc_client_init(&ipc);
	// strcpy(ipc.send_buf, "show_ifaces");
	// status = ipc_send(&ipc);
	// if (status < 0) {
	// 	fprintf(stderr, "Problem with command\n");
	// 	return (-1);
	// }	
	// size = ipc_recv(&ipc);
	// if (size < 0) {
	// 	fprintf(stderr, "Problem with stop sniffing\n");
	// 	return (-1);
	// }
	// printf("%s\n", ipc->recv_buf);
	// ipc_free(&ipc);
	return (0);
}

int	cli_select_iface(int argc, char *argv[])
{
	t_ipc	ipc;
	int		status;
	int		size;

	ipc_client_init(&ipc);
	strcpy(ipc.send_buf, "select_iface");
	status = ipc_send(&ipc);
	if (status < 0) {
		fprintf(stderr, "Problem with command\n");
		return (-1);
	}
	strcpy(ipc.send_buf, argv[3]);
	status = ipc_send(&ipc);
	if (status < 0) {
		fprintf(stderr, "Problem with command\n");
		return (-1);
	}
	size = ipc_recv(&ipc);
	if (size < 0) {
		fprintf(stderr, "Problem with command\n");
		return (-1);
	}
	printf("%s\n", ipc.recv_buf);
	ipc_free(&ipc);
	return (0);
}

int	cli_stat_iface(int argc, char *argv[])
{
	// t_ipc	ipc;
	// int		status;
	// int		size;

	// ipc_client_init(&ipc);
	// strcpy(ipc.send_buf, "select_iface");
	// status = ipc_send(&ipc);
	// if (status < 0) {
	// 	fprintf(stderr, "Problem with command\n");
	// 	return (-1);
	// }
	// strcpy(ipc.send_buf, argv[3]);
	// status = ipc_send(&ipc);
	// if (status < 0) {
	// 	fprintf(stderr, "Problem with command\n");
	// 	return (-1);
	// }
	// size = ipc_recv(&ipc);
	// if (size < 0) {
	// 	fprintf(stderr, "Problem with command\n");
	// 	return (-1);
	// }
	// printf("%s\n", ipc->recv_buf);
	// ipc_free(&ipc);
	return (0);	
}