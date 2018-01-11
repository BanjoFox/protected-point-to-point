/**
 * \file p3sec_net.c
 * <h3>Protected Point to Point template file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The secondary network functions include:
 * - Routing to secondaries
 * - Encrypting packets and encapsulating them in a modified IPSec
 *   packet
 */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <p><hr><hr>
 * \section P3_SEC_NET Secondary P3 Network Messages
 */

#define _p3SEC_NET_C
#include "p3net.h"

/** The main secondary network data structure */
p3host *pri_host;

/**
 * \par Function:
 * init_sec_net
 *
 * \par Description:
 * Initialize the secondary network functionality.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_sec_net()
{
	int stat = 0;

	// TODO: Connect to primary

out:
	return (stat);
} /* end init_sec_net */
