/**
 * \file p3linux.h
 * <h3>Protected Point to Point Linux header file</h3>
 *
 * Copyright (C) Velocite 2010
 */

#ifndef _p3k_LINUX_H
#define _p3k_LINUX_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/slab.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <linux/mm.h>
#include <linux/spinlock.h>
#include <linux/time.h>

#include <net/ip.h>
#include <net/ipv6.h>
#include <net/tcp.h>
#include <net/protocol.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

#include <net/route.h>

#define P3DEVNAME "p3dev"
#define P3IOC_TYPE 'p'

/**
 * The Linux kernel changes frequently, so the support for these changes
 * is maintained in macro definitions.  The p3LINUXVER is set by removing
 * the periods from the version.release.subrelease of the target kernel.
 * Thus, 2.6.24 = 2624.
 *
 * Note that to add support for kernel versions, the "if p3LINUXVER < version"
 * statements must be added in ascending order.
 */
#define p3LINUXVER 2631

  #if p3LINUXVER < 2624
#define SKBP				(*skb)
#define p3SKB_DST			dst
#define p3SKB_HDRLEN		hdr_len
#define p3SKB_MACHDR		mac
#define p3SKB_NETHDR		nh
#define p3SKB_TRANSHDR		h
#define p3SKB_DST_GET(skbuff) \
	skbuff->dst
#define p3SKB_DST_SET(skbuff, newdst) \
	skbuff->dst = newdst
#define p3ROUTE_HARDER(skbuff) \
	ip_route_me_harder(&skbuff)
#define p3MAC_HEADER_SET(skbuff) \
	skbuff->dev->hard_header

  #else
#define SKBP				skb
#define p3SKB_DST			_skb_dst
#define p3SKB_HDRLEN		hdr_len
#define p3SKB_MACHDR		mac_header
#define p3SKB_NETHDR		network_header
#define p3SKB_TRANSHDR		transport_header
#define p3SKB_DST_GET(skbuff) \
	skb_dst(skbuff)
#define p3SKB_DST_SET(skbuff, dst) \
	skb_dst_set(skbuff, dst)
#define p3ROUTE_HARDER(skbuff) \
	ip_route_me_harder(skbuff, RTN_UNSPEC)
#define p3MAC_HEADER_SET(skbuff) \
	skbuff->dev->header_ops->create

  #endif /* Version definitions */


  #ifdef _p3_PRIMARY
#define P3APP	"p3primary"
#define RAMDISK_SZ	0x10000
  #else
    #ifdef _p3_SECONDARY
#define P3APP	"p3secondary"
#define RAMDISK_SZ	0x1000
    #else
#define P3APP	"p3primaryplus"
#define RAMDISK_SZ	0x20000
    #endif
  #endif

#ifdef NF_IP_PRE_ROUTING
	#define p3PRE_ROUTING	NF_IP_PRE_ROUTING
	#define p3LOCAL_OUT	NF_IP_LOCAL_OUT
	#define p3FORWARD	NF_IP_FORWARD
	#define p3POST_ROUTING	NF_IP_POST_ROUTING
	#define p3LOCAL_IN	NF_IP_LOCAL_IN
#else
	#define p3PRE_ROUTING	NF_INET_PRE_ROUTING
	#define p3LOCAL_OUT	NF_INET_LOCAL_OUT
	#define p3FORWARD	NF_INET_FORWARD
	#define p3POST_ROUTING	NF_INET_POST_ROUTING
	#define p3LOCAL_IN	NF_INET_LOCAL_IN
#endif

/*****  DATA DEFINITIONS  *****/

typedef spinlock_t	p3lock;		/* The system dependent lock type */
typedef struct _p3netdata p3netdata;

/**
 * Structure:
 * p3netdata
 *
 * \par Description:
 * The operating system dependent network data structures.
 */

struct _p3netdata {
	// Set when p3net_utils called with p3SET_RAW
	struct dst_entry	*p3dst;
	struct sock			*p3sk;
	// Set when p3net_utils called with p3SET_DEVOUT
	struct proto		*p3proto;
	struct net			*p3knet;
	// Set when p3net_utils called with p3SET_DEVIN
	struct net_device	*p3ndev;
	int (*okfn)(struct sk_buff *);
	unsigned int		p3prot;
	int					p3ptype;
	unsigned char		p3locadr[MAX_ADDR_LEN];
	unsigned char		p3remadr[MAX_ADDR_LEN];
};

/*****  MACROS  *****/

/* IP Macros */
#define p3SET_CHECKSUM_V4(iph) \
	iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl)

/* Lock Macros */
#define p3lock_init(lock) \
	spin_lock_init(&lock)

#define p3lock(lock) \
	spin_lock(&lock)

#define p3unlock(lock) \
	spin_unlock(&lock)

MODULE_AUTHOR ("Velocite Systems");
MODULE_DESCRIPTION ("Velocite Systems P3 kernel module");
MODULE_LICENSE ("GPL");
MODULE_ALIAS (P3APP);

extern void p3errmsg(int type, char *message);
extern int p3send_packet(void *pkt);
extern int p3net_utils(int type, void *p3skb, void *pkt);

#endif /* _p3k_LINUX_H */
