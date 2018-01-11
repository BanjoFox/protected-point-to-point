/**
 * \file p3kprimary.h
 * <h3>Protected Point to Point product primary main functions header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3k_PRIMARY_H
#define _p3k_PRIMARY_H

/*****  INCLUDE FILES *****/

#include "p3kbase.h"
#include "p3knet.h"

/*****  CONSTANTS  *****/

#define p3MIN_IOCTL_SIZE	4

#define p3IOCMD_PID			"PID"

/*****  DATA DEFINITIONS  *****/

typedef struct _p3pri_main p3pri_main;
typedef struct _p3config p3config;

/**
 * Structure:
 * p3pri_main
 * 
 * \par Description:
 * The main primary structure which contains basic application data
 * as well as pointers to operational data.
 */

struct _p3pri_main {
	p3host				*timer;		/**< Timer queue */
	p3host				*tltimer;	/**< Timer queue tail */
	p3key_mgr			key_mgr;	/**< Key mangagement information */
	int					array_size;	/**< Default number of keys in key arrays */
#define p3RMT_ARSZ	256				/* Default */
	p3cluster			*cluster;	/**< Clustering and failover definitions */
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} addr;							/*<< Local host address (IPv4 or IPv6) */
// TODO: Create list of local subnets
	p3net				*subnet;	/*<< Local subnet information */
	int					port;		/*<< Primary listener port */
// reserve p3PRI_PORT	5653		/* Default */
	unsigned int		flag;
#define p3PRI_NINFO	0x00000001		/**< Ignore informational messages */
// reserve p3HST_IPV4	0x00100000	/* Host address is IPv4 */
// reserve p3HST_IPV6	0x00200000	/* Host address is IPv6 */
};

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

extern int init_primary (unsigned char *ramdisk);
extern int parse_p3cmd(unsigned char *buffer, int size);
int parse_p3data(unsigned char *buffer, int size);
void start_rekeying(p3session *session);

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _p3_PRIMARY_C
extern p3pri_main *primain;
#endif

#endif /* _p3k_PRIMARY_H */
