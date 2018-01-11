/**
 * \file p3kprimaryplus.c
 * <h3>Protected Point to Point main primary plus secondary file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The Velocite Systems Protected Point to Point (P3) Primary system
 * is the master end point in a virtual point to point connection over
 * the Internet.  It listens for connections from secondary P3 systems,
 * and when a request is accepted, establishes an IPSec encrypted session.
 * <p>
 * When an encrypted Data session is established, an encrypted Control
 * session which is tunneled through the main session is established.  This
 * is immediately used to set new encryption keys for both the Data and
 * Control sessions.
 * <p>
 * All runtime administration is performed from the primary P3 system.
 * The user interface is interactive from the command line.
 */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <p><hr><hr>
 * \section P3KM_PRIMARYPLUS Primary plus Secondary P3 System Messages
 */

#define _p3k_PRIMARYPLUS_C	0
#include "p3kprimaryplus.h"
#include "p3kshare.h"

/** The main primary plus secondary structure */
p3pri_main *primain = NULL;
p3sec_main *secmain = NULL;

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
enum ioctl_cmd iocmd;

/** Working buffer */
char tbuf[4092], *p3buf = tbuf;

/**
 * \par Function:
 * init_p3primaryplus
 *
 * \par Description:
 * Initialize the P3 Primary plus Secondary module by creating the basic structures for
 * handling P3 secondary connections.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

// TODO: Separate __init from function name
int init_primaryplus ()
{
	int stat = 0;

	// Create main p3 primary plus secondary data structure
	if ((primain = (p3pri_main *) p3calloc(sizeof(p3pri_main))) == NULL) {
		p3errmsg(p3MSG_CRIT, "init_p3primaryplus: Failed to allocate main p3 primary plus secondary data structure\n");
		stat = -1;
		goto out;
	}
	if ((secmain = (p3sec_main *) p3calloc(sizeof(p3sec_main))) == NULL) {
		p3errmsg(p3MSG_CRIT, "init_p3primaryplus: Failed to allocate main p3 primary plus secondary data structure\n");
		stat = -1;
		goto out;
	}

	// Initialize P3 routing
	if (init_p3net < 0) {
		stat = -1;
		goto out;
	}

out:
	if (stat > 0)
		stat = 0;
	return (stat);
} /* end init_p3primaryplus */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <hr><b>p3pri_main: Failed to allocate main p3 primary plus secondary data structure</b>
 * \par Description (CRIT):
 * The P3 primary plus secondary attempts to allocate its main structure before any other
 * activity.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem.
 *
 * <hr><b>p3pri_main: Failed to allocate main p3 primary plus secondary configuration structure</b>
 * \par Description (CRIT):
 * The P3 primary plus secondary attempts to allocate its configuration structure before parsing the
 * configuration from sysfs.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem.
 */

/**
 * \par Function:
 * pri_parse_config
 *
 * \par Description:
 * Parse the P3 Primary plus Secondary configuration file from the P3 sysfs structures.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int priplus_parse_config()
{
	int stat = 0;

	// Get data to parse from mmapped area
	// Access the sysfs structure

	// TODO: Parse sysfs configuration data
	// Get interface definitions
	// Get random number generator definition
	// Get default number of keys in key arrays
	// Get heartbeat period
	// Get heartbeat failure time
	// Get clustering definitions
	// Get failover definitions

	// Get secondary P3 host address
	// Get subnet list
	// Get key type
	// Get rekey wait time
	// Get key array allowed
	// Get heartbeat period override
	// Get heartbeat failure time override
out:
	return(stat);
} /* end pri_parse_config */

