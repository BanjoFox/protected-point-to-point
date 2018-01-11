/**
 * \file s3session.c
 * <h3>Secure Storage Solution session manager</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The S3 key server and encryption engine listen for a request from
 * the S3 agent.  Session management is common to either server.
 */

/**
 * \page S3MSGS Secure Storage Solution Messages
 * <p><hr><hr>
 * \section S3_SESS S3 Session Messages
 */

#define _s3UI_C
#include "s3session.h"

/** Global variable usage */
s3session *session;

/**
 * \par Function:
 * init_session
 *
 * \par Description:
 * Initialize the server to agent session.  This consists of exchanging
 * messages according to the S3 System Specification.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_session()
{
	int ps_status = 0;

	// Create key server session data structure
	if ((session = (s3session *) calloc(1, sizeof(s3session))) == NULL) {
		sprintf(s3buf, "init_session: Failed to allocate key server user interface\
 data structure: %s\n", strerror(errno));
		s3errmsg (s3MSG_CRIT, s3buf);
		ps_status = -1;
		goto out;
	}

	// Establish encrypted session using IPSec

	// Exchange initialization messages

out:
	return (ps_status);
} /* end init_session */

/**
 * \page S3MSGS Secure Storage Solution Messages
 * <hr><b>init_session: Failed to allocate key server session data structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The S3 server attempts to allocate the session structure when a S3
 * agent requests a connection.  If this fails, there is a system wide
 * problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * session_manager
 *
 * \par Description:
 * Manage a session between a server and a S3 agent.
 *
 * NOTE: If implemented as a continuous child process, see sendmsg,
 *   recvmsg, and cmsg for passing socket file descriptors.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int session_manager()
{
	int sm_status = 0;

out:
	return (sm_status);
} /* end session_manager */

