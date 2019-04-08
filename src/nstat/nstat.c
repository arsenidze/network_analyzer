#include "nstat_private.h"
#include "nstat.h"
#include "libtree.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int cmp_ip_fn(const struct avltree_node *a, const struct avltree_node *b)
{
	t_storage_node *p = avltree_container_of(a, t_storage_node, node);
	t_storage_node *q = avltree_container_of(b, t_storage_node, node);

	return (strcmp(p->ip_addr, q->ip_addr));
}

int	nstat_init(t_nstat **nstat_ptr)
{
	*nstat_ptr = malloc(sizeof(t_nstat));
	if (*nstat_ptr == NULL) {
		return (-1);
	}
	avltree_init(&(*nstat_ptr)->ip_storage, cmp_ip_fn, 0);
	(*nstat_ptr)->num_ips = 0;
	return (0);
}

int	nstat_add_ip(t_nstat *nstat, char *ip_addr, t_ip_add_type type)
{
	t_storage_node		*storage_node;
	t_storage_node		*existing_node;
	struct avltree_node	*ret;

	storage_node = malloc(sizeof(t_storage_node));
	if (storage_node == NULL) {
		return (-1);
	}
	memset(storage_node, 0, sizeof(t_storage_node));

	// strncpy(node.ip_addr, ip_addr, IP_LEN);
	strcpy(storage_node->ip_addr, ip_addr);
	if (type == INCOMING_IP) {
		storage_node->incoming_times++;
	}
	else if (type == UPCOMING_IP) {
		storage_node->upcoming_times++;
	}
	else {
		return (-1);
	}
	ret = avltree_insert(&storage_node->node, &nstat->ip_storage);
	if (ret != NULL) {
		existing_node = avltree_container_of(ret, t_storage_node, node);
		if (type == INCOMING_IP) {
			existing_node->incoming_times++;
		} 
		else if (type == UPCOMING_IP) {
			existing_node->upcoming_times++;
		}
		else {
			return (-1);
		}
	}
	else {
		nstat->num_ips++;
	}
	return (0);
}

#define MAX_BYTES_FOR_UINT 10
#define MAX_NUM_CHARS_FOR_IP_RECORD (IP_LEN + 2 * MAX_BYTES_FOR_UINT + 2 + 1)
//ip record format:	255.255.255.255 {incoming_times} {upcoming_times}\n

void	_storage_node_to_str(t_storage_node *storage_node, char *str)
{
	int		offset;
	char	buf[MAX_BYTES_FOR_UINT + 1];
	int		len;

	//Add ip address
	len = strlen(storage_node->ip_addr);
	strncpy(str, storage_node->ip_addr, len);
	offset = len;
	str[offset++] = ' ';

	//Add incoming times
	memset(buf, ' ', MAX_BYTES_FOR_UINT);
	sprintf(buf, "%u", storage_node->incoming_times);
	strncpy(str + offset, buf, MAX_BYTES_FOR_UINT);
	offset += MAX_BYTES_FOR_UINT;
	str[offset++] = ' ';

	//Add upcoming times
	memset(buf, ' ', MAX_BYTES_FOR_UINT);
	sprintf(buf, "%u", storage_node->upcoming_times);
	strncpy(str + offset, buf, MAX_BYTES_FOR_UINT);
	offset += MAX_BYTES_FOR_UINT;
	str[offset++] = ' ';

	str[offset++] = '\n';
}

int	nstat_save_stat_to_file(t_nstat *nstat, char *file_name)
{
	FILE			*fp;
	char			*stat_in_str;
	int				str_idx;
	char			buf[MAX_BYTES_FOR_UINT + 1];
	int				file_size;
	t_storage_node	*storage_node;

	fp = fopen(file_name, "w+b");
	if (fp == NULL) {
		return (-1);
	}
	file_size = MAX_BYTES_FOR_UINT + 1; //For number of ip packets + \n
	file_size += nstat->num_ips * (MAX_NUM_CHARS_FOR_IP_RECORD + 1); //For all ip records with \n
	stat_in_str = malloc(sizeof(char) * (file_size + 1));
	if (stat_in_str == NULL) {
		fclose(fp);
		return (-1);
	}
	stat_in_str[file_size] = '\0';

	//Add number of packets
	memset(buf, ' ', MAX_BYTES_FOR_UINT);
	sprintf(buf, "%u", nstat->num_ips);
	str_idx = strlen(buf);
	strncpy(stat_in_str, buf, str_idx);
	stat_in_str[str_idx++] = '\n';

	for (struct avltree_node *next = avltree_first(&nstat->ip_storage); next != NULL; next = avltree_next(next)) {
		storage_node = avltree_container_of(next, t_storage_node, node);
		_storage_node_to_str(storage_node, &stat_in_str[str_idx]);
		str_idx += MAX_NUM_CHARS_FOR_IP_RECORD;
		printf("qqqqqq\n");
	}
	printf("%s\n", stat_in_str);
	fwrite(stat_in_str, str_idx, sizeof(char), fp);
	free(stat_in_str);
	fclose(fp);
	return (0);
}

void	_str_to_storage_node(char *str, t_storage_node *storage_node)
{
	int		offset;
	char	*endptr;

	strncpy(storage_node->ip_addr, str, IP_LEN);
	offset = IP_LEN;
	offset += 1;
	storage_node->incoming_times = strtoul(str + offset, &endptr, 10);
	offset += MAX_BYTES_FOR_UINT;
	offset += 1;
	storage_node->upcoming_times = strtoul(str + offset, &endptr, 10);
}

int	nstat_load_stat_from_file(t_nstat *nstat, char *file_name)
{
	FILE			*fp;
	char			*stat_in_str;
	int				str_idx;
	// char			buf[MAX_BYTES_FOR_UINT + 1];
	int				stat_size;
	t_storage_node	*storage_nodes;

	fp = fopen(file_name, "rb");
	if (fp == NULL) {
		return (-1);
	}

	fscanf(fp, "%u", &nstat->num_ips);

	stat_size = nstat->num_ips * MAX_NUM_CHARS_FOR_IP_RECORD;

	stat_in_str = malloc(sizeof(char) * (stat_size + 1));
	if (stat_in_str == NULL) {
		fclose(fp);
		return (-1);
	}
	stat_in_str[stat_size] = '\0';

	fread(stat_in_str, stat_size, sizeof(char), fp);
	fclose(fp);

	storage_nodes = malloc(sizeof(t_storage_node) * nstat->num_ips);
	if (storage_nodes == NULL) {
		free(stat_in_str);
		return (-1);
	}

	memset(storage_nodes, 0, sizeof(t_storage_node) * nstat->num_ips);

	str_idx = 0;
	for (int i = 0; i < nstat->num_ips; i++) {
		_str_to_storage_node(&stat_in_str[str_idx], &storage_nodes[i]);
		str_idx += MAX_NUM_CHARS_FOR_IP_RECORD;
		avltree_insert(&storage_nodes[i].node, &nstat->ip_storage);
	}

	free(stat_in_str);
	return (0);	
}

int	nstat_free(t_nstat *nstat) 
{
	free(nstat);
	return (0);
}

void nstat_print(t_nstat *nstat)
{
	t_storage_node	*storage_node;

	for (struct avltree_node *next = avltree_first(&nstat->ip_storage); next != NULL; next = avltree_next(next)) {
		storage_node = avltree_container_of(next, t_storage_node, node);
		printf("%s %u %u\n", storage_node->ip_addr, storage_node->incoming_times, storage_node->upcoming_times);
	}
}