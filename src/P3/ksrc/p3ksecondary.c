/**
 * \file p3ksecondary.c
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
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <p><hr><hr>
 * \section P3KM_SECONDARY Secondary P3 System Messages
 */

#define _p3k_SECONDARY_C	0
#include "p3ksecondary.h"
#include "p3ksession.h"
#include "p3kshare.h"

/** The main secondary structure */
p3sec_main *secmain = NULL;

/** The list of commands sent between the system application
 * and the kernel module.
 * - Each command must have its index defined.
 * - The default is to assume that the ioctl command is
 *   configuration data to be parsed.
 */
char *p3cmdlist[] = {"NA"};
#define p3CMD_NONE	-1

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

/**
 * \par Function:
 * init_p3secondary
 *
 * \par Description:
 * Initialize the P3 Primary module by creating the basic structures for
 * handling P3 secondary connections.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_secondary ()
{
	int stat = 0;

	// Create main p3 secondary data structure
	if ((secmain = (p3sec_main *) p3calloc(sizeof(p3sec_main))) == NULL) {
		p3errmsg(p3MSG_CRIT, "init_p3secondary: Failed to allocate main p3 secondary data structure\n");
		stat = -1;
		goto out;
	}

	// Initialize P3 routing
	if (init_p3net < 0) {
		stat = -1;
		goto out;
	}

out:
	if (stat > 0)
		stat = 0;
	return (stat);
} /* end init_p3secondary */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <hr><b>init_p3secondary: Failed to allocate main p3 secondary data structure</b>
 * \par Description (CRIT):
 * The P3 secondary attempts to allocate its main structure before any other
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

	for (i=0; i < sizeof(p3cmdlist); i++) {
		if (strncmp((char *)&buffer[i], p3cmdlist[i], strlen(p3cmdlist[i])) == 0)
			break;
	}

	switch (i) {
	case p3CMD_NONE:
		break;

	default:
		if ((stat = parse_p3data((unsigned char *) buffer, size)) < 0) {
			goto out;
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
 * Parse the P3 Secondary configuration file.
 * 
 * \par Inputs:
 * - buffer: The data to be parsed
 * - size: The size of the data buffer
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 *   - >0 = Config does not exist, use defaults
 */

int parse_p3data(unsigned char *buffer, int size)
{
	int i, j, idx, hdr, stat = 0, ioc_cmd;
	unsigned long l;
	unsigned char *net, *mask;
	p3secondarycfg scfg;
	p3host *phost;
	p3prihostcfg phcfg;
	p3net *snet;
	p3subnetcfg sncfg;
p3errmsg(p3MSG_DEBUG, "Enter parse P3 data\n");

	if (secmain == NULL) {
		p3errmsg(p3MSG_ERR, "parse_p3data: Secondary structure is NULL\n");
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
	// Get local secondary host configuration data
	case secondarycfg:
		memcpy (&scfg, &buffer[idx], sizeof(p3secondarycfg));
		if (scfg.flag > 0)
			secmain->flag = scfg.flag;
// !!! TEMPORARY !!!
// !!! TEMPORARY !!!
		if ((secmain->subnet = (p3net *) p3calloc(sizeof(p3net))) == NULL) {
			p3errmsg(p3MSG_CRIT, "parse_p3data: Failed to allocate local subnet structure\n");
			stat = -ENOMEM;
			goto out;
		}
// !!! TEMPORARY !!!
// !!! TEMPORARY !!!
		if (scfg.flag & p3HST_IPV4) {
			memcpy(&secmain->addr.v4, &scfg.addr.v4, sizeof(struct in_addr));
			memcpy(&secmain->subnet->net.v4, &scfg.subnet.v4, sizeof(struct in_addr));
			secmain->subnet->flag |= p3HST_IPV4;
		} else if (scfg.flag & p3HST_IPV6) {
			memcpy(&secmain->addr.v6, &scfg.addr.v6, sizeof (struct in6_addr));
			secmain->subnet->flag |= p3HST_IPV6;
			// Get subnet list
		} else {
			p3errmsg(p3MSG_ERR, "parse_p3data: Secondary IP version unsupported\n");
			stat = -EINVAL;
			goto out;
		}
		break;

	// Get remote secondary P3 host configuration data
	case primaryhostcfg:
		memcpy (&phcfg, &buffer[idx], sizeof(p3prihostcfg));
		idx += sizeof(p3prihostcfg);
		// TODO: Add IPv6 support
		if (!(phcfg.flag & p3HST_IPV4)) {
			p3errmsg(p3MSG_WARN, "parse_p3data: Primary IP version unsupported\n");
			stat = -EINVAL;
			goto out;
		}
		// TODO: Convert to Red Black trees
		phost = p3hosts;
		while (phost != NULL) {
			if (phcfg.flag & p3HST_IPV4 &&
					memcmp(&phost->addr.v4, &phcfg.addr.v4, sizeof (struct in_addr)) == 0) {
				break;
			} else if (phcfg.flag & p3HST_IPV6 &&
					memcmp(&phost->addr.v6, &phcfg.addr.v6, sizeof (struct in6_addr)) == 0) {
				break;
			}
			phost = phost->hlist;
		}
		// Add new host definition to host list.
		// TODO: Implement lock
		if (phost == NULL) {
			if (phcfg.flag & p3HST_IPV4) {
				hdr = p3SESSION_HDR4;
			} else if (phcfg.flag & p3HST_IPV6) {
				hdr = p3SESSION_HDR6;
			}
			i = sizeof(p3host) + p3SESSION_SIZE + hdr + (phcfg.subnetsz * sizeof(p3net));
			if ((phost = (p3host *) p3calloc(i)) == NULL) {
				p3errmsg(p3MSG_CRIT, "parse_p3data: Failed to allocate primary host structure\n");
				stat = -ENOMEM;
				goto out;
			}
			// TODO: Convert remote host table to Red Black tree
			phost->hlist = p3hosts;
			p3hosts = phost;
		}
		if (phcfg.flag & p3HST_IPV4) {
			memcpy(&phost->addr.v4, &phcfg.addr.v4, sizeof(struct in_addr));
		} else if (phcfg.flag & p3HST_IPV6) {
			memcpy(&phost->addr.v6, &phcfg.addr.v6, sizeof(struct in6_addr));
		}
		// Initialize primary host
		if (phcfg.port > 0)
			phost->port = phcfg.port;
		if (phcfg.flag > 0)
			phost->flag = phcfg.flag;
		l = (unsigned long) phost + sizeof(p3host);
		// Initialize session
		// Note: Primary session initialization completed
		//       when 1st rekey command received
		phost->session = (p3session *) l;
		phost->session->host = phost;
		l += p3SESSION_SIZE + hdr;
		// Initialize subnets
		snet = (p3net *) l;
		phost->subnet = snet;
		phost->flag |= phcfg.subnetsz << p3HST_SNSHF;
		for (i=0; i < phcfg.subnetsz; i++) {
			memcpy(&sncfg, &buffer[idx], sizeof(p3subnetcfg));
			idx += sizeof(p3subnetcfg);
			if (phcfg.flag & p3HST_IPV4) {
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
			} else if (phcfg.flag & p3HST_IPV6) {
				memcpy(&snet->net.v6, &sncfg.net.v6, sizeof(struct in6_addr));
				snet->flag |= p3HST_IPV6;
				if (build_p3table(snet, p3HST_IPV6) < 0) {
					// TODO: Handle cleanup
					stat = -1;
					goto out;
				}
			}
			snet->host = phost;
			// Set P3 host network information
			if (phcfg.flag & p3HST_IPV4) {
				if (memcmp(&phost->addr.v4, &snet->net.v4,
						sizeof(struct in_addr)) == 0)
					phost->net = snet;
			} else if (phcfg.flag & p3HST_IPV6) {
				if (memcmp(&phost->addr.v6, &snet->net.v6,
						sizeof(struct in6_addr)) == 0)
					phost->net = snet;
			}
			l += sizeof(p3net);
			snet = (p3net *) l;
		}
/* !!!!! Temporary !!!!! */
/* !!!!! Temporary !!!!! */
p3errmsg(p3MSG_DEBUG, "Initialize session for unit testing\n");
	phost->flag |= p3KTYPE_AES128 << p3HST_KTSHF;
	init_session(phost, &secmain->addr.v4);
	for (i=0; i < 16; i++) {
		phost->session->keymgmt.dnewkey->key[i] = (i + 13) * 53;
		phost->session->keymgmt.cnewkey->key[i] = (i + 53) * 13;
	}
	if (p3_init_crypto(&phost->session->keymgmt) < 0) {
		stat = -1;
		goto out;
	}
/* !!!!! Temporary !!!!! */
/* !!!!! Temporary !!!!! */
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
 * <hr><b>parse_p3data: Secondary structure is NULL</b>
 * \par Description (ERR):
 * The secondary configuration structure was not initialized by the
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
 * <hr><b>parse_p3data: Failed to allocate primary host structure</b>
 * \par Description (CRIT):
 * The P3 primary attempts to allocate a primary host structure for each
 * defined primary host.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem.
 *
 */

