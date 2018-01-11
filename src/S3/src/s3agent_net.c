/**
 * \file s3agent_net.c
 * <h3>Secure Storage Solution agent network file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The agent network manages the connection between a S3 agent and the
 * key server and encryption engine.
 */

/**
 * \page S3AG_MSGS Secure Storage Solution Agent Messages
 * <p><hr><hr>
 * \section S3AG_NET S3 Agent Network Messages
 */

#define _s3NET_C
#include "s3agent_net.h"

/** The main agent network data structure */
s3agent_net *net;

/**
 * \par Function:
 * init_net
 *
 * \par Description:
 * Initialize the agent network functionality.
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

	// Create agent network data structure
	if ((net = (s3agent_net *) calloc(1, sizeof(s3agent_net))) == NULL) {
		sprintf(s3buf, "init_net: Failed to allocate s3 agent network data\
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
 * \page S3AG_MSGS Secure Storage Solution Agent Messages
 * <hr><b>init_net: Failed to allocate agent network data structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The S3 agent attempts to allocate the agent network structure during
 * initialization.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * req_connect
 *
 * \par Description:
 * Request a connection.
 * 
 * \par Inputs:
 * - host_id: The server host ID, which is maintained in the agent network
 *   structure.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int req_connect(int host_id)
{
	int rc_stat = 0;

out:
	return (rc_stat);
} /* end req_connect */

/**
 * \par Function:
 * send_msg
 *
 * \par Description:
 * Send a S3 message.
 * 
 * \par Inputs:
 * - host_id: The server host ID
 * - message: The message to send
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int send_msg(int host_id, unsigned char *message)
{
	int sm_stat = 0;

out:
	return (sm_stat);
} /* end send_msg */

/**
 * \par Function:
 * recv_msg
 *
 * \par Description:
 * Receive a S3 message.
 * 
 * \par Inputs:
 * - host_id: The server host ID
 *
 * \par Outputs:
 * - unsigned char *: The buffer containing the message.  If there is
 *   an error, NULL is returned.
 */

unsigned char *recv_msg(int host_id)
{
	char * rm_buf = NULL;

out:
	return (rm_buf);
} /* end recv_msg */

