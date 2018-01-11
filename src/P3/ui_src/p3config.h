/**
 * \file p3config.h
 * <h3>Protected Point to Point config header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3_CONFIG_H
#define _p3_CONFIG_H

/*****  INCLUDE FILES *****/

#include "p3utils.h"
#include "p3net.h"
#include <sys/socket.h>
#include <arpa/inet.h>

/*****  CONSTANTS  *****/

#define p3ADM_PORT			12358	/* Default anonymous port */
#define p3ADM_REKEY			3600	/* Default rekeying period (1 hr) */
#define p3ADM_ARRAYSZ		256		/* Default key array size */
#define p3ADM_DATARRAY		86400	/* Default data array period (24 hrs) */
#define p3ADM_CTLARRAY		82800	/* Default control array period (23 hrs) */
#define p3ADM_HRTBT_TIME	60		/* Default hearbeat period (1 min) */
#define p3ADM_HRTBT_FAIL	30		/* Default heartbeat fail (30 sec) */

/** Host configuration flags used for local,  group, and remote definitions */
#define p3LHCFG_KEYALG		0x0000000f	/* Encryption key algorithm */
#define p3LHCFG_AES128		0x00000001	/* Encryption key algorithm = AES 128 */
#define p3LHCFG_AES256		0x00000002	/* Encryption key algorithm = AES 256 */
/*
 * NOTE:  To add an encryption algorithm, set p3LHCFG_KEYALGMAX appropriately.
 *        Then find all instances of 'AES128' in p3 admin.c and p3config.c
 *        and add accordingly.
 */
#define p3LHCFG_KEYALGMAX	3			/* Number of supported algorithms */
#define p3LHCFG_REKEY		0x00000030	/* Rekeying period field */
#define p3LHCFG_REKEYTM		0x00000010	/* Rekeying period in seconds */
#define p3LHCFG_REKEYPKT	0x00000020	/* Rekeying period in packets */
#define p3LHCFG_NUSEARRAY	0x00000080	/* Use key arrays */
#define p3LHCFG_DATA		0x00000300	/* Data array period field */
#define p3LHCFG_DATTM		0x00000100	/* Data array period in seconds */
#define p3LHCFG_DATPKT		0x00000200	/* Data array period in packets */
#define p3LHCFG_CONTROL		0x00000c00	/* Control array period field */
#define p3LHCFG_CTLTM		0x00000400	/* Control array period in seconds */
#define p3LHCFG_CTLPKT		0x00000800	/* Control array period in packets */
#define p3LHCFG_IPVER		0x00300000  /* IP address version field */
//      p3HST_IPV4			0x00100000  /* Host address is IPv4 */
//      p3HST_IPV6			0x00200000  /* Host address is IPv6 */
#define p3LHCFG_UPDATE		0x00400000  /* Configuration data has been modified */
#define p3LHCFG_RESERVE		0xff000000  /* Available for specific structure use */
#define p3LHCFG_DEFAULT \
	(p3LHCFG_AES128 | p3LHCFG_REKEYTM | p3LHCFG_NUSEARRAY | p3LHCFG_DATTM | p3LHCFG_CTLTM)

#define p3CFG_MAXLEN		256			/* Maximum size of configuration string */

#define p3FIFOIN	"s2ufifo"		/* Inbound FIFO name prefix */
#define p3FIFOOUT	"u2sfifo"		/* Outbound FIFO prefix */

/*****  MACROS  *****/

/*****  DATA DEFINITIONS  *****/

typedef struct _p3config p3config;
typedef struct _p3local p3local;
typedef struct _p3group p3group;
typedef struct _p3remote p3remote;
typedef struct _p3subnet p3subnet;

/**
 * Structure:
 * p3config
 *
 * \par Description:
 * The configuration structure maintains information about configuration
 * files, including whether the P3 host is primary or secondary or both.
 */

struct _p3config {
	char				*path;		/**< Configuration files path */
#define p3CFG_PATH		"/etc/p3/"	/* Default configuration path */
	char				*pricfg;	/**< Primary configuration filename */
#define p3CFG_PRIFILE	"primary.config"	/* Default primary filename */
	char				*seccfg;	/**< Secondary configuration filename */
#define p3CFG_SECFILE	"secondary.config"	/* Default secondary filename */
	p3local				*local;		/**< Local host definitions */
	int					groups;		/**< The number of groups defined */
	p3group				*group;		/**< Group definitions */
	p3group				*grptail;	/**< End of group definitions */
	int					remotes;	/**< The number of remotes defined */
	p3remote			*remote;	/**< Remote host definitions */
	p3remote			*rmttail;	/**< End of remote host definitions */
	unsigned int		flag;
#define p3CFG_COMP		0x00000003	/**< Component field */
#define p3CFG_PRI		0x00000001	/**< Configure primary */
#define p3CFG_SEC		0x00000002	/**< Configure secondary */
#define p3CFG_PARMS		0x00000070	/**< Parameter selection field */
#define p3CFG_LOCAL		0x00000010	/**< Parse local host definitions */
#define p3CFG_REMOTE	0x00000020	/**< Parse remote host definitions */
#define p3CFG_GROUP		0x00000040	/**< Parse group definitions */
};

/**
 * Structure:
 * p3local
 *
 * \par Description:
 * The local host structure maintains information about the local host
 * to support anonymous connections.
 */

