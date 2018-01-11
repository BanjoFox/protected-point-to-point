/**
 * \file p3ksession.c
 * <h3>Protected Point to Point primary session</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The primary user session manages communications with secondary P3 systems.
 */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <p><hr><hr>
 * \section P3KM_SESSION P3 Session Messages
 */

#include "p3ksession.h"
#include "p3knet.h"

/** The time_t equivalent of the previous midnight */
time_t midnight = 0;

/**
 * \par Function:
 * init_session
 *
 * \par Description:
 * Initialize a session.  This allocates a new session structure and
 * all of its fields.
 *
 * \par Inputs:
 * - host: The host structure for the session to be initialized.
 * - srcaddr: The local host address.  Because it can be either
 *   IPv4 or IPv6, it is passed as a void pointer.
 *
 * \par Outputs:
 * - None.
 */

 void init_session(p3host *host, void *srcaddr)
{
	int size;
	unsigned long l;
	struct timeval kern_tv, *now = &kern_tv;
	p3session *session = host->session;

	do_gettimeofday(now);

	// Initialize key fields
	l = (unsigned long) session + sizeof(p3session);
	session->keymgmt.dnewkey = (p3key *) l;
	l += sizeof(p3key);
	session->keymgmt.cnewkey = (p3key *) l;
	l += sizeof(p3key);
	session->p3hdr = (unsigned char *) l;

	// P3 session sequence starts at 1
	session->sseq = 1;
	p3lock_init(session->lock);
#ifndef _p3_SECONDARY
	session->dikey = now->tv_sec + session->ditime;
	session->cikey = now->tv_sec + session->citime;
#endif
	session->flag = (host->flag & p3HST_IPVER) | ((host->flag & p3HST_KTYPE) >> p3HST_KTSHF);
	if ((session->flag & p3PSS_KTYPE) == p3KTYPE_AES128) {
		session->keymgmt.dnewkey->size = p3KSIZE_AES128;
		session->keymgmt.cnewkey->size = p3KSIZE_AES128;
	} else if ((session->flag & p3PSS_KTYPE) == p3KTYPE_AES256) {
		session->keymgmt.dnewkey->size = p3KSIZE_AES256;
		session->keymgmt.cnewkey->size = p3KSIZE_AES256;
	} else {
		session->keymgmt.dnewkey->size = p3KSIZE_AES128;
		session->keymgmt.cnewkey->size = p3KSIZE_AES128;
	}

	// Initialize P3 network header
	if (host->flag & p3HST_IPV4) {
		session->p3hdr[0] = 0x45;	// Set version and header length
		session->p3hdr[1] = 0;		// Type of Service
		session->p3hdr[6] = 0x40;	// Don't fragment
		session->p3hdr[8] = 128;	// TTL
		session->p3hdr[9] = p3PROTO;	// Set P3 protocol
		memcpy(&session->p3hdr[12], srcaddr, sizeof(struct in_addr));
		memcpy(&session->p3hdr[16], &host->addr.v4, sizeof(struct in_addr));
	} else if (host->flag & p3HST_IPV6) {
		// TODO: Support IPv6 in init session
	}

out:
	return;
} /* end init_session */

#ifndef _p3_SECONDARY
/**
 * \par Function:
 * build_newkey_message
 *
 * \par Description:
 * Build a control message that only contains a flag.
 *
 * \par Inputs:
 * - flag: The flag settings, note that DINDEX and CINDEX flags are
 *   set automatically
 * - sess: Session structure
 *
 * \par Outputs:
 * - unsigned char *: The encrypted message or NULL if there is an error.
 */

