#include "sniffer.h"
#include "sniffer_private.h"
#include <syslog.h>
#include <pcap.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

static int  _pcap_handle_init(pcap_t **handle, char *dev, char *errbuf)
{
    int     status;

    *handle = pcap_create(dev, errbuf);
    if (*handle == NULL) {
        syslog(LOG_ERR, "%s", errbuf);
        return (-1);
    }

    status = pcap_set_promisc(*handle, 1); /* Capture packets that are not yours */
    if (status != 0) {
        strcpy(errbuf, pcap_geterr(*handle));
        return (-1);
    }
    if (pcap_can_set_rfmon(*handle)) {
        status = pcap_set_rfmon(*handle, 1);
        if (status != 0) {
            strcpy(errbuf, pcap_geterr(*handle));
            return (-1);
        }
    }
    status = pcap_set_snaplen(*handle, SNAP_LEN); /* Snapshot length */
    if (status != 0) {
        strcpy(errbuf, pcap_geterr(*handle));
        return (-1);
    }
    status = pcap_set_timeout(*handle, PACKET_TIMEOUT); /* Timeout in milliseconds */
    if (status != 0) {
        strcpy(errbuf, pcap_geterr(*handle));
        return (-1);
    }
    status = pcap_activate(*handle);
    if (status < 0) {
        strcpy(errbuf, pcap_geterr(*handle));
        return (-1);
    }
    return (0);
}

void    sniffer_init(t_sniffer *sniffer)
{
    char        errbuf[PCAP_ERRBUF_SIZE];
    int         status = 0;
    pcap_if_t   *alldevsp = NULL;
    char        *dev = NULL;
    pcap_t      *handle = NULL;

    memset(errbuf, 0, PCAP_ERRBUF_SIZE);

    status = pcap_findalldevs(&alldevsp, errbuf);
    if (status != 0) {
        syslog(LOG_ERR, "%s", errbuf);
        exit(EXIT_FAILURE);
    }

    if (alldevsp == NULL) {
        syslog(LOG_DEBUG, "%s", "No availiable devices\n");
        exit(EXIT_SUCCESS);
    }
    dev = alldevsp->name;

    syslog(LOG_DEBUG, "dev name: %s", dev);

    status = _pcap_handle_init(&handle, dev, errbuf);
    if (status < 0) {
        syslog(LOG_ERR, "%s", errbuf);
        exit(EXIT_FAILURE);
    }

    sniffer->interface_idx = 0;
    sniffer->alldevsp = alldevsp;
    sniffer->dev[sniffer->interface_idx] = dev;
    sniffer->handle[sniffer->interface_idx] = handle;
    sniffer->num_active_interfaces = 1;

    nstat_init(&sniffer->nstat, STAT_FILE_NAME);
    syslog(LOG_DEBUG, "%s", "Sniffer is ready");

    // int link_type;
    // link_type = pcap_datalink(handle);
    // printf("%d\n", link_type);
    // printf("%d\n", PCAP_ERROR_NOT_ACTIVATED);
}

int    sniffer_sniff(t_sniffer *sniffer)
{
    int                 ret;
    struct pcap_pkthdr  *header;
    const u_char        *packet;

    ret = pcap_next_ex(sniffer->handle[sniffer->interface_idx], &header, &packet);
    if (ret <= 0) {
        return (-1);
    }
    _packet_handler((u_char *)sniffer, header, packet);
    return (0);
}

void    sniffer_free(t_sniffer *sniffer)
{
    for (int i = 0; i < sniffer->num_active_interfaces; ++i) {
        pcap_close(sniffer->handle[i]);
    }
    pcap_freealldevs(sniffer->alldevsp);
    nstat_free(sniffer->nstat);
}