struct _p3local {
	char				*hostname;	/**< Host name avoids use of DNS */
	char				*addr;		/**< Host address string */
	char				*s2uname;	/**< System to UI FIFO name */
#define p3LOC_S2UNAME	"s2ufifo"
	char				*u2sname;	/**< UI to System FIFO name */
#define p3LOC_U2SNAME	"u2sfifo"
#ifdef _p3_PRIMARY
	int					port;		/**< Anonymous connections port */
	// Defaults
	unsigned int		rekey;		/**< Rekeying period */
	unsigned int		arraysz;	/**< Key array size */
	unsigned int		datarray;	/**< Data array period */
	unsigned int		ctlarray;	/**< Control array period */
	unsigned int		heartbeat;	/**< Heartbeat period */
	unsigned int		heartfail;	/**< Heartbeat failure time */
#endif

#ifdef _p3_SECONDARY
	// Secondary must provide subnets for anonymous connections
	p3subnet			*subnets;	/**< List of local subnets */
#endif
	unsigned int		flag;		/* Use p3HCFG constants */
#define p3LOC_HWGEN		0x01000000	/* Use hardware key generator */
};

#ifdef _p3_PRIMARY
/**
 * Structure:
 * p3group
 *
 * \par Description:
 * The administration group structure is used to maintain
 * group override information.  The groups are maintained
 * in lists, one for each level, as follows:
 * <ol>
 *   <li>Organization</li>
 *   <li>Location</li>
 *   <li>Security Level</li>
 *   <li>Network</li>
 * </ol>
 * The arrays for each level are sorted alphabetically by
 * group name for binary searching.
 */

struct _p3group {
	p3group				*next;		/**< List pointer */
	char				*name;		/**< Group name */
	int					ID;			/**< Group member ID */
	int					level;		/* Group level */
#define p3GRP_ORG		1
#define p3GRP_LOC		2
#define p3GRP_SEC		3
#define p3GRP_NET		4
	unsigned int		rekey;		/**< Rekeying period */
	unsigned int		arraysz;	/**< Key array size */
	unsigned int		datarray;	/**< Data array period */
	unsigned int		ctlarray;	/**< Control array period */
	unsigned int		heartbeat;	/**< Heartbeat period */
	unsigned int		heartfail;	/**< Heartbeat failure time */
	unsigned int		flag;		/* Use p3HCFG constants */
};
#endif

/**
 * Structure:
 * p3remote
 *
 * \par Description:
 * The remote host structure maintains information about
 * remote host definitions.  The groups array is an index
 * into the group lists.  See the description of these in
 * the p3group structure.  If a value is -1, then the remote
 * definition is not a member of any group at that level.
 */

struct _p3remote {
	p3remote			*next;		/**< List pointer */
	char				*hostname;	/**< Host name avoids use of DNS */
	char				*addr;		/**< Host address string */
	p3subnet			*subnets;	/**< List of local subnets */
	p3network			*net;		/**< Anonymous connection data */
	int					ID;			/**< Group member ID */
#ifdef _p3_PRIMARY
#define p3RMT_GROUPS	(p3MAX_GROUPS * 2)
	unsigned char		groups[p3RMT_GROUPS];	/**< Group assignment */
	unsigned int		rekey;		/**< Rekeying period */
	unsigned int		arraysz;	/**< Key array size */
	unsigned int		datarray;	/**< Data array period */
	unsigned int		ctlarray;	/**< Control array period */
	unsigned int		heartbeat;	/**< Heartbeat period */
	unsigned int		heartfail;	/**< Heartbeat failure time */
#endif

#ifdef _p3_SECONDARY
	int					port;		/**< Anonymous connections port */
#endif
	unsigned int		flag;		/* Use p3HCFG constants */
#define p3RMT_REPORT	0x01000000	/* Anonymous remote definition reported */
};

#ifdef _p3_PRIMARY
/**
 * Set a remote group ID in the specified category.
 */
#define set_rgroup(rmt, cat, gid) \
	do { \
		rmt->groups[(cat << 1)] = (gid & 0xff00) >> 8; \
		rmt->groups[(cat << 1) + 1] = (gid & 0xff); \
	} while (0)

/**
 * Get a remote group ID in the specified category.
 */
#define get_rgroup(rmt, cat, gid) \
		gid = (rmt->groups[(cat << 1)] << 8) | rmt->groups[(cat << 1) + 1]
#endif

/**
 * Structure:
 * p3subnet
 *
 * \par Description:
 * The subnet host structure maintains information about
 * subnets for which P3 hosts handle encryption.
 */

struct _p3subnet {
	p3subnet			*next;		/**< List pointer */
	char				*addr;		/**< Subnet address in CIDR format */
	int					ID;			/**< Subnet ID for parsing */
	unsigned int		flag;
// p3LHCFG_IPVER		0x00300000  /* IP address version field */
// p3HST_IPV4			0x00100000  /* Host address is IPv4 */
// p3HST_IPV6			0x00200000  /* Host address is IPv6 */
};

/*****  PROTOTYPES  *****/

int init_config(p3config *admin_cfg);
int get_config();
int parse_def(char *cfg_input, p3comm *cfg_def);
int handle_def(p3comm *cfg_def);
int handle_local_defs(p3comm *cfg_def);
int handle_group_defs(p3comm *cfg_def, p3group *group);
int handle_remote_defs(p3comm *cfg_def, p3remote *remote);
int validate_local(p3local *local);
int validate_group(p3group *group);
int validate_remote(p3remote *remote);
int save_config();

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _p3_CONFIG_C
extern p3group **grouplist;
#endif

#endif /* _p3_CONFIG_H */
