#ifndef NSTAT_H
#define NSTAT_H

#define STAT_FILE_NAME "stat.txt"

typedef struct s_nstat	t_nstat;

typedef enum	e_ip_add_type
{
	INCOMING_IP,
	UPCOMING_IP
}				t_ip_add_type;

int		nstat_init(t_nstat **nstat_ptr, char *file_name);
int		nstat_add_ip(t_nstat *nstat, char *ip_addr, t_ip_add_type type);
int		nstat_save_stat_to_file(t_nstat *nstat, char *file_name);
int		nstat_load_stat_from_file(t_nstat *nstat, char *file_name);
int		nstat_free(t_nstat *nstat);
void	nstat_print(t_nstat *nstat);
int		nstat_lookup_ip_times(t_nstat *nstat, char *ip_addr, unsigned int times[2]);

#endif