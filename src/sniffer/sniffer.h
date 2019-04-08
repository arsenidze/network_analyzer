#ifndef SNIFFER_H
#define SNIFFER_H

#include <pcap.h>
#include "nstat.h"

#define MAX_DEV_NAME 32

typedef struct	s_sniffer
{
	char	dev[MAX_DEV_NAME];
	pcap_t	*handle;
	t_nstat	*nstat;
}				t_sniffer;

void			sniffer_init(t_sniffer *sniffer);
int				sniffer_start(t_sniffer *sniffer);
void			sniffer_free(t_sniffer *sniffer);

#endif