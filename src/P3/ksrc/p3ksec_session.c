/**
 * \file p3ksec_session.c
 * <h3>Protected Point to Point secondary session</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The secondary user session manages communications with primary P3 systems.
 */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <p><hr><hr>
 * \section P3KM_SEC_SESS Secondary P3 Session Messages
 */

#define _p3kSEC_SS_C
#include "p3ksession.h"
#include "p3ksecondary.h"

/**
 * \par Function:
 * session_manager
 *
 * \par Description:
 * Manage the secondary to secondary session.
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

int sec_session_manager()
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
 * set_key_array
 *
 * \par Description:
 * Receive an array of keys from the primary.  The secondary stores these
 * in an array for when an index is used to indicate the key replacement.
 * 
 * \par Inputs:
 * - message: The array of keys
 * - ksize: The key size
 * - dsize: The number of keys in the array
 * - p3sess: The session structure for the current P3 session.
 *
 * \par Outputs:
 * - int: Status (see the errors for the p3CMSG_ACK_KEY_ARRAY field)
 */

int set_key_array(unsigned char *message, int ksize, int dsize, p3session *p3sess)
{
	int sstat = 0;

	// TODO: Handle set key array message

	return (sstat);
} /* end set_key_array */

/**
 * \par Function:
 * replace_key
 *
 * \par Description:
 * Replace the data and control keys from the primary using
 * the key or index sent in the message.
 * 
 * \par Inputs:
 * - dkey: The data key contained in the message or NULL
 * - dindex: The data key index contained in the message or length of key
 * - ckey: The control key contained in the message or NULL
 * - cindex: The control key index contained in the message or length of key
 * - p3sess: The session structure for the current P3 session.
 *
 * \par Outputs:
 * - int: Status (see the errors for the p3CMSG_KEY_STATUS field)
 */

int replace_key(unsigned char *dkey, int dindex, unsigned char *ckey, int cindex,
				p3session *p3sess)
{
	int idx, len, stat = 0;
	unsigned int flag = 0, newseq;
	unsigned char message[4];
	p3ctlmsg *ctlmsg;

	// Set new data key
	if (dkey != NULL) {
		memcpy(p3sess->keymgmt.dnewkey->key, dkey, dindex);
	} else {
		if (p3sess->keylist == NULL || p3sess->listsize <= dindex) {
			stat = -1;
			flag |= p3CMSG_RKDIERR;
		} else {
			if ((p3sess->flag & p3PSS_KTYPE) == p3KTYPE_AES128) {
				idx = dindex * p3KSIZE_AES128;
				len = p3KSIZE_AES128;
			} else if ((p3sess->flag & p3PSS_KTYPE) == p3KTYPE_AES256) {
				idx = dindex * p3KSIZE_AES256;
				len = p3KSIZE_AES256;
			} else {
				stat = -1;
				flag |= p3CMSG_RKDIERR;
			}
		}
		if (!stat) {
			memcpy(p3sess->keymgmt.dnewkey->key, &p3sess->keylist[idx], len);
		}
	}

	// Set new control key
	if (ckey != NULL) {
		memcpy(p3sess->keymgmt.cnewkey->key, ckey, cindex);
	} else {
		if (p3sess->keylist == NULL || p3sess->listsize <= dindex) {
			stat = -1;
			flag |= p3CMSG_RKCIERR;
		} else {
			if ((p3sess->flag & p3PSS_KTYPE) == p3KTYPE_AES128) {
				idx = dindex * p3KSIZE_AES128;
				len = p3KSIZE_AES128;
			} else if ((p3sess->flag & p3PSS_KTYPE) == p3KTYPE_AES256) {
				idx = dindex * p3KSIZE_AES256;
				len = p3KSIZE_AES256;
			} else {
				stat = -1;
				flag |= p3CMSG_RKDIERR;
			}
		}
		if (!stat) {
			memcpy(p3sess->keymgmt.cnewkey->key, &p3sess->keylist[idx], len);
		}
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
	if ((ctlmsg = build_vlen_message(p3CMSG_REKEY, flag, message, 4))
		== NULL) {
		p3errmsg(p3MSG_ERR, "Error building data rekey response\n");
		stat = -1;
		goto out;
	} else if (p3send_control(p3sess, ctlmsg) < 0) {
		stat = -1;
		goto out;
	}
	p3lock(p3sess->lock);
	p3sess->flag |= p3PSS_REKEY;
	p3unlock(p3sess->lock);

out:
	return(stat);
} /* end replace_key */

/**
 * \par Function:
 * rekey_session
 *
 * \par Description:
 * Start using a replacement key from the primary.
 * 
 * \par Inputs:
 * - flag: The result flag from the primary
 * - key_num: The packet ID of the first packet to use the new key
 * - p3sess: The session structure for the current P3 session
 *
 * \par Outputs:
 * - None
 */

void rekey_session(int flag, unsigned int key_num, p3session *p3sess)
{
	int stat = 0;

	// Error in rekeying, continue using existing keys
	if (flag & p3CMSG_RKERR) {
		goto out;
	}
	// Use new key
	p3_rekey(&p3sess->keymgmt);
	p3sess->rID0 = p3sess->rID1;
	p3sess->rID1 = key_num;

out:
	// Allow data to be transmitted
	p3lock(p3sess->lock);
	p3sess->flag &= ~p3PSS_REKEY;
	p3unlock(p3sess->lock);
	return;
} /* end rekey_session */

/**
 * \par Function:
 * rekey_test_sec
 *
 * \par Description:
 * Receive a message from the primary, reverse it and resend it.
 * 
 * \par Inputs:
 * - message: The message from the primary
 * - size: The size of the message
 * - p3sess: The session structure for the current P3 session.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int rekey_test_sec(unsigned char *message, int size, p3session *p3sess)
{
	int stat = 0;

	// TODO: Handle rekey test message

	return (stat);
} /* end rekey_test_sec */

/**
 * \par Function:
 * heartbeat_query
 *
 * \par Description:
 * Handle a heartbeat query message from a primary.
 * 
 * \par Inputs:
 * - time: The P3 time that the message was sent
 * - seqnum: The sequence number of the query
 * - p3sess: The session structure for the current P3 session.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int heartbeat_query(unsigned int time, unsigned int seqnum, p3session *p3sess)
{
	int stat = 0;

	// TODO: Handle heartbeat query message

	return (stat);
} /* end heartbeat_query */

