/**
 * \file p3secondary.c
 * <h3>Protected Point to Point main secondary file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The Velocite Systems Protected Point to Point (P3) Secondary system
 * is the client end point in a virtual point to point connection over
 * the Internet.  It requests a connections from a primary P3 system,
 * and when a request is accepted, establishes an IPSec encrypted session.
 * <p>
 * When an encrypted Data session is established, an encrypted Control
 * session which is tunneled through the main session is established.  This
 * is immediately used to set new encryption keys for both the Data and
 * Control sessions.
 * <p>
 * All runtime administration is performed from the primary P3 system.
 * The user interface for initial setup is interactive from the command line.
 */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <p><hr><hr>
 * \section P3_SECONDARY Secondary P3 Application Messages
 */

#define _p3_SECONDARY_C	0
#include "p3system.h"
#include "p3secondary.h"
#include "p3sec_key_handler.h"
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
 * Initialize the P3 Secondary application, then listen for requests from
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
	// TODO: Attempt connecting to primary P3 system

	// TODO: Attempt to shut down cleanly

	while (1)
		sleep(2);
out:
	if (stat > 0)
		stat = 0;
	return (stat);
} /* end P3 secondary main */
