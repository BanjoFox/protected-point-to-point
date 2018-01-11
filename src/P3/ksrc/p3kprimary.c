/**
 * \file p3kprimary.c
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
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <p><hr><hr>
 * \section P3KM_PRIMARY Primary P3 System Messages
 */

#define _p3k_PRIMARY_C	0
#include "p3kprimary.h"
#include "p3kshare.h"
#include "p3ksession.h"
#include "p3kcrypto.h"

/** The main primary structure */
p3pri_main *primain = NULL;

/** The list of commands sent between the system application
 * and the kernel module.
 * - Each command must have its index defined.
 * - The default is to assume that the ioctl command is
 *   configuration data to be parsed.
 */
char *p3cmdlist[] = {"INIT"};
#define p3CMD_INIT	0

/**
 * ioctl_cmd defines the format of the ioctl data passed between the user and kernel space
 * where the command meanings are:
 * - noop: Unused
 * - primarycfg: Configuration data for a P3 primary system
 * - secondarycfg: Configuration data for a P3 secondary system
 * - prihostcfg: Configuration data for a remote P3 primary system
 * - sechostcfg: Configuration data for a remote P3 secondary system
 * - newsession: Data for starting a new P3 session
 */
enum ioctl_cmd iocmd;

/** Working buffer */
char tbuf[4092], *p3buf = tbuf;
struct p3config *config = NULL;

/**
 * \par Function:
 * init_p3primary
 *
 * \par Description:
 * Initialize the P3 Primary module by creating the basic structures for
 * handling P3 primary connections.
 *
 * \par Inputs:
 * - ramdisk: The location of the RAM disk area.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_primary (unsigned char *ramdisk)
{
	int stat = 0;

	// Create main p3 primary data structure
	if ((primain = (p3pri_main *) p3calloc(sizeof(p3pri_main))) == NULL) {
		p3errmsg(p3MSG_CRIT, "init_p3primary: Failed to allocate main P3 primary data structure\n");
		stat = -1;
		goto out;
	}
	// Set default values
	primain->array_size = p3RMT_ARSZ;
	primain->port = p3PRI_PORT;

	// Initialize key server information
	primain->key_mgr.key_serv = (p3key_serv *) ramdisk;
	p3lock_init(primain->key_mgr.lock);

	// Initialize P3 routing
	if (init_p3net < 0) {
		stat = -1;
		goto out;
	}

out:
	if (stat > 0)
		stat = 0;
	return (stat);
} /* end init_p3primary */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <hr><b>init_p3primary: Failed to allocate main P3 primary data structure</b>
 * \par Description (CRIT):
 * The P3 primary attempts to allocate its main structure before any other
 * activity.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem.
 *
 * <hr><b>init_p3primary: Failed to allocate <i>ip version</i> route table</b>
 * \par Description (CRIT):
 * The P3 primary attempts to allocate the route table during initialization.
 * If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem.
 *
 */

/**
 * \par Function:
 * parse_p3cmd
 *
 * \par Description:
 * Determine how to handle an ioctl command from the system application.
 * The list of supported commands is defined in the p3cmdlist global array.
 *
 * \par Inputs:
 * - buffer: The ioctl command buffer, including data.
 * - size: The buffer size.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0: Error
 */
int parse_p3cmd(unsigned char *buffer, int size)
{
	int i, stat = 0;
	void *saddr;
	p3host *shost;

	for (i=0; i < sizeof(p3cmdlist); i++) {
		if (strncmp((char *)&buffer[i], p3cmdlist[i], strlen(p3cmdlist[i])) == 0)
			break;
	}

	switch (i) {
	case p3CMD_INIT:
p3errmsg(p3MSG_DEBUG, "Initialize session to original key\n");
		if (size < (strlen(p3cmdlist[i]) + sizeof(struct in_addr))) {
p3errmsg(p3MSG_DEBUG, "Invalid buffer size\n");
			stat = -1;
			goto out;
		}
		shost =  p3hosts;
		saddr = (void *)&buffer[strlen(p3cmdlist[i])];
		// TODO: Add support for IPv6
		while (shost != NULL) {
			if (memcmp(saddr, &shost->addr.v4, sizeof(struct in_addr)) == 0) {
				if (shost->session == NULL) {
p3errmsg(p3MSG_DEBUG, "No session\n");
					stat = -1;
					goto out;
				}
/* !!!!! Temporary ====> */
/* !!!!! Temporary ====> */
				for (i=0; i < 16; i++) {
					shost->session->keymgmt.dnewkey->key[i] = (i + 13) * 53;
					shost->session->keymgmt.cnewkey->key[i] = (i + 53) * 13;
				}
/* <==== Temporary !!!!! */
/* <==== Temporary !!!!! */
				if (p3_rekey(&shost->session->keymgmt) < 0) {
p3errmsg(p3MSG_DEBUG, "Invalid buffer size\n");
					stat = -1;
					goto out;
				}
/* !!!!! Temporary ====> */
				shost->session->rID0 = 1;
				shost->session->rID1 = 1;
/* <==== Temporary !!!!! */
				break;
			}
			shost = shost->next;
		}
		break;

	default:
		if ((stat = parse_p3data(buffer, size)) < 0) {
		}
	}

out:
	return(stat);

} /* end parse_p3cmd */

