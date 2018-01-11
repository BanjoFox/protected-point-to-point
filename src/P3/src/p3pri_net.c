/**
 * \file p3pri_net.c
 * <h3>Protected Point to Point template file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The primary network functions include:
 * - Routing to secondaries
 * - Encrypting packets and encapsulating them in a modified IPSec
 *   packet
 */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <p><hr><hr>
 * \section P3_PRI_NET Primary P3 Network Messages
 */

#define _p3PRI_NET_C
#include "p3net.h"

/** The main primary network data structure */
p3host *sec_host = NULL;

/**
 * \par Function:
 * init_pri_net
 *
 * \par Description:
 * Initialize the primary P3 system to listen for secondary P3 session
 * requests.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_pri_net()
{
	int stat = 0;

	// TODO: Monitor for session requests

out:
	return (stat);
} /* end init_pri_net */

