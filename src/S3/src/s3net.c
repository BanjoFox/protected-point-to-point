/**
 * \file s3net.c
 * <h3>Secure Storage Solution template file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The S3 network functions include:
 * - Establishing an encrypted session between a S3 agent and either the
 *   key server or the encryption engine
 */

/**
 * \page S3MSGS Secure Storage Solution Messages
 * <p><hr><hr>
 * \section S3_NET S3 Network Messages
 */

#define _s3NET_C
#include "s3net.h"
#include "s3key_server.h"
#include "s3key server.h"

/** The main key server network data structure */
s3net *net;

/**
 * \par Function:
 * init_net
 *
 * \par Description:
 * Initialize the key server network functionality.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_net()
{
	int pn_stat = 0;

	// Create key server network data structure
	if ((net = (s3net *) calloc(1, sizeof(s3net))) == NULL) {
		sprintf(s3buf, "init_net: Failed to allocate s3 key server network data\
 structure: %s\n", strerror(errno));
		s3errmsg (s3MSG_CRIT, s3buf);
		pn_stat = -1;
		goto out;
	}

	// Build S3 table

	// 

	// Start monitoring network data

out:
	return (pn_stat);
} /* end init_net */

/**
 * \page S3MSGS Secure Storage Solution Messages
 * <hr><b>init_net: Failed to allocate key server network data structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The S3 key server attempts to allocate the key server network structure during
 * initialization.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * s3_listen
 *
 * \par Description:
 * Listen on the designated server port.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int s3_listen()
{
	int sl_stat = 0;

	// Listen on the designated server port

out:
	return (sl_stat);
} /* end s3_listen */

