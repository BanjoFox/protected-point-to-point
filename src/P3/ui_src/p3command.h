/**
 * \file p3command.h
 * <h3>Protected Point to Point command interface header file</h3>
 *
 * Copyright (C) Velocite 2010
 */

#ifndef _p3_COMMAND_H
#define _p3_COMMAND_H

#include "p3utils.h"
#include "p3config.h"

/*****  CONSTANTS  *****/

#define p3IFCONFIG	"/sbin/ifconfig"
#define p3ROUTE		"/sbin/route"
#define p3FREE		"/usr/bin/free"
#define p3SYSCMDSZ	sizeof(p3IFCONFIG)	/* Longest system command */

#define p3RMT_UPDATE	0x01000000	/* Configuration data received */

/*****  MACROS  *****/

/*****  DATA DEFINITIONS  *****/

typedef struct _p3command p3command;

/**
 * Structure:
 * p3command
 *
 * \par Description:
 * The main command structure is used to locate all other
 * data structures.
 */

struct _p3command {
	p3config			*config;	/**< Access to local configuration data */
	int					s2ufifo;	/**< System to UI FIFO descriptor */
	int					u2sfifo;	/**< UI to System FIFO descriptor */
	int					stat_len;	/**< Number of log messages to return */
	time_t				stat_time;	/**< Timestamp of last status message */
	int					aport;		/**< Anonymous connection port */
#ifdef _p3_PRIMARY
	union {
		struct in_addr	v4;
		struct in6_addr	v6;
	} subnet;						/**< Subnet address (IPv4 or IPv6) */
	union {
		struct in_addr	v4;
		struct in6_addr	v6;
	} mask;							/**< Network mask (IPv4) */
	p3remote			*aremotes;	/**< Anonymous connection remote definitions */
#define p3CMD_ANONRPT	0x01000000	/* Anonymous host definition reported */
	int					anonpid;	/**< Anonymous connection handler process ID */
	int					anonsd;		/**< Anonymous connection socket descriptor */
	unsigned int		flag;
#define p3CMD_ANONSET	0x00000001	/* Anonymous connections allowed */
#define p3CMD_ANONNET	0x00000002	/* Limit anonymous connections to subnet */
#define p3CMD_ANONSRDY	0x00000004	/* Anonymous connection subnet ready */
#define p3CMD_ADMIN		0x00000008	/* Process is administration library */
//      p3HST_IPV4		0x00100000  /* Host address is IPv4 */
//      p3HST_IPV6		0x00200000  /* Host address is IPv6 */
#endif
};

/*****  PROTOTYPES  *****/

int init_command(p3command *admin_cmd);
int handle_cmd(p3comm *comm);
int handle_command(p3comm *cmd);
int handle_session(p3comm *sess);
int command_shutdown();
int external_cmd(char *cmd, char *argv[], p3comm *comm);

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _p3_COMMAND_C
extern p3command *command;
#endif

#endif /* _p3_COMMAND_H */