p3ctlmsg *build_newkey_message(unsigned int flag, p3session *p3sess, p3key_mgr *key_mgr)
{
	int msize = 1, didx = -1, cidx = -1, mask = 1;
	unsigned int mflag = flag | (p3sess->flag & p3PSS_KTYPE);
	unsigned char message[(5 + (p3MAX_KSIZE * 2))];
	p3ctlmsg *ctlmsg = NULL;
	struct timeval kern_tv, *now = &kern_tv;

	do_gettimeofday(now);

	// Use data key or index
	if (now->tv_sec > p3sess->dikey) {
		p3sess->dikey += p3sess->ditime;
		mflag |= p3CMSG_KRDIDX;
		// Randomize listsize
		while (mask < p3sess->listsize)
			mask <<= 1;
		mask -= 1;
		didx = now->tv_usec & mask;
		while (didx > p3sess->listsize)
			didx -= p3sess->listsize;
		message[2] = (unsigned char) didx;
		didx >>= 8;
		message[1] = (unsigned char) didx;
		msize += 2;
	} else {
		if (p3_get_key(p3sess->keymgmt.dnewkey, key_mgr) < 0)
			goto out;
		memcpy(&message[1], p3sess->keymgmt.dnewkey->key,
			   p3sess->keymgmt.dnewkey->size);
		msize += p3sess->keymgmt.dnewkey->size;
	}

	// Use control key or index
	if (now->tv_sec > p3sess->cikey) {
		p3sess->cikey += p3sess->citime;
		mflag |= p3CMSG_KRCIDX;
		// Randomize listsize
		while (mask < p3sess->listsize)
			mask <<= 1;
		mask -= 1;
		cidx = (now->tv_usec >> 1) & mask;
		while (cidx > p3sess->listsize)
			cidx -= p3sess->listsize;
		message[msize + 1] = (unsigned char) cidx;
		cidx >>= 8;
		message[msize] = (unsigned char) cidx;
		msize += 2;
	} else {
		if (p3_get_key(p3sess->keymgmt.cnewkey, key_mgr) < 0)
			goto out;
		memcpy(&message[msize], p3sess->keymgmt.cnewkey->key,
			   p3sess->keymgmt.cnewkey->size);
		msize += p3sess->keymgmt.cnewkey->size;
	}

	// Set the flag
	message[0] = (unsigned char) mflag;

	ctlmsg = build_ctl_message (p3CMSG_REPLACE_KEY, message, msize);

out:
	return(ctlmsg);
} /* end build_newkey_message */
#endif

/**
 * \par Function:
 * build_flag_message
 *
 * \par Description:
 * Build a control message that only contains a flag.
 *
 * \par Inputs:
 * - type: P3 control message type
 * - flag: The flag settings
 *
 * \par Outputs:
 * - unsigned char *: The encrypted message or NULL if there is an error.
 */

p3ctlmsg *build_flag_message(int type, unsigned int flag)
{
	unsigned char message[1];
	p3ctlmsg *ctlmsg;

	message[0] = (unsigned char) flag;
	ctlmsg = build_ctl_message (type, message, 1);

	return(ctlmsg);
} /* end build_flag_message */

/**
 * \par Function:
 * build_vlen_message
 *
 * \par Description:
 * Build a varilable length control message.  All variable length messages
 * except Rekey Test include a flag.
 *
 * \par Inputs:
 * - type: P3 control message type
 * - flag: The flag settings
 * - message: The variable length message
 * - len: The length of the message
 *
 * \par Outputs:
 * - unsigned char *: The encrypted message or NULL if there is an error.
 */

