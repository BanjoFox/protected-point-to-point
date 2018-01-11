/**
 * \file p3kshare.h
 * <h3>Protected Point to Point product base header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3k_SHARE_H
#define _p3k_SHARE_H

#include "p3kbase.h"

/*****  CONSTANTS  *****/

/*****  MACROS  *****/

/*****  DATA DEFINITIONS  *****/

/**
 * ioctl_cmd defines the format of the ioctl data passed between the user and kernel space
 * where the command meanings are:
 * - noop: Unused
 * - primarycfg: Configuration data for a P3 primary system
 * - secondarycfg: Configuration data for a P3 secondary system
 * - prihostcfg: Configuration data for a remote P3 primary system
 * - sechostcfg: Configuration data for a remote P3 secondary system
 * - newsession: Data for starting a new P3 session
 */
enum ioctl_cmd {noop, primarycfg, secondarycfg, primaryhostcfg, secondaryhostcfg, newsession};

typedef struct _p3primarycfg p3primarycfg;
typedef struct _p3secondarycfg p3secondarycfg;
typedef struct _p3prihostcfg p3prihostcfg;
typedef struct _p3sechostcfg p3sechostcfg;
typedef struct _p3subnetcfg p3subnetcfg;
typedef struct _p3newsession p3newsession;

/**
 * Structure:
 * p3primarycfg
 *
 * \par Description:
 * The configuration data for a local primary host.
 */

struct _p3primarycfg {
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} addr;						/*<< Host address for secondary connection */
// TODO: Build list of local subnets
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} subnet;					/*<< Local subnet */
	int				port;		/*<< Primary listener port */
	int				array_size;	/**< Default number of keys in key arrays */
	int				rk_wait;	/**< Default rekey period in seconds */
	int				datidx_wait; /**< Default data array index period (secs) */
	int				ctlidx_wait; /**< Default control array index period (secs) */
	int				hb_wait;	/**< Default heartbeat period in seconds */
	int				hb_fail;	/**< Default heartbeat fail time in seconds */
	unsigned int	flag;
	// reserve p3HST_IPV4	0x00100000	Host address is IPv4
	// reserve p3HST_IPV6	0x00200000	Host address is IPv6
};

/**
 * Structure:
 * p3secondarycfg
 *
 * \par Description:
 * The configuration data for a local secondary host.
 */

struct _p3secondarycfg {
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} addr;						/*<< Host address that communicates with primaries */
// TODO: Build list of local subnets
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} subnet;					/*<< Local subnet */
	unsigned int	flag;
	// reserve p3HST_IPV4	0x00100000	Host address is IPv4
	// reserve p3HST_IPV6	0x00200000	Host address is IPv6
};

/**
 * Structure:
 * p3prihostcfg
 *
 * \par Description:
 * The configuration data for a remote primary host.
 */

struct _p3prihostcfg {
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} addr;						/*<< Host address (IPv4 or IPv6) */
	p3subnetcfg		*subnets;	/*<< List of subnets */
	int				subnetsz;	/*<< Number of subnet structures following */
	int				port;		/*<< Primary listener port */
	unsigned int	flag;
// reserve p3HST_ID		0x000fffff	Host ID
// reserve p3HST_IPV4	0x00100000	Host address is IPv4
// reserve p3HST_IPV6	0x00200000	Host address is IPv6
};

/**
 * Structure:
 * p3sechostcfg
 *
 * \par Description:
 * The configuration data for a remote secondary host.
 */

struct _p3sechostcfg {
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} addr;						/*<< Host address (IPv4 or IPv6) */
	p3subnetcfg		*subnets;	/*<< List of subnets */
	int				subnetsz;	/*<< Number of subnet structures following */
	int				rk_wait;	/*<< Rekey period in seconds */
	int				hb_wait;	/*<< Period between heartbeats in seconds */
	int				hb_fail;	/*<< Length of heartbeat failure in seconds */
	int				ditime;		/*<< Period to rekey data from list */
	int				citime;		/*<< Period to rekey control from list */
	unsigned int	flag;
// reserve p3HST_ID		0x000fffff	Host ID
// reserve p3HST_IPV4	0x00100000	Host address is IPv4
// reserve p3HST_IPV6	0x00200000	Host address is IPv6
// reserve p3HST_ARRAY	0x00400000	Key arrays allowed
// reserve p3HST_KTYPE	0x0f000000	Key type field
// reserve p3HST_KTSHF	24			Field shift amount
};

/**
 * Structure:
 * p3subnetcfg
 *
 * \par Description:
 * The configuration data for a remote host subnet.
 */

struct _p3subnetcfg {
	p3subnetcfg			*next;	/*<< List pointer */
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} net;						/*<< Network address (IPv4 or IPv6) */
	struct in_addr		mask;	/*<< Network mask (IPv4) */
	int					id;		/*<< Subnet ID under remote host */
};

/**
 * Structure:
 * p3newsession
 *
 * \par Description:
 * The information for new session with a remote host.
 */

struct _p3newsession {
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} addr;							/*<< Network address (IPv4 or IPv6) */
	int					flag;
// reserve p3IP_TYPE	0x30000000	/* IP version field */
	char				datakey[p3MAX_KSIZE];	/**<< Data key value */
	char				ctlkey[p3MAX_KSIZE];	/**<< Control key value */
};

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _p3_PRIMARY_C
  #ifndef _p3_SECONDARY_C
    #ifndef _p3_PRIMARYPLUS_C
extern enum ioctl_cmd iocmd;
    #endif
  #endif
#endif

#endif /* _p3k_SHARE_H */

