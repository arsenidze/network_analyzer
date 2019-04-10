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

int    sniffer_init(t_sniffer *sniffer)
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
        return (-1);
    }

    if (alldevsp == NULL) {
        syslog(LOG_DEBUG, "%s", "No availiable devices\n");
        return (-1);
    }
    dev = alldevsp->name;

    syslog(LOG_DEBUG, "dev name: %s", dev);

    status = _pcap_handle_init(&handle, dev, errbuf);
    if (status < 0) {
        syslog(LOG_ERR, "%s", errbuf);
        return (-1);
    }

    sniffer->interface_idx = 0;
    sniffer->alldevsp = alldevsp;
    sniffer->devs[sniffer->interface_idx] = dev;
    sniffer->handles[sniffer->interface_idx] = handle;
    sniffer->num_active_interfaces = 1;

    status = nstat_init(&sniffer->nstats[sniffer->interface_idx], 
        nstat_get_interface_stat_file_name(dev));
    if (status < 0) {
        pcap_close(handle);
        syslog(LOG_ERR, "%s", "Failed to init interface statistic");
        return (-1);
    }
    syslog(LOG_DEBUG, "%s", "Sniffer is ready");

    // int link_type;
    // link_type = pcap_datalink(handle);
    // printf("%d\n", link_type);
    // printf("%d\n", PCAP_ERROR_NOT_ACTIVATED);
    return (0);
}

int    sniffer_sniff(t_sniffer *sniffer)
{
    int                 ret;
    struct pcap_pkthdr  *header;
    const u_char        *packet;

    ret = pcap_next_ex(sniffer->handles[sniffer->interface_idx], &header, &packet);
    if (ret <= 0) {
        return (-1);
    }
    _packet_handler((u_char *)sniffer, header, packet);
    return (0);
}

void    sniffer_free(t_sniffer *sniffer)
{
    for (int i = 0; i < sniffer->num_active_interfaces; ++i) {
        pcap_close(sniffer->handles[i]);
    }
    pcap_freealldevs(sniffer->alldevsp);
    nstat_free(sniffer->nstats[sniffer->interface_idx]);
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

    nstat_increase_num_of_packets(sniffer->nstats[sniffer->interface_idx], 1);
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
    status = nstat_add_ip(sniffer->nstats[sniffer->interface_idx], ip_addr, INCOMING_IP);
    if (status < 0) {
        syslog(LOG_ERR, "Error with adding packet to stat");
    }
    status = nstat_add_ip(sniffer->nstats[sniffer->interface_idx], ip_addr, UPCOMING_IP);
    if (status < 0) {
        syslog(LOG_ERR, "Error with adding packet to stat");
    }

    if (count != 0 && count % 10 == 0) {
        nstat_save_stat_to_file(sniffer->nstats[sniffer->interface_idx], NULL);
    }

    return ;
}

int             sniffer_try_set_interface(t_sniffer *sniffer, char *interface, char *errbuf)
{
    pcap_if_t   *d;
    pcap_t      *handle;
    int         status;
    int         prev_interface_idx;

    for (int i = 0; i < sniffer->num_active_interfaces; ++i)
    {
        if (strcmp(interface, sniffer->devs[i]) == 0) {
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

    prev_interface_idx = sniffer->interface_idx;
    sniffer->interface_idx = sniffer->num_active_interfaces;
    status = nstat_init(&sniffer->nstats[sniffer->interface_idx], 
        nstat_get_interface_stat_file_name(d->name));
    if (status < 0) {
        pcap_close(handle);
        sniffer->interface_idx = prev_interface_idx;
        syslog(LOG_ERR, "%s", "Failed to init interface statistic");
        return (-1);
    }
    sniffer->handles[sniffer->interface_idx] = handle;
    sniffer->devs[sniffer->interface_idx] = d->name;
    sniffer->num_active_interfaces++;

    return (0);
}

char            *sniffer_get_interface_stat(t_sniffer *sniffer, char *interface)
{
    int     i;
    char    *ret;
    int     len;

    for (i = 0; i < sniffer->num_active_interfaces; ++i)
    {
        if (strcmp(interface, sniffer->devs[i]) == 0) {
            break ;
        }
    }
    if (i == sniffer->num_active_interfaces) {
        return (NULL);
    }
    ret = malloc(sizeof(char) * ((6 + 10 + 3 + 19 + 10 + 3 + 33 + 10) + 1));
    if (ret == NULL) {
        return (NULL);
    }
    strcpy(ret, "name: ");  //6
    strcat(ret, sniffer->devs[i]); //10
    strcat(ret, " | "); //3
    strcat(ret, "number of packets: "); //19
    len = strlen(ret);
    sprintf(ret + len, "%u", nstat_get_num_of_packets(sniffer->nstats[i])); //10
    strcat(ret, " | "); //3
    strcat(ret, "number of captured ip addresses: "); //33
    len = strlen(ret);
    sprintf(ret + len, "%u", nstat_get_num_of_ips(sniffer->nstats[i])); //10
    return (ret);
}

char            **sniffer_get_all_stat(t_sniffer *sniffer)
{
    char    **all_stat;

    all_stat = malloc(sizeof(char *) * (sniffer->num_active_interfaces + 1));
    if (all_stat == NULL) {
        return (NULL);
    }
    for (int i = 0; i < sniffer->num_active_interfaces; ++i)
    {
        all_stat[i] = sniffer_get_interface_stat(sniffer, sniffer->devs[i]);
        if (all_stat[i] == NULL) {
            all_stat[i] = "Problem with getting interface info";
        }
    }
    return (all_stat);
}