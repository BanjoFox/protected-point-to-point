/**
 * \file p3system.h
 * <h3>Protected Point to Point product system header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3_SYSTEM_H
#define _p3_SYSTEM_H

#include "p3base.h"
#include "p3share.h"
#include "p3net.h"

/*****  CONSTANTS  *****/

#define p3ADM_HOME		"/etc/p3"
#define p3ADM_CFG		"/etc/p3"
#define p3ADM_MSG		"/var/log/p3"

/*****  MACROS  *****/

/*****  DATA DEFINITIONS  *****/

typedef struct _p3mm_anchor p3mm_anchor;
typedef struct _p3cluster p3cluster;
typedef struct _p3chost p3chost;

/**
 * Structure:
 * p3mm_anchor
 *
 * \par Description:
 * The configuration data sent by the user space application to the kernel module.
 */

struct _p3mm_anchor {
	int				fd;		/**< Ramdisk file descriptor */
	int				sd;		/**< Raw socket descriptor */
	pid_t			pid;	/**< Process ID of user space application */
#ifndef _p3_SECONDARY
/* Used by primary or primary plus secondary */
	p3key_serv		*kserv;	/**< Key server circular buffer management */
#endif
};

/**
 * Structure:
 * p3cluster
 *
 * \par Description:
 * The clustering and failover definitions.
 */

struct _p3cluster {
	p3chost			*chost;		/**< Cluster host list */
	unsigned int	flag;
#define p3CLS_LB_RR	0x00000001	/* Use round robin load balancing */
#define p3CLS_LB_BC	0x00000002	/* Use byte count load balancing */
#define p3CLS_IREC	0x00000004	/* Immediate recovery */
};

/**
 * Structure:
 * p3chost
 *
 * \par Description:
 * The clustering host definitions.
 */

struct _p3chost {
	p3chost			*next;		/**< List pointer */
	unsigned int	flag;
#define p3CHS_PRI	0x00000001	/* Host is primary in cluster */
};

#ifdef _p3_PRIMARY
typedef struct _p3pri_main p3pri_main;

/**
 * Structure:
 * p3pri_main
 *
 * \par Description:
 * The main primary structure which contains basic application data
 * as well as pointers to operational data.
 */

struct _p3pri_main {
	p3mm_anchor			*anchor;	/**< Anchor in mmap buffer */
	char 				*home;		/**< Home directory path */
	char		 		*cfgdir;	/**< Configuration directory path */
	char 				*msgdir;	/**< Messages directory path */
	char 				*config;	/**< Configuration file name */
#define p3PRI_CONFIG	"p3primary.conf"
	FILE				*cfg_data;	/**< Configuration file */
	p3cluster			*cluster;	/**< Clustering and failover definitions */
	p3Network			*net;		/**< Network connection information */
	p3Network			*seclist;	/**< List of secondaries */
	p3host				**subnets;	/**< Local subnet list */
	p3host				*sec_host;	/**< Secondary host list */
	unsigned int		flag;
#define p3MN_NINFO		0x00000001	/* Ignore informational messages */
/* Random number generation flags must be: 1=software, 2=hardware, 3=both */
#define p3MN_RNSW		0x00000002	/* Use software random number generation */
#define p3MN_RNHW		0x00000004	/* Use hardware random number generation */
#define p3MN_RNSHF		1			/* Shift value for random number generation */
#define p3MN_CLENABLE	0x00000010	/* Clustering state enabled */
#define p3MN_CLDBALRR	0x00000020	/* Load balancing = round robin */
#define p3MN_CLDBALBC	0x00000040	/* Load balancing = byte count */
};
#endif

#ifdef _p3_SECONDARY
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
	p3mm_anchor			*anchor;	/**< Anchor in mmap buffer */
	char 				*home;		/**< Home directory path */
	char		 		*cfgdir;	/**< Configuration directory path */
	char 				*msgdir;	/**< Messages directory path */
	char 				*config;	/**< Configuration file name */
#define p3SEC_CONFIG	"p3secondary.conf"
	FILE				*cfg_data;	/**< Configuration file */
	// Local host definitions
	p3cluster			*cluster;	/**< Clustering and failover definitions */
	p3Network			*net;		/**< Network connection information */
	p3host				**subnets;	/**< Local subnet list */
	unsigned int		flag;
#define p3MN_NINFO		0x00000001	/* Ignore informational messages */
#define p3MN_CLENABLE	0x00000002	/* Clustering state enabled */
#define p3MN_CLDBALRR	0x00000004	/* Load balancing = round robin */
#define p3MN_CLDBALBC	0x00000008	/* Load balancing = byte count */
};
#endif

/*****  PROTOTYPES  and  EXTERNAL DEFINITIONS  *****/

int init_system(int argc, char *argv[]);
int parse_cmdline(int argc, char *argv[]);
int parse_config();
int init_config();
int system_handler();

#ifdef _p3_PRIMARY
int update_sechost(p3sechostcfg *sechost);
  #ifndef _p3_SYSTEM_C
extern p3pri_main *p3main;
  #endif
#endif

#ifdef _p3_SECONDARY
int update_prihost(p3prihostcfg *prihost);
  #ifndef _p3_SYSTEM_C
extern p3sec_main *p3main;
  #endif
#endif

#ifndef _p3_PRIMARY_C
  #ifndef _p3_SECONDARY_C
	#ifndef _p3_PRIMARYPLUS_C
extern char *p3buf;
	#endif
  #endif
#endif

#endif /* _p3_SYSTEM_H */
