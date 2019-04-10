#ifndef SNIFFER_H
#define SNIFFER_H

#include <pcap.h>
#include "nstat.h"

#define MAX_NUM_INTERFACES	30

typedef struct	s_sniffer
{
	pcap_if_t	*alldevsp;
	t_nstat		*nstats[MAX_NUM_INTERFACES];
	pcap_t		*handles[MAX_NUM_INTERFACES];
	char		*devs[MAX_NUM_INTERFACES];
	int			interface_idx;
	int			num_active_interfaces;
	int			is_active;
}				t_sniffer;

int				sniffer_init(t_sniffer *sniffer);
int				sniffer_sniff(t_sniffer *sniffer);
void			sniffer_free(t_sniffer *sniffer);
int				sniffer_try_set_interface(t_sniffer *sniffer, char *interface, char *errbuf);
char			*sniffer_get_interface_stat(t_sniffer *sniffer, char *interface);
char			**sniffer_get_all_stat(t_sniffer *sniffer);

#endif