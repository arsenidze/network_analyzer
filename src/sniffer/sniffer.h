#ifndef SNIFFER_H
#define SNIFFER_H

#include <pcap.h>
#include "nstat.h"

#define MAX_NUM_INTERFACES	10

typedef struct	s_sniffer
{
	t_nstat		*nstat;
	pcap_if_t	*alldevsp;
	pcap_t		*handle[MAX_NUM_INTERFACES];
	char		*dev[MAX_NUM_INTERFACES];
	int			interface_idx;
	int			num_active_interfaces;
}				t_sniffer;

void			sniffer_init(t_sniffer *sniffer);
int				sniffer_start(t_sniffer *sniffer);
void			sniffer_free(t_sniffer *sniffer);
char			**sniffer_get_avaliable_interfaces(t_sniffer *sniffer);

#endif