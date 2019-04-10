#ifndef NSTAT_PRIVATE_H
#define NSTAT_PRIVATE_H

#include "libtree.h"

#define IP_LEN 15

#define MAX_STAT_FILE_NAME	64

typedef struct	s_storage_node
{
	char				ip_addr[IP_LEN + 1];
	unsigned int		incoming_times;
	unsigned int		upcoming_times;
	struct avltree_node	node;
}				t_storage_node;

typedef struct	s_nstat
{
	struct avltree		ip_storage;
	unsigned int		num_ips;
	unsigned int		num_packets;
	char				file_name[MAX_STAT_FILE_NAME];
}				t_nstat;

#endif