/**
 * \file p3kpri_session.c
 * <h3>Protected Point to Point primary session</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The primary user session manages communications with secondary P3 systems.
 */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <p><hr><hr>
 * \section P3KM_PRI_SESS Primary P3 Session Messages
 */

#define _p3kPRI_SS_C
#include "p3ksession.h"
#include "p3kprimary.h"

/**
 * \par Function:
 * session_manager
 *
 * \par Description:
 * Manage the primary to secondary session.
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

int pri_session_manager()
{
	int stat = 0;

	// TODO: Establish encrypted session using IPSec

	// Open Control Channel

	// Get data/control key pair from key server

	// Reset Data key

	// Reset Control key

out:
	return (stat);
} /* end session_manager */

/**
 * \par Function:
 * key_array_error
 *
 * \par Description:
 * Handle a key array error message from the secondary session.
 *
 * \par Inputs:
 * - flag:  Indicates the specific error
 * - p3sess: The session structure for the current P3 session.
 *
 * \par Outputs:
 * - None
 */

void key_array_error(int flag, p3session *p3sess)
{

	// TODO: Handle set key array error response

} /* end key_array_error */

/**
 * \par Function:
 * rekey_session
 *
 * \par Description:
 * Start using a replacement key from the secondary.  The primary
 * sends a Rekey message as acknowledgment to the secondary.
 * 
 * \par Inputs:
 * - flag: The result flag from the secondmary
 * - key_num: The packet ID of the first packet to use the new key
 * - p3sess: The session structure for the current P3 session
 *
 * \par Outputs:
 * - None
 */

void rekey_session(int flag, unsigned int key_num, p3session *p3sess)
{
	int stat = 0;
	unsigned int newseq;
	unsigned char message[4];
	p3ctlmsg *ctlmsg;

	// Error in rekeying, continue using existing keys
	if (flag & p3CMSG_RKERR) {
		goto out;
	}

	newseq = p3sess->sseq + 1;
	if (!newseq)
		newseq++;
	message[3] = (unsigned char) newseq;
	newseq >>= 8;
	message[2] = (unsigned char) newseq;
	newseq >>= 8;
	message[1] = (unsigned char) newseq;
	newseq >>= 8;
	message[0] = (unsigned char) newseq;
	if ((ctlmsg = build_vlen_message(p3CMSG_REKEY, 0, message, 4))
		== NULL) {
		p3errmsg(p3MSG_ERR, "Error building data rekey response\n");
		stat = -1;
		goto out;
	} else if (p3send_control(p3sess, ctlmsg) < 0) {
		stat = -1;
		goto out;
	}

	// Use new key
	p3_rekey(&p3sess->keymgmt);
	p3sess->rID0 = p3sess->rID1;
	p3sess->rID1 = key_num;

out:
	p3lock(p3sess->lock);
	p3sess->flag &= ~p3PSS_REKEY;
	p3unlock(p3sess->lock);
	return;
} /* end rekey_session */

/**
 * \par Function:
 * rekey_test_pri
 *
 * \par Description:
 * Receive a message from a secondary, reverse it and compare it to the
 * original test message.
 * 
 * \par Inputs:
 * - message: The message from the secondary
 * - size: The size of the message
 * - p3sess: The session structure for the current P3 session.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int rekey_test_pri(unsigned char *message, int size, p3session *p3sess)
{
	int stat = 0;

p3errmsg(p3MSG_INFO, "HANDLER: Rekey test\n");
	// TODO: Handle rekey test message

	return (stat);
} /* end rekey_test_pri */

/**
 * \par Function:
 * heartbeat_answer
 *
 * \par Description:
 * Handle a heartbeat answer message from a secondary.
 * 
 * \par Inputs:
 * - time: The P3 time that the message was sent
 * - seqnum: The sequence number of the query
 * - p3sess: The session structure for the current P3 session.
 *
 * \par Outputs:
 * - None
 */

void heartbeat_answer(unsigned int time, unsigned int seqnum, p3session *p3sess)
{

	// TODO: Handle heartbeat answer message

	return;
} /* end heartbeat_answer */

