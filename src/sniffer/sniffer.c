#include "sniffer.h"
#include "sniffer_private.h"
#include <syslog.h>
#include <pcap.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

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
        // syslog(LOG_ERR, "%s", errbuf);
        printf("%s", errbuf);
        exit(EXIT_FAILURE);
    }

    if (alldevsp == NULL) {
        // syslog(LOG_DEBUG, "%s", "No availiable devices\n");
        printf("%s", "No availiable devices\n");
        exit(EXIT_SUCCESS);
    }
    dev = alldevsp->name;

    // syslog(LOG_DEBUG, "dev name: %s", dev);
    printf("dev name: %s", dev);
    handle = pcap_create(dev, errbuf);
    if (handle == NULL) {
        // syslog(LOG_ERR, "%s", errbuf);
        printf("%s", errbuf);
        exit(EXIT_FAILURE);
    }

    // pcap_set_rfmon(handle, 1);
    // pcap_set_promisc(handle, 1); /* Capture packets that are not yours */
    pcap_set_snaplen(handle, SNAP_LEN); /* Snapshot length */
    pcap_set_timeout(handle, PACKET_TIMEOUT); /* Timeout in milliseconds */
    status = pcap_activate(handle);
    if (status != 0) {
        printf("%s", pcap_geterr(handle));
        exit(EXIT_FAILURE);
    }

    sniffer->interface_idx = 0;
    sniffer->alldevsp = alldevsp;
    sniffer->dev[sniffer->interface_idx] = dev;
    sniffer->handle[sniffer->interface_idx] = handle;
    sniffer->num_active_interfaces = 1;

    nstat_init(&sniffer->nstat);
    // syslog(LOG_DEBUG, "%s", "Sniffer is ready");
    printf("%s", "Sniffer is ready");

    // int link_type;
    // link_type = pcap_datalink(handle);
    // printf("%d\n", link_type);
    // printf("%d\n", PCAP_ERROR_NOT_ACTIVATED);
}

int    sniffer_start(t_sniffer *sniffer)
{
    int status;
    // syslog(LOG_DEBUG, "%s", "Sniffer start");
    printf("%s", "Sniffer start");
    status = pcap_loop(sniffer->handle[sniffer->interface_idx], 10, _packet_handler, (u_char *)sniffer);
    if (status != 0) {
        printf("%s\n", pcap_geterr(sniffer->handle[sniffer->interface_idx]));
        return (-1);
    }
    nstat_print(sniffer->nstat);
    nstat_save_stat_to_file(sniffer->nstat, STAT_FILE_NAME);

    // syslog(LOG_DEBUG, "%s", "Sniffer end");
    printf("%s", "Sniffer end");
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
    const struct sniff_ethernet *ethernet;
    const struct sniff_ip       *ip;
    int                         size_ip;
    t_sniffer                   *sniffer;
    char                        *ip_addr;

    sniffer = (t_sniffer *)args;

    count++;
    // syslog(LOG_INFO, "Packet number %d:\n", count);
    printf("Packet number %d:\n", count);
    
    ethernet = (struct sniff_ethernet *)(packet);
    
    ip = (struct sniff_ip *)(packet + SIZE_ETHERNET);
    size_ip = IP_HL(ip)*4;
    if (size_ip < 20) {
        // syslog(LOG_ERR, "Invalid IP header length: %u bytes\n", size_ip);
        printf("Invalid IP header length: %u bytes\n", size_ip);
        return ;
    }

    ip_addr = inet_ntoa(ip->ip_src);
    // syslog(LOG_INFO, "From: %s\n", ip_addr);
    printf("From: %s\n", ip_addr);
    nstat_add_ip(sniffer->nstat, ip_addr, INCOMING_IP);

    ip_addr = inet_ntoa(ip->ip_dst);
    // syslog(LOG_INFO, "To: %s\n", ip_addr);
    printf("To: %s\n", ip_addr);
    nstat_add_ip(sniffer->nstat, ip_addr, UPCOMING_IP);

    return ;
}

char            **sniffer_get_avaliable_interfaces(t_sniffer *sniffer)
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