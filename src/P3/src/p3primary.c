/**
 * \file p3primary.c
 * <h3>Protected Point to Point main primary file</h3>
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
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <p><hr><hr>
 * \section P3_PRIMARY Primary P3 Application Messages
 */

#define _p3_PRIMARY_C	0
#include "p3system.h"
#include "p3primary.h"
#include "p3pri_key_server.h"
#include "p3admin.h"

/**
 * ioctl_cmd defines the format of the ioctl data passed between the user and kernel space
 * where the command meanings are:
 * - noop: Unused
 * - primarycfg: Configuration data for a P3 primary system
 * - secondarycfg: Configuration data for a P3 secondary system
 * - prihostcfg: Configuration data for a remote P3 primary system
 * - sechostcfg: Configuration data for a remote P3 secondary system
 * - datakey1: The initial data key for a new P3 session
 * - controlkey1: The initial control key for a new P3 session
 */
enum ioctl_cmd iocmd;

/**
 * \par Function:
 * main
 *
 * \par Description:
 * Initialize the P3 Primary application, then listen for requests from
 * secondary P3 systems.
 * 
 * \par Inputs:
 * - argc: Number of arguments
 * - argv: Argument vector
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */
int main(int argc, char *argv[])
{
	int stat = 0;

	if ((stat = init_system(argc, argv)) != 0) {
		goto out;
	}

	// Call handler to monitor connections
	stat = system_handler();

out:
	if (stat > 0)
		stat = 0;
	// TODO: Attempt to shut down cleanly

	return (stat);
} /* end P3 primary main */