/**
 * \par Function:
 * parse_p3data
 *
 * \par Description:
 * Parse data sent from the P3 user space application in an ioctl command.
 * 
 * \par Inputs:
 * - buffer: The data to be parsed
 * - size: The size of the data buffer
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Critical error
 *   - >0 = Non-critical error
 */

int parse_p3data(unsigned char *buffer, int size)
{
	int i, j, idx, hdr, stat = 0, ioc_cmd;
	int zero[4] = {0, 0, 0, 0};
	unsigned long l;
	unsigned char *net, *mask;
	p3primarycfg pcfg;
	p3host *shost;
	p3sechostcfg shcfg;
	p3net *snet;
	p3subnetcfg sncfg;
	p3newsession nsess;
	struct timeval kern_tv, *now = &kern_tv;
p3errmsg(p3MSG_DEBUG, "Enter parse P3 data\n");

	if (primain == NULL) {
		p3errmsg(p3MSG_ERR, "parse_p3data: Primary structure is NULL\n");
		stat = -EINVAL;
		goto out;
	}
	if (size < 2) {
		sprintf(p3buf, "parse_p3data: Invalid ioctl buffer size: %d\n", size);
		p3errmsg(p3MSG_WARN, p3buf);
		stat = -EINVAL;
		goto out;
	}
	// Get ioctl command
	ioc_cmd = buffer[0];
	idx = 1;

	switch (ioc_cmd) {
	// Get local primary host configuration data
	case primarycfg:
		memcpy(&pcfg, &buffer[idx], sizeof(p3primarycfg));
		if (pcfg.array_size > 0)
			primain->array_size = pcfg.array_size;
		if (pcfg.port > 0)
			primain->port = pcfg.port;
		if (pcfg.flag > 0)
			primain->flag = pcfg.flag;
// !!! TEMPORARY !!!
// !!! TEMPORARY !!!
		if (primain->subnet == NULL && (primain->subnet =
				(p3net *) p3calloc(sizeof(p3net))) == NULL) {
			p3errmsg(p3MSG_CRIT, "parse_p3data: Failed to allocate local subnet structure\n");
			stat = -ENOMEM;
			goto out;
		}
// !!! TEMPORARY !!!
// !!! TEMPORARY !!!
		if (pcfg.flag & p3HST_IPV4) {
			memcpy(&primain->addr.v4, &pcfg.addr.v4, sizeof(struct in_addr));
			memcpy(&primain->subnet->net.v4, &pcfg.subnet.v4,
					sizeof(struct in_addr));
			primain->subnet->flag |= p3HST_IPV4;
		} else if (pcfg.flag & p3HST_IPV6) {
			memcpy(&primain->addr.v6, &pcfg.addr.v6, sizeof (struct in6_addr));
			primain->subnet->flag |= p3HST_IPV6;
			// Get subnet list
		} else {
			p3errmsg(p3MSG_ERR, "parse_p3data: Primary IP version unsupported\n");
			stat = -EINVAL;
			goto out;
		}
		break;

	// Get remote secondary P3 host configuration data
	case secondaryhostcfg:
		memcpy (&shcfg, &buffer[idx], sizeof(p3sechostcfg));
		idx += sizeof(p3sechostcfg);
		// TODO: Add IPv6 support
		if (!(shcfg.flag & p3HST_IPV4)) {
			p3errmsg(p3MSG_WARN, "parse_p3data: Secondary IP version unsupported\n");
			stat = -EINVAL;
			goto out;
		}
		// TODO: Convert to Red Black trees
		shost = p3hosts;
		while (shost != NULL) {
			if (shcfg.flag & p3HST_IPV4 && memcmp(&shost->addr.v4,
						&shcfg.addr.v4, sizeof (struct in_addr)) == 0) {
				break;
			} else if (shcfg.flag & p3HST_IPV6 && memcmp(&shost->addr.v6,
						&shcfg.addr.v6, sizeof (struct in6_addr)) == 0) {
				break;
			}
			shost = shost->hlist;
		}
		// Add new host definition to host list.
		// TODO: Implement lock
		if (shost == NULL) {
			if (shcfg.flag & p3HST_IPV4) {
				hdr = p3SESSION_HDR4;
			} else if (shcfg.flag & p3HST_IPV6) {
				hdr = p3SESSION_HDR6;
			}
			i = sizeof(p3host) + p3SESSION_SIZE + hdr + (shcfg.subnetsz * sizeof(p3net));
			if ((shost = (p3host *) p3calloc(i)) == NULL) {
				p3errmsg(p3MSG_CRIT, "parse_p3data: Failed to allocate secondary host structure\n");
				stat = -ENOMEM;
				goto out;
			}
			// TODO: Convert remote host table to Red Black tree
			shost->hlist = p3hosts;
			shost->port = primain->port;
			p3hosts = shost;
		}
		if (shcfg.flag & p3HST_IPV4) {
			if (memcmp((void *)&zero[0], &shcfg.addr.v4,
					sizeof(struct in_addr)) != 0)
				memcpy(&shost->addr.v4, &shcfg.addr.v4, sizeof(struct in_addr));
		} else if (shcfg.flag & p3HST_IPV6) {
			if (memcmp((void *)&zero[0], &shcfg.addr.v6,
					sizeof(struct in6_addr)) != 0)
				memcpy(&shost->addr.v6, &shcfg.addr.v6, sizeof(struct in6_addr));
		}
		// Initialize secondary host
		if (shcfg.hb_wait > 0)
			shost->hb_wait = shcfg.hb_wait;
		if (shcfg.hb_fail > 0)
			shost->hb_fail = shcfg.hb_fail;
		if (shcfg.flag > 0)
			shost->flag = shcfg.flag;
		l = (unsigned long) shost + sizeof(p3host);
		// Initialize session
		shost->session = (p3session *) l;
		shost->session->host = shost;
		if (shcfg.rk_wait > 0)
			shost->session->rk_wait = shcfg.rk_wait;
		if (shcfg.ditime > 0)
			shost->session->ditime = shcfg.ditime;
		if (shcfg.citime > 0)
			shost->session->citime = shcfg.citime;
		init_session(shost, &primain->addr.v4);
		l += p3SESSION_SIZE + hdr;
		// Initialize subnets
		snet = (p3net *) l;
		shost->subnet = snet;
		shost->flag |= shcfg.subnetsz << p3HST_SNSHF;
		for (i=0; i < shcfg.subnetsz; i++) {
			memcpy(&sncfg, &buffer[idx], sizeof(p3subnetcfg));
			idx += sizeof(p3subnetcfg);
			if (shcfg.flag & p3HST_IPV4) {
				memcpy(&snet->net.v4, &sncfg.net.v4, sizeof(struct in_addr));
				memcpy(&snet->mask, &sncfg.mask, sizeof(struct in_addr));
				snet->flag |= p3HST_IPV4;
				// Clear host bits to be sure
				net = (unsigned char *)&snet->net.v4;
				mask = (unsigned char *)&snet->mask;
				for (j=0; j < sizeof(struct in_addr); j++) {
					net[j] &= mask[j];
				}
				if (build_p3table(snet, p3HST_IPV4) < 0) {
					// TODO: Handle cleanup
					stat = -1;
					goto out;
				}
			} else if (shcfg.flag & p3HST_IPV6) {
				memcpy(&snet->net.v6, &sncfg.net.v6, sizeof(struct in6_addr));
				snet->flag |= p3HST_IPV6;
				if (build_p3table(snet, p3HST_IPV6) < 0) {
					// TODO: Handle cleanup
					stat = -1;
					goto out;
				}
			}
			snet->host = shost;
			// Set P3 host network information
			if (shcfg.flag & p3HST_IPV4) {
				if (memcmp(&shost->addr.v4, &snet->net.v4,
						sizeof(struct in_addr)) == 0)
					shost->net = snet;
			} else if (shcfg.flag & p3HST_IPV6) {
				if (memcmp(&shost->addr.v6, &snet->net.v6,
						sizeof(struct in6_addr)) == 0)
					shost->net = snet;
			}
			l += sizeof(p3net);
			snet = (p3net *) l;
		}
/* !!!!! Temporary !!!!! */
/* !!!!! Temporary !!!!! */
p3errmsg(p3MSG_DEBUG, "Initialize session for unit testing\n");
	for (i=0; i < 16; i++) {
		shost->session->keymgmt.dnewkey->key[i] = (i + 13) * 53;
		shost->session->keymgmt.cnewkey->key[i] = (i + 53) * 13;
	}
	if (p3_init_crypto(&shost->session->keymgmt) < 0) {
		stat = -1;
		goto out;
	}
/* !!!!! Temporary !!!!! */
/* !!!!! Temporary !!!!! */
		break;

	case newsession:
		memcpy (&nsess, &buffer[idx], sizeof(p3newsession));
		// TODO: Add IPv6 support
		if (!(nsess.flag & p3HST_IPV4)) {
			p3errmsg(p3MSG_WARN, "parse_p3data: Secondary IP version unsupported\n");
			stat = -EINVAL;
			goto out;
		}
		// Find session and set active
		// TODO: Convert to Red Black trees
		shost = p3hosts;
		while (shost != NULL) {
			if (nsess.flag & p3HST_IPV4 &&
					memcmp(&shost->addr.v4, &nsess.addr.v4, sizeof (struct in_addr)) == 0) {
				break;
			} else if (nsess.flag & p3HST_IPV6 &&
					memcmp(&shost->addr.v6, &nsess.addr.v6, sizeof (struct in6_addr)) == 0) {
				break;
			}
			shost = shost->hlist;
		}
		if (shost == NULL) {
			p3errmsg(p3MSG_ERR, "parse_p3data: Secondary host undefined\n");
			stat = -EINVAL;
			goto out;
		}
		// Get new data and control keys
		i = p3_get_key_size(shost->flag & p3PSS_KTYPE);
		memcpy(shost->session->keymgmt.dnewkey->key, nsess.datakey, i);
		memcpy(shost->session->keymgmt.cnewkey->key, nsess.ctlkey, i);
		if (p3_init_crypto(&shost->session->keymgmt) < 0) {
			stat = -EPERM;
			goto out;
		}
		// Put host on timer queue
		// TODO: Implement lock
		do_gettimeofday(now);
		shost->session->rekey = now->tv_sec + shost->session->rk_wait;
		shost->session->dikey = now->tv_sec + shost->session->ditime;
		shost->session->cikey = now->tv_sec + shost->session->citime;
		break;

	default:
		sprintf(p3buf, "parse_p3data: Invalid ioctl buffer command: %d\n", ioc_cmd);
		p3errmsg(p3MSG_WARN, p3buf);
		stat = -EINVAL;
		goto out;
	}

out:
p3errmsg(p3MSG_DEBUG, "Exit parse P3 data\n");
	return(stat);
} /* end parse_p3data */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <hr><b>parse_p3data: Primary strucutre is NULL</b>
 * \par Description (ERR):
 * The primary configuration structure was not initialized by the
 * kernel module.
 * \par Response:
 * See previous error messages.  If there are none, notify the P3
 * application support.
 *
 * <hr><b>parse_p3data: Invalid ioctl buffer size: <i>size</i></b>
 * \par Description (WARN):
 * The data sent by the user space application was not a valid size.
 * \par Response:
 * Notify the P3 application support.
 *
 * <hr><b>parse_p3data: Invalid ioctl buffer command: <i>command</i></b>
 * \par Description (WARN):
 * The command sent by the user space application was not valid.
 * \par Response:
 * Notify the P3 application support.
 *
 * <hr><b>parse_p3data: <i>P3 type</i> IP version unsupported</b>
 * \par Description (WARN):
 * The IP version of the P3 host was not valid.
 * \par Response:
 * Correct the configuration data.
 *
 * <hr><b>parse_p3data: Failed to allocate secondary host structure</b>
 * \par Description (CRIT):
 * The P3 primary attempts to allocate a secondary host structure for each
 * defined secondary host.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem.
 *
 * <hr><b>parse_p3data: Secondary host undefined</b>
 * \par Description (ERR):
 * A P3 secondary has attempted to establish a connection, but there is no
 * definition for it in the kernel module.
 * \par Response:
 * Notify the P3 application support.
 *
 */

/**
 * \par Function:
 * start_rekeying
 *
 * \par Description:
 * Start the rekeying control sequence
 * 
 * \par Inputs:
 * - p3sess: The remote P3 host session information
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Critical error
 *   - >0 = Non-critical error
 */

void start_rekeying(p3session *session)
{
	p3ctlmsg *cmsg;

	if ((cmsg = build_newkey_message((session->flag & p3PSS_KTYPE),
			session, &primain->key_mgr)) == NULL) {
		goto out;
	}
	// Only set the Rekey flag once
	p3lock(session->lock);
	if (session->flag & p3PSS_REKEY) {
		p3unlock(session->lock);
		goto out;
	}
	session->flag |= p3PSS_REKEY;
	p3unlock(session->lock);
	if (p3send_control(session, cmsg) < 0) {
		p3lock(session->lock);
		session->flag &= ~p3PSS_REKEY;
		p3unlock(session->lock);
		goto out;
	}
p3errmsg(p3MSG_DEBUG, "Rekey success\n");

out:
	return;
}