p3ctlmsg *build_vlen_message(int type, unsigned int flag,
				  unsigned char *message, int len)
{
	int i, msize = len;
	unsigned char vlenmsg[256], *vmsg = NULL;
	p3ctlmsg *ctlmsg = NULL;

	// Get size of variable length message
	switch (type) {
		case p3CMSG_REKEY_TEST:
		case p3CMSG_REKEY:
			msize += 1;
			break;
		case p3CMSG_SET_KEY_ARRAY:
		case p3CMSG_STATUS_REQ:
		case p3CMSG_STATUS_RESP:
			msize += 4;
			break;
		case p3CMSG_UPDATE_INFO:
			msize += 5;
			break;
		default:
			goto out;
	}

	if (msize > p3MAX_MSG_SZ) {
		p3errmsg(p3MSG_ERR, "build_vlen_message: Message length exceeds maximum\n");
		goto out;
	} else if (msize > 256 && (vmsg = (unsigned char *)
			p3malloc(msize)) == NULL) {
		p3errmsg(p3MSG_ERR, "build_vlen_message: Failed to allocate message buffer\n");
		goto out;
	} else
		vmsg = vlenmsg;

	// Build variable length message
	switch (type) {
		case p3CMSG_REKEY_TEST:
			vmsg[0] = (unsigned char) len;
			memcpy(&vmsg[1], message, len);
			break;
		case p3CMSG_REKEY:
			vmsg[0] = (unsigned char) flag;
			memcpy(&vmsg[1], message, len);
			break;
		case p3CMSG_STATUS_REQ:
		case p3CMSG_STATUS_RESP:
			vmsg[0] = (unsigned char) flag;
			i = len;
			vmsg[2] = (unsigned char) i;
			i >>= 8;
			vmsg[1] = (unsigned char) i;
			memcpy(&vmsg[3], message, len);
			break;
		case p3CMSG_SET_KEY_ARRAY:
			vmsg[0] = (unsigned char) flag;
			i = len;
			vmsg[3] = (unsigned char) i;
			i >>= 8;
			vmsg[2] = (unsigned char) i;
			i >>= 8;
			vmsg[1] = (unsigned char) i;
			memcpy(&vmsg[4], message, len);
			break;
		case p3CMSG_UPDATE_INFO:
			vmsg[0] = (unsigned char) flag;
			i = len;
			vmsg[4] = (unsigned char) i;
			i >>= 8;
			vmsg[3] = (unsigned char) i;
			i >>= 8;
			vmsg[2] = (unsigned char) i;
			i >>= 8;
			vmsg[1] = (unsigned char) i;
			memcpy(&vmsg[5], message, len);
			break;
		default:
			goto out;
	}

	ctlmsg = build_ctl_message (type, vmsg, msize);

out:
	if (msize > 256 && vmsg != NULL)
		p3free(vmsg);
	return(ctlmsg);
} /* end build_vlen_message */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <hr><b>build_vlen_message: Message length exceeds maximum</b>
 * \par Description (ERR):
 * The variable length message was too large to fit into a single
 * packet, and needs to be divided into smaller pieces.
 * \par Response:
 * Report the problem to Velocite Systems support.
 *
 * <hr><b>build_vlen_message: Failed to allocate message buffer</b>
 * \par Description (ERR):
 * The message builder attempts to allocate a buffer to build a
 * control message.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem.
 *
 */

/**
 * \par Function:
 * build_heartbeat_message
 *
 * \par Description:
 * Build a heartbeat query or answer message.
 *
 * \par Inputs:
 * - type: P3 control message type
 * - sequence: The heartbeat sequence number
 *
 * \par Outputs:
 * - unsigned char *: The encrypted message or NULL if there is an error.
 */

p3ctlmsg *build_heartbeat_message(int type, unsigned int sequence)
{
	unsigned char message[8];
	p3ctlmsg *ctlmsg = NULL;
	time_t bhm_time;
	struct timeval kern_tv, *now = &kern_tv;

	do_gettimeofday(now);

	// Get number of seconds past midnight
	if (!midnight || (bhm_time = (now->tv_sec - midnight)) > 86400) {
		// Reset midnight for current day
		midnight = now->tv_sec / 86400;
		bhm_time = now->tv_sec - midnight;
	}

	message[7] = (unsigned int) sequence;
	sequence >>= 8;
	message[6] = (unsigned int) sequence;
	sequence >>= 8;
	message[5] = (unsigned int) sequence;
	sequence >>= 8;
	message[4] = (unsigned int) sequence;

	message[3] = (unsigned char) now->tv_usec;
	now->tv_usec >>= 8;
	bhm_time <<= 4;
	message[2] = (unsigned char) ((bhm_time & 0xf0) | (now->tv_usec & 0xf));
	bhm_time >>= 12;
	message[1] = (unsigned char) bhm_time;
	bhm_time >>= 8;
	message[0] = (unsigned char) bhm_time;

	ctlmsg = build_ctl_message (type, message, 8);

out:
	return(ctlmsg);
} /* end build_heartbeat_message */

