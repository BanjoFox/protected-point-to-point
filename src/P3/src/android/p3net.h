/**
 * \file p3net.h
 * <h3>Protected Point to Point network header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3NET_H
#define _p3NET_H

/*****  INCLUDE FILES *****/

#include "p3base.h"
#include <netinet/ip.h>
#include <netinet/in6.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/*****  CONSTANTS  *****/

/*****  DATA DEFINITIONS  *****/

typedef struct _p3host p3host;
typedef struct _p3net p3net;

/**
 * Structure:
 * p3host
 * 
 * \par Description:
 * The definition for a remote secondary host.
 */

struct _p3host {
	p3host				*next;	/*<< List pointer */
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} addr;						/*<< Host address (IPv4 or IPv6) */
	p3net			*net;		/*<< Remote network list */
#ifndef _p3_PRIMARY
	int				rekey;		/*<< Period to initiate rekeying in seconds */
	int				hb_wait;	/*<< Period between heartbeats in seconds */
	int				hb_fail;	/*<< Length of heartbeat failure in seconds */
#endif
	unsigned int	flag;
#define p3HST_ID	0x000fffff	/* Host ID */
#define p3HST_IPV4	0x00100000	/* Host address is IPv4 */
#define p3HST_IPV6	0x00200000	/* Host address is IPv6 */
#define p3HST_ARRAY	0x00400000	/* Key arrays allowed */
#define p3HST_KTYPE	0x0f000000	/* Key type field */
#define p3HST_KTSHF	24			/* Field shift amount */
};

/**
 * Macro:
 * p3GET_HOST_KEYTYPE
 *
 * \par Description:
 * Get the remote host key type.
 *
 * \par Parameters:
 * - flag: The flag field in the remote host structure.
 */
#define p3GET_HOST_KEYTYPE(flag) \
	((flag >> p3HST_KTSHF) & 0x0f)

/**
 * Macro:
 * p3SET_HOST_KEYTYPE
 *
 * \par Description:
 * Set the remote host key type.
 *
 * \par Parameters:
 * - flag: The flag field in the remote host structure.
 * - value: The key type value to be set.
 */
#define p3SET_HOST_KEYTYPE(flag, value) \
	flag &= ~(0x0f << p3HST_KTSHF); \
	flag |= value << p3HST_KTSHF;

/**
 * Structure:
 * p3net
 * 
 * \par Description:
 * The network information for the P3 routing table.  There are two
 * routing tables, one each for IPv4 and IPv6, so the network type
 * does not need to be defined in the structure.
 */

struct _p3net {
	p3net				*next;	/*<< List pointer */
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} net;						/*<< Network address (IPv4 or IPv6) */
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} mask;						/*<< Network mask (IPv4 or IPv6) */
};

/*****  PROTOTYPES  *****/

#ifndef _p3_SECONDARY
int init_pri_net();
#endif

#ifndef _p3_PRIMARY
int init_sec_net();
#endif

/*****  EXTERNAL DEFINITIONS  *****/

#endif /* _p3NET_H */