void    _packet_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
    static int                  count = 0;
    const struct sniff_ip       *ip;
    int                         size_ip;
    t_sniffer                   *sniffer;
    char                        *ip_addr;
    int                         status;

    (void)header;
    
    sniffer = (t_sniffer *)args;

    count++;
    syslog(LOG_INFO, "Packet number %d:\n", count);
    
    ip = (struct sniff_ip *)(packet + SIZE_ETHERNET);
    size_ip = IP_HL(ip)*4;
    if (size_ip < 20) {
        syslog(LOG_ERR, "Invalid IP header length: %u bytes\n", size_ip);
        return ;
    }

    ip_addr = inet_ntoa(ip->ip_src);
    syslog(LOG_INFO, "From: %s\n", ip_addr);
    status = nstat_add_ip(sniffer->nstat, ip_addr, INCOMING_IP);
    if (status < 0) {
        syslog(LOG_ERR, "Error with adding packet to stat");
    }
    status = nstat_add_ip(sniffer->nstat, ip_addr, UPCOMING_IP);
    if (status < 0) {
        syslog(LOG_ERR, "Error with adding packet to stat");
    }
    
    // status = nstat_save_stat_to_file(sniffer->nstat, STAT_FILE_NAME);
    // if (status < 0) {
    //     syslog(LOG_ERR, "Error with saving stat to file");
    // }
    // if (status < 0) {
        // syslog(LOG_ERR, "Error with adding packet to stat");
        
    // if (status == 1) { 
    //     //It's new ip
    //     // nstat_add_stat_to_file(sniffer->nstat, STAT_FILE_NAME);
    //     ;
    // }
    // else {
    //     nstat_save_stat_to_file(sniffer->nstat, STAT_FILE_NAME);
    // }

    
    // if (status < 0) {
        // syslog(LOG_ERR, "Error with adding packet to stat");
    // if (status == 1) { 
    //     //It's new ip
    //     // nstat_add_stat_to_file(sniffer->nstat, STAT_FILE_NAME);
    //     ;
    // }
    // else {
    //     nstat_save_stat_to_file(sniffer->nstat, STAT_FILE_NAME);
    // }
    if (count != 0 && count % 10 == 0) {
        nstat_save_stat_to_file(sniffer->nstat, STAT_FILE_NAME);
    }

    return ;
}

char    **sniffer_get_avaliable_interfaces(t_sniffer *sniffer)
{
    char    **interfaces_names;
    int     i;

    interfaces_names = malloc(sizeof(char *) * (MAX_NUM_INTERFACES + 1));
    if (interfaces_names == NULL) {
        syslog(LOG_ERR, "%s", strerror(errno));
        return (NULL);
    }
    i = 0;
    for(pcap_if_t *d = sniffer->alldevsp; d != NULL; d = d->next) {
        for(pcap_addr_t *a = d->addresses; a != NULL; a = a->next) {
            if(a->addr->sa_family == AF_INET) {
                interfaces_names[i++] = d->name;
                break ;
            }
        }
    }
    interfaces_names[i] = NULL;
    return (interfaces_names);
}

int             sniffer_try_set_interface(t_sniffer *sniffer, char *interface, char *errbuf)
{
    pcap_if_t   *d;
    pcap_t      *handle;
    int         status;

    for (int i = 0; i < sniffer->num_active_interfaces; ++i)
    {
        if (strcmp(interface, sniffer->dev[i]) == 0) {
            sniffer->interface_idx = i;
            return (0);
        }
    }

    for(d = sniffer->alldevsp; d != NULL; d = d->next) {
        if (strcmp(interface, d->name) == 0) {
            break ;
        }
    }
    if (d == NULL) {
        strcpy(errbuf, "No such interface");
        syslog(LOG_ERR, "%s\n", errbuf);
        return (-1);
    }
    status = _pcap_handle_init(&handle, d->name, errbuf);
    if (status < 0) {
        syslog(LOG_ERR, "%s\n", errbuf);
        return (-1);
    }

    sniffer->num_active_interfaces++;
    sniffer->interface_idx++;
    sniffer->handle[sniffer->interface_idx] = handle;
    sniffer->dev[sniffer->interface_idx] = d->name;

    return (0);
}