/**
 * \par Function:
 * build_ctl_message
 *
 * \par Description:
 * Build the requested control message and encrypt it.  This is called by
 * individual control message build functions.
 *
 * \par Inputs:
 * - type: P3 control message type
 * - message: Message data
 * - len: Length of message data
 *
 * \par Outputs:
 * - p3ctlmsg *: The encrypted message structure or NULL if there is an error.
 */

p3ctlmsg *build_ctl_message(int type, unsigned char *message, int len)
{
	int i, msize = len + 5;
	unsigned long l;
	p3ctlmsg *ctlmsg = NULL;

	// Initialize control message
	if ((ctlmsg = (p3ctlmsg *) p3calloc(sizeof(p3ctlmsg) + msize)) == NULL) {
		goto out;
	}
	l = (unsigned long) ctlmsg + sizeof(p3ctlmsg);
	ctlmsg->message = (unsigned char *) l;
	ctlmsg->len = msize;

	// Set length, type, and message fields
	ctlmsg->message[0] = (unsigned char) (msize >> 24) & 0xff;
	ctlmsg->message[1] = (unsigned char) (msize >> 16) & 0xff;
	ctlmsg->message[2] = (unsigned char) (msize >> 8) & 0xff;
	ctlmsg->message[3] = (unsigned char) msize & 0xff;
	ctlmsg->message[4] = (unsigned char) type;
	memcpy(&ctlmsg->message[5], message, len);

out:
	return (ctlmsg);
} /* end build_ctl_message */

/**
 * \par Function:
 * parse_ctl_message
 *
 * \par Description:
 * Parse a message and call the function to handle it.  The format of
 * the Control Messages is:
 * <pre>
 * | 4 octets | 1 octet | Variable length |
 * | Size     | Command | Data            |
 *   where the fields are:
 * Size:  The total size of the message, including the size field itself
 * Command:  An 8-bit number
 * Data:  The data, if any, associated with the command
 * </pre>
 *
 * The handler functions must be defined in the primary or secondary session
 * handler, depending on which receives the message.  The message handling
 * code is only compiled if the P3 system is the receiver.  All message handling
 * is compiled for the Primary Plus Secondary P3 system.
 *
 * See the P3 System Specification for details on the commands.
 *
 * \par Inputs:
 * - ctlmsg: The message structure, including the buffer with the size field.
 * - p3sess: The session structure for the current P3 session.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 *   - >0 = Status from remote P3 system
 */

