#ifndef CLI_H
#define CLI_H

int	cli_help(int argc, char *argv[]);
int	cli_start(int argc, char *argv[]);
int	cli_stop(int argc, char *argv[]);
int	cli_show_ip_count(int argc, char *argv[]);
int	cli_select_iface(int argc, char *argv[]);
int	cli_stat_iface(int argc, char *argv[]);
int	cli_handle_command(int argc, char *argv[]);

#endif