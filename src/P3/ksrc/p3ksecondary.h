/**
 * \file p3ksecondary.h
 * <h3>Protected Point to Point product secondary main functions header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3k_SECONDARY_H
#define _p3k_SECONDARY_H

/*****  INCLUDE FILES *****/

#include "p3kbase.h"
#include "p3knet.h"

/*****  CONSTANTS  *****/

/*****  DATA DEFINITIONS  *****/

typedef struct _p3sec_main p3sec_main;

/**
 * Structure:
 * p3sec_main
 * 
 * \par Description:
 * The main secondary structure which contains basic application data
 * as well as pointers to operational data.
 */

struct _p3sec_main {
	p3host			*timer;		/**< Timer queue */
	p3route			*route;		/**< P3 Route table */
	// Local host definitions
	p3cluster		*cluster;	/**< Clustering and failover definitions */
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} addr;							/*<< Local host address (IPv4 or IPv6) */
// TODO: Create list of local subnets
	p3net			*subnet;		/*<< Local subnet information */
	unsigned int	flag;
#define p3SEC_FLG1	0x00000001
// reserve p3HST_IPV4	0x00100000	/* Host address is IPv4 */
// reserve p3HST_IPV6	0x00200000	/* Host address is IPv6 */
};

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

extern int init_secondary (void);
int parse_p3cmd(unsigned char *buffer, int size);
extern int parse_p3data(unsigned char *buffer, int size);

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _p3_SECONDARY_C
extern p3sec_main *secmain;
#endif

#endif /* _p3k_SECONDARY_H */