int parse_ctl_message(p3ctlmsg *ctlmsg, p3session *p3sess)
{
	int idx, stat = 0;
	int cmd, cflag, dsize, ksize;
	int didx, cidx;
	unsigned int pktidx;
	unsigned char *dkey, *ckey;
int i;
sprintf(p3buf, "Control message %d:\n", ctlmsg->message[4]);
for (i=0; i < ctlmsg->len; i++) {
	if (!(i & 0x3))
		sprintf(p3buf, "%s ", p3buf);
	sprintf(p3buf, "%s%2.2x", p3buf, ctlmsg->message[i]);
}
sprintf(p3buf, "%s\n", p3buf);
p3errmsg(p3MSG_DEBUG, p3buf);

	// Get the command
	cmd = (unsigned int) ctlmsg->message[4];
	// Parse each command and call the appropriate handler
	switch (cmd) {

// The ifndef statement is defined for the sender of the message
// to allow the primary plus to handle both types of messages
#ifndef _p3_PRIMARY
		// | 1 octet | 3 octets       | Variable length |
		// | Flags   | Key Array Size | Key Array       |
		case p3CMSG_SET_KEY_ARRAY:
			cflag = (unsigned int) ctlmsg->message[5];
			dsize = (unsigned int) ctlmsg->message[6];
			dsize <<= 8;
			dsize |= (unsigned int) ctlmsg->message[7];
			dsize <<= 8;
			dsize |= (unsigned int) ctlmsg->message[8];
			if ((cflag & p3CMSG_KTYPE) == p3KTYPE_AES128) {
				ksize = 128 >> 3;
			} else if ((cflag & p3CMSG_KTYPE) == p3KTYPE_AES256) {
				ksize = 256 >> 3;
			} else {
				break;
			}
			stat = set_key_array(&(ctlmsg->message[9]), ksize, dsize, p3sess);
			break;
#endif

#ifndef _p3_SECONDARY
		// | 1 octet |
		// | Flags   |
		case p3CMSG_ACK_KEY_ARRAY:
			cflag = (unsigned int) ctlmsg->message[5];
			if (cflag & p3CMSG_AKERR) {
				key_array_error(cflag & ~p3CMSG_AKERR, p3sess);
				stat = -1;
			}
			break;
#endif

#ifndef _p3_PRIMARY
		// | 1 octet | Index or key length | Index or key length  |
		// | Flags   | Data Key            | Control Key          |
		case p3CMSG_REPLACE_KEY:
			cflag = (unsigned int) ctlmsg->message[5];
			// Replace data key
			idx = 0;
			if (cflag & p3CMSG_KRDIDX) {
				dkey = NULL;
				didx = (unsigned int) ctlmsg->message[6];
				didx <<= 8;
				didx |= (unsigned int) ctlmsg->message[7];
				idx = 8;
			} else if ((cflag & p3CMSG_KTYPE) == p3KTYPE_AES128) {
				dkey = &ctlmsg->message[6];
				didx = p3KSIZE_AES128;
				idx = 6 + p3KSIZE_AES128;
			} else if ((cflag & p3CMSG_KTYPE) == p3KTYPE_AES256) {
				dkey = &ctlmsg->message[6];
				didx = p3KSIZE_AES256;
				idx = 6 + p3KSIZE_AES256;
			} else {
				stat = -1;
				goto out;
			}
			
			// Replace control key
			if (cflag & p3CMSG_KRCIDX) {
				ckey = NULL;
				cidx = (unsigned int) ctlmsg->message[idx++];
				cidx <<= 8;
				cidx |= (unsigned int) ctlmsg->message[idx];
			} else if ((cflag & p3CMSG_KTYPE) == p3KTYPE_AES128) {
				ckey = &ctlmsg->message[idx];
				cidx = p3KSIZE_AES128;
			} else if ((cflag & p3CMSG_KTYPE) == p3KTYPE_AES256) {
				ckey = &ctlmsg->message[idx];
				cidx = p3KSIZE_AES256;
			} else {
				stat = -1;
				goto out;
			}

			if ((stat = replace_key(dkey, didx, ckey, cidx, p3sess)) != 0)
				goto out;
			break;
#endif

		// | 1 octet | 4 octets   |
		// | Flags   | Key number |
		case p3CMSG_REKEY:
			cflag = (unsigned int) ctlmsg->message[5];
			pktidx = (unsigned int) ctlmsg->message[6];
			pktidx <<= 8;
			pktidx |= (unsigned int) ctlmsg->message[7];
			pktidx <<= 8;
			pktidx |= (unsigned int) ctlmsg->message[8];
			pktidx <<= 8;
			pktidx |= (unsigned int) ctlmsg->message[9];
			rekey_session(cflag, pktidx, p3sess);
			break;

		// | 1 octet | Variable length |
		// | Size    | Test Message    |
		case p3CMSG_REKEY_TEST:
			dsize = (unsigned int) ctlmsg->message[5];
#ifndef _p3_SECONDARY
			rekey_test_pri(&ctlmsg->message[6], dsize, p3sess);
#endif
#ifndef _p3_PRIMARY
			rekey_test_sec(&ctlmsg->message[6], dsize, p3sess);
#endif
			break;

#ifndef _p3_PRIMARY
		// | 4 octets  | 4 octets   |
		// | Timestamp | Sequence # |
		case p3CMSG_HRTBEAT_QUERY:
			// Get seconds after midnight
			cidx = (unsigned int) ctlmsg->message[5];
			cidx <<= 8;
			cidx |= (unsigned int) ctlmsg->message[6];
			cidx <<= 8;
			cidx |= (unsigned int) ctlmsg->message[7];
			cidx <<= 8;
			cidx |= (unsigned int) ctlmsg->message[8];
			// Get heartbeat sequence #
			pktidx = (unsigned int) ctlmsg->message[5];
			pktidx <<= 8;
			pktidx |= (unsigned int) ctlmsg->message[6];
			pktidx <<= 8;
			pktidx |= (unsigned int) ctlmsg->message[7];
			pktidx <<= 8;
			pktidx |= (unsigned int) ctlmsg->message[8];
			stat = heartbeat_query(cidx, pktidx, p3sess);
			break;
#endif

#ifndef _p3_SECONDARY
		// | 4 octets  | 4 octets   |
		// | Timestamp | Sequence # |
		case p3CMSG_HRTBEAT_ANSWER:
			// Get seconds after midnight
			cidx = (unsigned int) ctlmsg->message[5];
			cidx <<= 8;
			cidx |= (unsigned int) ctlmsg->message[6];
			cidx <<= 8;
			cidx |= (unsigned int) ctlmsg->message[7];
			cidx <<= 8;
			cidx |= (unsigned int) ctlmsg->message[8];
			// Get heartbeat sequence #
			pktidx = (unsigned int) ctlmsg->message[5];
			pktidx <<= 8;
			pktidx |= (unsigned int) ctlmsg->message[6];
			pktidx <<= 8;
			pktidx |= (unsigned int) ctlmsg->message[7];
			pktidx <<= 8;
			pktidx |= (unsigned int) ctlmsg->message[8];
			heartbeat_answer(cidx, pktidx, p3sess);
			break;
#endif

#ifndef _p3_PRIMARY
		// | 2 octets  | 2 octets     | Variable length |
		// | Request # | Request Size | Status Requests |
		case p3CMSG_STATUS_REQ:
			dsize = (unsigned int) ctlmsg->message[5];
			dsize <<= 8;
			dsize = (unsigned int) ctlmsg->message[6];
			break;
#endif

#ifndef _p3_SECONDARY
		// | 2 octets  | 2 octets      | Variable length  |
		// | Request # | Response Size | Status Responses |
		case p3CMSG_STATUS_RESP:
			dsize = (unsigned int) ctlmsg->message[5];
			dsize <<= 8;
			dsize = (unsigned int) ctlmsg->message[6];
			break;
#endif

#ifndef _p3_PRIMARY
		// | 1 octet | 4 octets    | Variable length |
		// | Flags   | Update Size | Update Data     |
		case p3CMSG_UPDATE_INFO:
			cflag = (unsigned int) ctlmsg->message[5];
			dsize = (unsigned int) ctlmsg->message[6];
			dsize <<= 8;
			dsize = (unsigned int) ctlmsg->message[7];
			dsize <<= 8;
			dsize = (unsigned int) ctlmsg->message[8];
			dsize <<= 8;
			dsize = (unsigned int) ctlmsg->message[9];
			break;
#endif

#ifndef _p3_SECONDARY
		// | 1 octet |
		// | Flags   |
		case p3CMSG_ACK_UPDATE:
			cflag = (unsigned int) ctlmsg->message[5];
			break;
#endif

#ifndef _p3_PRIMARY
		// | 1 octet |
		// | Flags   |
		case p3CMSG_SHUTDOWN:
			cflag = (unsigned int) ctlmsg->message[5];
			break;
#endif

#ifndef _p3_SECONDARY
		// | 1 octet |
		// | Flags   |
		case p3CMSG_ACK_SHUTDOWN:
			cflag = (unsigned int) ctlmsg->message[5];
			break;
#endif

		default:
			stat = -1;
			goto out;
			break;
	}

out:
	return (stat);
} /* end parse_ctl_message */

