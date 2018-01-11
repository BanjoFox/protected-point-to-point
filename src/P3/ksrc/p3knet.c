/**
 * \file p3knet.c
 * <h3>Protected Point to Point template file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The network functions include:
 * - Routing to P3 hosts
 * - Encrypting packets and encapsulating them in a modified IPSec
 *   packet
 */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <p><hr><hr>
 * \section P3KM_NET P3 Network Kernel Module Messages
 */

#define _p3kNET_C
#include "p3knet.h"
#ifndef _p3_SECONDARY
#include "p3kprimary.h"
#endif
#ifndef _p3_PRIMARY
#include "p3ksecondary.h"
#endif
#include "p3ksession.h"

/** The primary P3 route tables */
p3route *ipv4route = NULL;
p3route *ipv6route = NULL;
/** The remote host table */
// TODO: Convert remote host table to Red Black tree
p3host *p3hosts = NULL;

/**
 * \par Function:
 * init_p3net
 *
 * \par Description:
 * Initialize the network functionality.
 *
 * The P3 routing table is used to determine if a packet is to be
 * encrypted and forwarded to a remote P3 system.  There is a
 * configuration file for all pre-defined remote P3 systems.  However,
 * there must be a mechanism for dynamic configurations.  The
 * requirements for this are:
 * - Administrator approval
 * - Automation where possible
 * - Immediate use by the remote P3 system
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_p3net(void)
{
	int i, stat = 0;
	unsigned long l;

	// TODO: Build Red Black tree
	// Create P3 route tables
	i = sizeof(p3route) + (P3MAXROUTE * sizeof(p3net *));
	if ((ipv4route = (p3route *) p3calloc(i)) == NULL) {
		p3errmsg(p3MSG_CRIT, "init_p3primary: Failed to allocate IPv4 route table\n");
		stat = -1;
		goto out;
	}
	l = (unsigned long) ipv4route + sizeof(p3route);
	ipv4route->nets = (p3net **) l;

	i = sizeof(p3route) + (P3MAXROUTE * sizeof(p3net *));
	if ((ipv6route = (p3route *) p3calloc(i)) == NULL) {
		p3errmsg(p3MSG_CRIT, "init_p3primary: Failed to allocate IPv6 route table\n");
		stat = -1;
		goto out;
	}
	l = (unsigned long) ipv6route + sizeof(p3route);
	ipv6route->nets = (p3net **) l;

out:
	return (stat);
} /* end init_net */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <hr><b>init_net: Failed to allocate P3 route table structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 kernel module attempts to allocate the P3 route table during
 * initialization.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * build_p3table
 *
 * \par Description:
 * Build the P3 routing table.
 * 
 * \par Inputs:
 * - net: A network destination for remote P3 hosts.
 * - ipver: The IP version of the network being defined.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 *   - >0 = Warning
 */

int build_p3table(p3net *net, int ipver)
{
	int i, stat = 0;

p3errmsg(p3MSG_DEBUG, "Enter build P3 table\n");
	// Check for network already defined
	if (ipver == p3HST_IPV4) {
		// TODO: Build Red Black tree
		if (ipv4route->netsz == P3MAXROUTE) {
			p3errmsg(p3MSG_WARN, "build_p3table: Maximum routes exceeded\n");
			stat = 1;
			goto out;
		}
		for (i=0; i < ipv4route->netsz; i++) {
			// TODO: Put host definition before subnet definition
			if (memcmp(&ipv4route->nets[i]->net.v4, &net->net.v4, sizeof(struct in_addr)) == 0 &&
				memcmp(&ipv4route->nets[i]->mask, &net->mask, sizeof(struct in_addr)) == 0) {
				p3errmsg(p3MSG_WARN, "build_p3table: Route already exists\n");
				stat = 1;
				goto out;
			}
		}
		ipv4route->nets[ipv4route->netsz++] = net;
	}
	// TODO: Build IPv6 P3 table

out:
p3errmsg(p3MSG_DEBUG, "Exit build P3 table\n");
	return (stat);
} /* end build_p3table */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <hr><b>build_p3table: Maximum routes exceeded</b>
 * \par Description (WARN):
 * The P3 kernel module has attempted to add a new route to the P3 route table.
 * \par Response:
 * Report the problem to Velocite Systems support.
 *
 * <hr><b>build_p3table: Route already exists</b>
 * \par Description (WARN):
 * The P3 kernel module has attempted to add a new route to the P3 route table
 * but found both the network and mask.
 * \par Response:
 * Verify the configuration and correct if appropriate.
 *
 */

/**
 * \par Function:
 * set_net_active
 *
 * \par Description:
 * Set or reset the network Active flag.
 * 
 * \par Inputs:
 * - host: A P3 host definition
 * - act: Flag where 1 = active, 0 = inactive
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int set_net_active(p3host *host, int act)
{
	int i, stat = 0;

	if (host->subnet == NULL ||
			((host->flag & p3HST_SNETS) >> p3HST_SNSHF) == 0) {
		stat = -1;
		goto out;
	}

	for (i=0; i < (host->flag & p3HST_SNETS) >> p3HST_SNSHF; i++) {
		if (act)
			host->subnet[i].flag |= p3NET_ACT;
		else
			host->subnet[i].flag &= ~p3NET_ACT;
	}

out:
	return (stat);
} /* end set_net_active */

#define PW pkt->work

/**
 * \par Function:
 * packet_handler
 *
 * \par Description:
 * Determine what action to perform on an intercepted packet.
 * 
 * - If source is another P3 system, remove the ESP enhanced header
 *   - Get the appropriate key and decrypt the packet
 *     - If destination is this P3 system, decrypt and handle control message
 *     - Else, send the packet to the appropriate interface
 * - Else, check P3 tree for destination
 *   - If the destination is another P3 system
 *     - If the packet is TCP SYN, make sure the MSS allows for P3 header requirements
 *     - Encrypt the packet, add the P3 header and return the packet to the stack
 *   - If destination is not another P3 system, return the packet to the stack
 *
 * \par Inputs:
 * - pkt: A p3packet structure containing information about the packet.
 * - p3sys_net: The system network structure pointer.  This is passed as
 *   void pointer and cast appropriately in the callback, p3net_utils.
 *
 * \par Outputs:
 * - int: Packet status
 *   - p3PKTS_NOMOD = Unmodified
 *   - p3PKTS_ADDHDR = Header added (implies encryption)
 *   - p3PKTS_RMVHDR = Header removed (implies decryption)
 *   - p3PKTS_CONTROL = Control packet handled by P3 processing
 *   - p3PKTS_RAWSOCK = Packet is establishing a Raw socket
 */

int packet_handler(p3packet *pkt, void *p3sys_net)
{
	int stat = 0, decode_dat, decode_ctl, addmss;
	unsigned int sseq;
	struct iphdr *iph;
	struct tcphdr *tcph;
	unsigned char *bufp;
// TODO: Support IPv6 in packet handler

	p3_lookup(pkt);
/***
 *** Packet source is local subnet, it will be intercepted again
 ***/
	if (pkt->flag & p3PKT_SRSUB) {
		goto out;
/***
 *** Source is P3 host
 ***/
	} else if (pkt->flag & p3PKT_P3SRC) {
sprintf(p3buf, "P3 source net found: pkt %p, net %p, flag %x\n",
	pkt->packet, pkt->net, pkt->flag);
p3errmsg(p3MSG_DEBUG, p3buf);
		// Network is not active, return packet to stack
		if (!(pkt->net->flag & p3NET_ACT)) {
sprintf(p3buf, "P3 session not active: %2.2x%2.2x%2.2x%2.2x\n",
	pkt->packet[12], pkt->packet[13], pkt->packet[14], pkt->packet[15]);
p3errmsg(p3MSG_DEBUG, p3buf);
			goto out;
		}
		// Session is not a P3 session
		iph = (struct iphdr *) pkt->packet;
		if (iph->protocol != p3PROTO) {
sprintf(p3buf, "Protocol not P3: %d\n", iph->protocol);
p3errmsg(p3MSG_DEBUG, p3buf);
			goto out;
		}
		// Pass initialization connection
		iph = (struct iphdr *)pkt->packet;
		if (iph->protocol == 6) {
			decode_ctl = iph->ihl << 2;
			tcph = (struct tcphdr *)&pkt->packet[decode_ctl];
#ifndef _p3_SECONDARY
			decode_ctl = ntohs(tcph->dest);
#else
			decode_ctl = ntohs(tcph->source);
#endif
			if (tcph->syn && decode_ctl == pkt->net->host->port) {
p3errmsg(p3MSG_DEBUG, "P3 session init\n");
				goto out;
			}
		}
		// Set OS dependent inbound network info for remote P3 host
		if ((pkt->flag & p3PKT_SRP3) && !(pkt->net->flag & p3NET_DEVI)) {
			if (p3net_utils(p3SET_DEVIN, p3sys_net, pkt) < 0) {
				sprintf(p3buf, "packet_handler: System network utility\
 failed: %d\n", p3SET_DEVIN);
				p3errmsg(p3MSG_ERR, p3buf);
				stat = -1;
				goto out;
			}
			pkt->net->flag |= p3NET_DEVI;
		}
		// Get work space with 2 data buffers
		addmss = sizeof(p3work) + 
				(((pkt->flag & p3PKT_SIZE) - p3SESSION_HDR4) << 1);
		if ((pkt->work = (p3work *) p3malloc(addmss)) == NULL) {
			p3errmsg(p3MSG_CRIT, "packet_handler: Failed to allocate P3 packet buffer\n");
			stat = -1;
			goto out;
		}
strcpy(p3buf, "P3 hdr:");
for (PW->i1=0; PW->i1 < p3SESSION_HDR4; PW->i1++) {
	if (!(PW->i1 & 3))
		sprintf(p3buf, "%s ", p3buf);
	sprintf(p3buf, "%s%2.2x", p3buf, pkt->packet[PW->i1]);
}
sprintf(p3buf, "%s\n", p3buf);
p3errmsg(p3MSG_DEBUG, p3buf);
		memset(pkt->work, 0, sizeof(p3work));
		PW->l = (unsigned long) PW + sizeof(p3work);
		PW->newbuf = (unsigned char *) PW->l;
		PW->l += (pkt->flag & p3PKT_SIZE) - p3SESSION_HDR4;
		PW->buf = (unsigned char *) PW->l;
		PW->newlen = (pkt->flag & p3PKT_SIZE) - p3SESSION_HDR4;
		memcpy(PW->newbuf, &pkt->packet[p3SESSION_HDR4], PW->newlen);
		// Use P3 sequence number to choose encryption key
		sseq = (unsigned int) pkt->packet[p3SESSION_HDR4 - 4];
		sseq <<= 8;
		sseq |= (unsigned int) pkt->packet[p3SESSION_HDR4 - 3];
		sseq <<= 8;
		sseq |= (unsigned int) pkt->packet[p3SESSION_HDR4 - 2];
		sseq <<= 8;
		sseq |= (unsigned int) pkt->packet[p3SESSION_HDR4 - 1];
		if (pkt->host->session->rID1 <= sseq ||
				sseq < pkt->host->session->rID0) {
			decode_dat = p3DATDEC1;
			decode_ctl = p3CTLDEC1;
		} else {
			decode_dat = p3DATDEC0;
			decode_ctl = p3CTLDEC0;
		}
		// Decrypt original packet
sprintf(p3buf, "Decrypt packet: Seq %d\n", sseq);
p3errmsg(p3MSG_DEBUG, p3buf);
		if (p3_decrypt(PW->newbuf, PW->newlen, sseq,
				decode_dat, &pkt->host->session->keymgmt) < 0) {
p3errmsg(p3MSG_DEBUG, "Decryption error\n");
			stat = -1;
			goto out;
		}
		pkt->packet = PW->newbuf;
		pkt->flag &= ~p3PKT_SIZE;
		pkt->flag |= PW->newlen;
sprintf(p3buf, "Deobfuscate packet: New Len %d\n", PW->newlen);
p3errmsg(p3MSG_DEBUG, p3buf);
		if (deobfuscate(pkt) < 0) {
			stat = -1;
			goto out;
		}
		iph = (struct iphdr *)pkt->packet;
		// Handle control message
		if (iph->protocol == 17) {
			PW->i1 = iph->ihl << 2;
			PW->udph = (struct udphdr *)&PW->newbuf[PW->i1];
#ifndef _p3_SECONDARY
			PW->i1 = ntohs(PW->udph->dest);
			bufp = (char *) &primain->addr.v4;
#else
			PW->i1 = ntohs(PW->udph->source);
			bufp = (char *) &secmain->addr.v4;
#endif
			if (memcmp(&PW->newbuf[p3IP4_DADDR], bufp, sizeof(struct in_addr))
					== 0 && PW->i1 == pkt->host->port) {
				// Get length of encrypted data (multiple of 16)
p3errmsg(p3MSG_DEBUG, "Decrypt control packet\n");
sprintf(p3buf, "Control Pkt: Len %d Seq %d\n", ntohs(PW->udph->len), sseq);
for (PW->i1=0; PW->i1 < ntohs(iph->tot_len); PW->i1++) {
	if (!(PW->i1 & 3))
		sprintf(p3buf, "%s ", p3buf);
	if (!(PW->i1 & 15))
		sprintf(p3buf, "%s\n  ", p3buf);
	sprintf(p3buf, "%s%2.2x", p3buf, PW->newbuf[PW->i1]);
}
sprintf(p3buf, "%s\n", p3buf);
p3errmsg(p3MSG_DEBUG, p3buf);
				PW->i1 = ntohs(PW->udph->len);
				if (p3_decrypt(&PW->newbuf[p3CONTROL_HDR4], PW->i1, sseq,
						decode_ctl, &pkt->host->session->keymgmt) < 0) {
p3errmsg(p3MSG_DEBUG, "Control decryption error\n");
					stat = -1;
					goto out;
				}
				PW->ctlmsg.message = &PW->newbuf[p3SESSION_HDR4];
				PW->ui1 = (unsigned int) PW->ctlmsg.message[0];
				PW->ui1 <<= 8;
				PW->ui1 |= (unsigned int) PW->ctlmsg.message[1];
				PW->ui1 <<= 8;
				PW->ui1 |= (unsigned int) PW->ctlmsg.message[2];
				PW->ui1 <<= 8;
				PW->ui1 |= (unsigned int) PW->ctlmsg.message[3];
				PW->ctlmsg.len = PW->ui1;
if (pkt->flag & p3PKT_DSSUB)
	pkt->host->session->flag |= p3PSS_CFWD;
else
	pkt->host->session->flag &= ~p3PSS_CFWD;
				if (parse_ctl_message(&PW->ctlmsg, pkt->host->session) < 0) {
p3errmsg(p3MSG_DEBUG, "Control parsing error\n");
					// Clear rekeying just in case
					pkt->host->session->flag &= ~p3PSS_REKEY;
					stat = -1;
					goto out;
				}
p3errmsg(p3MSG_DEBUG, "Control packet handled\n");
				stat = p3PKTS_CONTROL;
				goto out;
			}
		}
		// Determine final destination of this P3 host or local subnet
#ifndef _p3_SECONDARY
		bufp = (char *) &primain->addr.v4;
#else
		bufp = (char *) &secmain->addr.v4;
#endif
		if (memcmp(&iph->daddr, bufp, sizeof(struct in_addr)) == 0) {
			pkt->flag |= p3PKT_DSP3;
		} else {
			pkt->flag |= p3PKT_DSSUB;
#ifndef _p3_SECONDARY
			pkt->net = primain->subnet;
#else
			pkt->net = secmain->subnet;
#endif
		}
		// Return packet to stack to be forwarded to destination
		stat = p3PKTS_RMVHDR;
		// If packet is TCP SYN, do checksum for the stack
		if (iph->protocol == 6) {
			PW->idx1 = (pkt->packet[0] & 0xf) << 2;
			PW->tcph = (struct tcphdr *) &pkt->packet[PW->idx1];
			if (PW->tcph->syn) {
				// TODO: Do checksum for the stack
				stat |= p3PKTS_CHKSUM;
			}
		}
PW->newlen = pkt->flag & p3PKT_SIZE;
if (PW->newlen > 96)
	PW->ui1 = 96;
else
	PW->ui1 = PW->newlen;
sprintf(p3buf, "Pkt (Len %d):", PW->newlen);
for (PW->i1=0; PW->i1 < PW->ui1; PW->i1++) {
	if (!(PW->i1 & 3))
		sprintf(p3buf, "%s ", p3buf);
	if (!(PW->i1 & 15))
		sprintf(p3buf, "%s\n  ", p3buf);
	sprintf(p3buf, "%s%2.2x", p3buf, pkt->packet[PW->i1]);
}
sprintf(p3buf, "%s\n", p3buf);
p3errmsg(p3MSG_DEBUG, p3buf);
/***
 *** Destination is P3 subnet
 ***/
	} else if (pkt->flag & p3PKT_P3DST) {
sprintf(p3buf, "P3 dest net found: pkt %p, net %p, flag %x\n",
	pkt->packet, pkt->net, pkt->flag);
p3errmsg(p3MSG_DEBUG, p3buf);
		// Pass initialization connection
		iph = (struct iphdr *)pkt->packet;
		if (iph->protocol == 6) {
			decode_ctl = iph->ihl << 2;
			tcph = (struct tcphdr *)&pkt->packet[decode_ctl];
#ifndef _p3_SECONDARY
			decode_ctl = ntohs(tcph->source);
#else
			decode_ctl = ntohs(tcph->dest);
#endif
			if (tcph->syn && decode_ctl == pkt->net->host->port) {
p3errmsg(p3MSG_DEBUG, "P3 session init\n");
				goto out;
			}
		}
		// Get estimated packet length
		if (pkt->net->flag & p3HST_IPV4)
			decode_dat = p3SESSION_HDR4;
		else if (pkt->net->flag & p3HST_IPV6)
			decode_dat = p3SESSION_HDR6;
		else {
p3errmsg(p3MSG_DEBUG, "Err 1\n");
			stat = -1;
			goto out;
		}
// Set OS dependent inbound network info
		if (!(pkt->net->flag & p3NET_DEVO)) {
			if (p3net_utils(p3SET_DEVOUT, p3sys_net, pkt) < 0) {
				sprintf(p3buf, "packet_handler: System network utility\
 failed: %d\n", p3SET_DEVOUT);
				p3errmsg(p3MSG_ERR, p3buf);
				stat = -1;
				goto out;
			}
			pkt->net->flag |= p3NET_DEVO;
		}
		// Calculate new packet size (minimum of 2 obfuscation fields)
		decode_ctl = (pkt->flag & p3PKT_SIZE) + 6;
		if (decode_ctl <= p3PKT_SMALL)
			addmss = p3PKT_SMALL;
		else if (decode_ctl < p3PKT_MED)
			addmss = p3PKT_MED;
		else if (decode_ctl < p3PKT_LARGE)
			addmss = p3PKT_LARGE;
		else
			addmss = (decode_ctl + 0xf) & ~0xf;
		// Get work space with 2 data buffers
		addmss += decode_dat + 4;
		decode_dat = addmss;
		addmss <<= 1;
		addmss += sizeof(p3work);
		if ((pkt->work = (p3work *) p3malloc(addmss)) == NULL) {
			p3errmsg(p3MSG_CRIT, "packet_handler: Failed to allocate P3 packet buffer\n");
			stat = -1;
			goto out;
		}
		addmss = 0;
		memset(pkt->work, 0, sizeof(p3work));
		PW->l = (unsigned long) PW + sizeof(p3work);
		PW->newbuf = (unsigned char *) PW->l;
		PW->l += decode_dat;
		PW->buf = (unsigned char *) PW->l;
		// Check for packet establishing a raw socket (P3 counter = 0)
		PW->ui1 = (unsigned int) pkt->packet[p3SESSION_HDR4 - 4];
		PW->ui1 <<= 8;
		PW->ui1 |= (unsigned int) pkt->packet[p3SESSION_HDR4 - 3];
		PW->ui1 <<= 8;
		PW->ui1 |= (unsigned int) pkt->packet[p3SESSION_HDR4 - 2];
		PW->ui1 <<= 8;
		PW->ui1 |= (unsigned int) pkt->packet[p3SESSION_HDR4 - 1];
		iph = (struct iphdr *) pkt->packet;
// Set OS dependent network info from raw socket packet
		if (iph->protocol == p3PROTO && !PW->ui1) {
			if (!(pkt->net->flag & p3NET_RAW)) {
				if (p3net_utils(p3SET_RAW, p3sys_net, pkt) < 0) {
					sprintf(p3buf, "packet_handler: System network utility\
 failed: %d\n", p3SET_RAW);
					p3errmsg(p3MSG_ERR, p3buf);
					stat = -1;
					goto out;
				}
				if (pkt->net->host != NULL) {
					if (set_net_active(pkt->net->host, 1) < 0) {
						p3errmsg(p3MSG_ERR, "packet_handler: Error setting\
 subnets active\n");
						stat = -1;
						goto out;
					}
				} else
					pkt->net->flag |= p3NET_ACT;
				stat = p3PKTS_RAWSOCK;
			} else {
p3errmsg(p3MSG_DEBUG, "Err 2\n");
				stat = -1;
			}
			goto out;
		}
		if (!(pkt->net->flag & p3NET_ACT)) {
			p3free(pkt->work);
p3errmsg(p3MSG_DEBUG, "Net not active\n");
			goto out;
		}
PW->ui1 = ntohs(iph->tot_len);
sprintf(p3buf, "Pkt (Len %d):", PW->ui1);
if (PW->ui1 > 96)
	PW->ui1 = 96;
for (PW->i1=0; PW->i1 < PW->ui1; PW->i1++) {
	if (!(PW->i1 & 3))
		sprintf(p3buf, "%s ", p3buf);
	if (!(PW->i1 & 15))
		sprintf(p3buf, "%s\n  ", p3buf);
	sprintf(p3buf, "%s%2.2x", p3buf, pkt->packet[PW->i1]);
}
sprintf(p3buf, "%s\n", p3buf);
p3errmsg(p3MSG_DEBUG, p3buf);
		// If packet is TCP SYN, make sure MSS allows for P3 header requirements
		if (iph->protocol == 6) {
			PW->idx1 = (pkt->packet[0] & 0xf) << 2;
			PW->tcph = (struct tcphdr *) &pkt->packet[PW->idx1];
			// Check SYN for MSS
			if (PW->tcph->syn) {
				// Get start and end of TCP options
				bufp = &pkt->packet[PW->idx1];
				PW->tcph = (struct tcphdr *) bufp;
				PW->idx2 = PW->idx1 + ((bufp[12] & 0xf0) >> 2);
				bufp += 20;
				while (bufp < &pkt->packet[PW->idx2]) {
					if (bufp[0] < 2) {
						// Turn EOL into NOP in case MSS has to be added
						if (bufp[0] == 0)
							bufp[0] = 1;
						bufp++;
					} else if (bufp[0] != 2) {
						PW->l = (unsigned long) bufp + bufp[1];
						bufp = (unsigned char *) PW->l;
					// Reduce MSS to prevent IP fragmentation
					} else {
						PW->i1 = bufp[2];
						PW->i1 <<= 8;
						PW->i1 |= bufp[3];
						pkt->netdata = PW->i1 & 0xffff;
						if (pkt->net->flag & p3HST_IPV4)
							PW->idx1 = p3MSS_V4;
						else if (pkt->net->flag & p3HST_IPV6)
							PW->idx1 = p3MSS_V6;
						else {
p3errmsg(p3MSG_DEBUG, "Err 3\n");
							stat = -1;
							goto out;
						}
						if (PW->i1 > PW->idx1) {
							bufp[2] = (unsigned char) ((PW->idx1 >> 8) & 0xff);
							bufp[3] = (unsigned char) (PW->idx1 & 0xff);
							pkt->netdata |= PW->idx1 << 16;
							// Recalculate the TCP checksum
sprintf(p3buf, "Reduce MSS from %d to %d (%x)\n",
	PW->i1, PW->idx1, pkt->netdata);
p3errmsg(p3MSG_DEBUG, p3buf);
							if (p3net_utils(p3TCP_CHECK, p3sys_net, pkt) < 0) {
								sprintf(p3buf, "packet_handler: System network\
 utility failed: %d\n", p3TCP_CHECK);
								p3errmsg(p3MSG_ERR, p3buf);
								stat = -1;
								goto out;
							}
						}
						break;
					}
				}
				// MSS not already in TCP options
				if (bufp >= &pkt->packet[PW->idx2]) {
					if (p3net_utils(p3GET_MTU, p3sys_net, pkt) < 0) {
						sprintf(p3buf, "packet_handler: System network\
 utility failed: %d\n", p3GET_MTU);
						p3errmsg(p3MSG_ERR, p3buf);
						stat = -1;
						goto out;
					}
sprintf(p3buf, "Set MSS: MTU = %d\n", pkt->netdata);
p3errmsg(p3MSG_DEBUG, p3buf);
					if (pkt->net->flag & p3HST_IPV4)
						addmss = pkt->netdata - p3EXTRA_V4;
					else if (pkt->net->flag & p3HST_IPV6)
						addmss = pkt->netdata - p3EXTRA_V6;
					if (addmss < p3MSS)
						addmss = p3MSS;
				}
			}

		}
		// Set length of P3 header + packet (with possible MSS option addition)
		PW->i1 = ntohs(iph->tot_len) + 6;
		if (addmss)
			PW->i1 += 4;
		// Get tunneled (fixed) packet length
		if (pkt->net->flag & p3HST_IPV4)
			PW->idx1 = p3SESSION_HDR4;
		else if (pkt->net->flag & p3HST_IPV6)
			PW->idx1 = p3SESSION_HDR6;
		// Encrypted data size must be multiple of 16
		if (PW->i1 <= p3PKT_SMALL)
			PW->newlen = p3PKT_SMALL + PW->idx1;
		else if (PW->i1 < p3PKT_MED)
			PW->newlen = p3PKT_MED + PW->idx1;
		else if (PW->i1 < p3PKT_LARGE)
			PW->newlen = p3PKT_LARGE + PW->idx1;
		else
			PW->newlen = ((PW->i1 + 0xf) & ~0xf) + PW->idx1;
		if (PW->newlen > p3PKT_MAX) {
			sprintf(p3buf, "packet_handler: Packet too large (%d)\n", PW->newlen);
			p3errmsg(p3MSG_DEBUG, p3buf);
			stat = -1;
			goto out;
		}
		pkt->flag &= ~p3PKT_SIZE;
		pkt->flag |= PW->newlen;
		if (addmss) {
			// Copy original header
			PW->idx1 = (pkt->packet[0] & 0xf) << 2;		// IP header length
			if ((PW->i1 = (pkt->packet[PW->idx1 + 12] & 0xf0) >> 2) == 0x3c) {
				p3errmsg(p3MSG_WARN, "packet_handler: Cannot increase TCP options field\n");
				stat = -1;
				goto out;
			}
			PW->idx2 = PW->idx1 + PW->i1;	// IP + TCP header length
sprintf(p3buf, "Add MSS (%d) Idx %d, Endx %d\n", addmss, PW->idx1, PW->idx2);
p3errmsg(p3MSG_DEBUG, p3buf);
			memcpy(&PW->newbuf[p3SESSION_HDR4], pkt->packet, PW->idx2);
			// Add MSS field at end of current options (EOL changed to NOP previously)
			PW->i1 = PW->idx2 + p3SESSION_HDR4;
			PW->newbuf[PW->i1++] = 0x02;
			PW->newbuf[PW->i1++] = 0x04;
			PW->newbuf[PW->i1++] = (unsigned char) ((addmss >> 8) & 0xff);
			PW->newbuf[PW->i1++] = (unsigned char) (addmss & 0xff);
			// Append payload to new header
			memcpy(&PW->newbuf[PW->i1], &pkt->packet[PW->idx2],
					(pkt->flag & p3PKT_SIZE) - PW->idx2);
			// Set new IP total length and checksum
			pkt->flag += 4;
			iph = (struct iphdr *) &PW->newbuf[p3SESSION_HDR4];
			iph->tot_len = htons((pkt->flag & p3PKT_SIZE));
			iph->check = 0;
			p3SET_CHECKSUM_V4(iph);
			// Set new TCP header length and checksum
			PW->newbuf[(p3SESSION_HDR4 + 12) + PW->idx1] += 0x10;
			pkt->packet = &PW->newbuf[p3SESSION_HDR4];
			if (p3net_utils(p3TCP_CHECK_ADD, p3sys_net, pkt) < 0) {
				sprintf(p3buf, "packet_handler: System network utility failed:\
 %d\n", p3TCP_CHECK_ADD);
				p3errmsg(p3MSG_ERR, p3buf);
				stat = -1;
				goto out;
			}
		} else {
			memcpy(&PW->newbuf[p3SESSION_HDR4], pkt->packet, (pkt->flag & p3PKT_SIZE));
		}
		// Increment session sequence number
		p3lock(pkt->net->host->session->lock);
		// If rekeying, do not send data packets
		// TODO: Set delay sequence ID
		if (pkt->net->host->session->flag & p3PSS_REKEY) {
			p3unlock(pkt->net->host->session->lock);
p3errmsg(p3MSG_DEBUG, "Err 4\n");
			stat = -1;
			goto out;
		}
		PW->ui1 = pkt->net->host->session->sseq + p3SEQ_DIFF;
		sseq = pkt->net->host->session->sseq;
		pkt->net->host->session->sseq++;
		if (!pkt->net->host->session->sseq)
			pkt->net->host->session->sseq++;
		p3unlock(pkt->net->host->session->lock);
		// Initialize P3 header
		memcpy(PW->newbuf, pkt->net->host->session->p3hdr, p3SESSION_HDR4);
		PW->newbuf[p3IP4_ID] = (PW->ui1 >> 8) & 0xff;
		PW->newbuf[p3IP4_ID + 1] = PW->ui1 & 0xff;
		iph = (struct iphdr *) PW->newbuf;
		iph->tot_len = htons(PW->newlen);
		p3SET_CHECKSUM_V4(iph);
		PW->newbuf[p3SESSION_HDR4 - 4] = (sseq >> 24) & 0xff;
		PW->newbuf[p3SESSION_HDR4 - 3] = (sseq >> 16) & 0xff;
		PW->newbuf[p3SESSION_HDR4 - 2] = (sseq >> 8) & 0xff;
		PW->newbuf[p3SESSION_HDR4 - 1] = sseq & 0xff;
		// Packet being forwarded
		if (pkt->flag & p3PKT_DSSUB) {
p3errmsg(p3MSG_DEBUG, "Set forwarding flag in P3 header\n");
			PW->newbuf[p3SESSION_HDR4 - p3HDR_FLAG3] |= p3HDR_FORWARD;
		}
		// Provide kernel handler with new buffer and status
		pkt->packet = PW->newbuf;
sprintf(p3buf, "Obfuscate pkt: New len %d\n", PW->newlen);
p3errmsg(p3MSG_DEBUG, p3buf);
		if (obfuscate(pkt) < 0) {
p3errmsg(p3MSG_DEBUG, "Err 5\n");
			stat = -1;
			goto out;
		}
sprintf(p3buf, "Encrypt pkt Seq %d\n", sseq);
p3errmsg(p3MSG_DEBUG, p3buf);
		// Encrypt the current packet
		if (p3_encrypt(&PW->newbuf[p3SESSION_HDR4], (PW->newlen - p3SESSION_HDR4),
				sseq, p3DATENC1, &pkt->net->host->session->keymgmt) < 0) {
p3errmsg(p3MSG_DEBUG, "Err 6\n");
			stat = -1;
			goto out;
		}

		stat = p3PKTS_ADDHDR;

sprintf(p3buf, "P3 Hdr: ");
for (PW->i1=0; PW->i1 < p3SESSION_HDR4; PW->i1++) {
	if (!(PW->i1 & 3))
		sprintf(p3buf, "%s ", p3buf);
	sprintf(p3buf, "%s%2.2x", p3buf, pkt->packet[PW->i1]);
}
sprintf(p3buf, "%s\n", p3buf);
p3errmsg(p3MSG_DEBUG, p3buf);

	} // Else let stack handle intercepted packet as is

out:
	if (stat < 0 && pkt->work != NULL ) {
		p3free(pkt->work);
		pkt->work = NULL;
		pkt->packet = NULL;
	}
	return (stat);
} /* end packet_handler */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <hr><b>packet_handler: Failed to allocate P3 packet buffer</b>
 * \par Description (CRIT):
 * The P3 packet handler attempts to allocate a new buffer for a packet
 * that is to be encrypted and have a P3 header prepended to it.  If
 * this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem.
 *
 * <hr><b>packet_handler: System network utility failed:
 *  <i>function id</i></b>
 * \par Description (ERR):
 * The P3 packet handler calls a utility function to perform certain
 * system specific operations.  If this fails, the packet handling
 * cannot proceed.
 * \par Response:
 * Notify the P3 application support.
 *
 * <hr><b>packet_handler: Cannot increase TCP options field</b>
 * \par Description (WARN):
 * The P3 packet handler modifies the TCP MSS field to avoid IP
 * fragmentation.  If the TCP options field is already at the maximum
 * size, this cannot be done.  The packet is discarded.
 * \par Response:
 * None.
 *
 */

/**
 * \par Function:
 * p3_lookup
 *
 * \par Description:
 * Lookup the IP addresses of a packet in the P3 routing tables.
 *
 * \par Inputs:
 * - pkt: A p3packet structure containing information about the packet.
 *
 * \par Outputs:
 * - p3host *:  Remote P3 host structure where
 *   - NULL: Not found in P3 route table
 *   - Structure pointer bit 0:
 *     - 0 = Source is a P3 host
 *     - 1 = Destination is a P3 network
 *       - <i>Note that this bit must be cleared by the caller</i>
 */

void p3_lookup(p3packet *pkt)
{
	int i, j;
	unsigned char *adr1, *adr2;
	unsigned char ip4[sizeof(struct in_addr)];
	unsigned char ip6[sizeof(struct in6_addr)];
	void *local_adr;
	struct iphdr *iph = (struct iphdr *) pkt->packet;

	if (pkt == NULL || pkt->packet == NULL) {
		p3errmsg(p3MSG_ERR, "p3_lookup: NULL packet data\n");
		goto out;
	}
	if (ipv4route == NULL) {
		goto out;
	}
	if (p3hosts == NULL) {
		goto out;
	}
#ifndef _p3_SECONDARY
	local_adr = (void *) &primain->addr.v4;
#else
	local_adr = (void *) &secmain->addr.v4;
#endif

	if (iph->version == 4) {
		// Test for encrypted packet from P3 host
		pkt->host = p3hosts;
		adr1 = (unsigned char *)&iph->saddr;
		while (pkt->host != NULL) {
// adr2 = (unsigned char *)&pkt->host->addr.v4;
// sprintf(p3buf, "Src %2.2x%2.2x%2.2x%2.2x Cmpr %2.2x%2.2x%2.2x%2.2x\n",
// adr1[0], adr1[1], adr1[2], adr1[3],
// adr2[0], adr2[1], adr2[2], adr2[3]);
// p3errmsg(p3MSG_DEBUG, p3buf);
			if (memcmp(adr1, &pkt->host->addr.v4, sizeof(struct in_addr)) == 0) {
				// Forward packet to local subnet
				if (pkt->packet[p3SESSION_HDR4 - p3HDR_FLAG3] & p3HDR_FORWARD) {
#ifndef _p3_SECONDARY
					pkt->net = primain->subnet;
#else
					pkt->net = secmain->subnet;
#endif
					pkt->flag |= p3PKT_P3SRC | p3PKT_SRP3 | p3PKT_DSSUB;
					goto out;
				} else {
					pkt->net = pkt->host->net;
					pkt->flag |= p3PKT_P3SRC | p3PKT_SRP3;
				}
				goto out;
			}
			pkt->host = pkt->host->hlist;
		}
		// Test for destination
		// TODO: Use Red Black tree
		for (i=0; i < ipv4route->netsz; i++) {
			// Get packet subnet using route entry mask
			adr1 = (unsigned char *)&iph->daddr;
			adr2 = (unsigned char *)&ipv4route->nets[i]->mask;
			for (j=0; j < sizeof(struct in_addr); j++) {
				ip4[j] = adr1[j] & adr2[j];
			}
// adr2 = (unsigned char *)&ipv4route->nets[i]->net.v4;
// sprintf(p3buf, "Dst %2.2x%2.2x%2.2x%2.2x Cmpr %2.2x%2.2x%2.2x%2.2x\n",
// ip4[0], ip4[1], ip4[2], ip4[3],
// adr2[0], adr2[1], adr2[2], adr2[3]);
// p3errmsg(p3MSG_DEBUG, p3buf);
			if (memcmp(&ipv4route->nets[i]->net.v4, ip4,
					sizeof(struct in_addr)) == 0) {
				pkt->net = ipv4route->nets[i];
				pkt->flag |= p3PKT_P3DST;
				// Check for dest is P3 host or subnet
				if (memcmp(adr1, &pkt->net->host->addr.v4,
						sizeof(struct in_addr)) == 0)
					pkt->flag |= p3PKT_DSP3;
				else
					pkt->flag |= p3PKT_DSSUB;
				// Test for packet source is P3 host or local subnet
				adr1 = (unsigned char *)&iph->saddr;
				if (memcmp(adr1, local_adr, sizeof(struct in_addr)) == 0)
					pkt->flag |= p3PKT_SRP3;
				else {
					pkt->flag ^= p3PKT_SRSUB;
				}
// !!! Temporary !!!
// !!! Temporary !!!
#ifndef _p3_SECONDARY
if (pkt->net->host->session != NULL && !(pkt->net->host->session->sseq & 0x3f) &&
		!(pkt->net->host->session->flag & p3PSS_REKEY)) {
sprintf(p3buf, "Start Rekeying: Seq %d\n", pkt->net->host->session->sseq);
p3errmsg(p3MSG_DEBUG, p3buf);
if (pkt->flag & p3PKT_DSSUB)
	pkt->net->host->session->flag |= p3PSS_CFWD;
else
	pkt->net->host->session->flag &= ~p3PSS_CFWD;
	start_rekeying(pkt->net->host->session);
	goto out;
}
#endif
// !!! Temporary !!!
// !!! Temporary !!!
				goto out;
			}
		}
		// Test for raw packet to local subnet
#ifndef _p3_SECONDARY
		adr1 = (unsigned char *)&primain->subnet->net.v4;
#else
		adr1 = (unsigned char *)&secmain->subnet->net.v4;
#endif
		if (memcmp(local_adr, &iph->saddr, sizeof(struct in_addr)) == 0 &&
				memcmp(adr1, &iph->daddr, sizeof(struct in_addr)) == 0) {
#ifndef _p3_SECONDARY
			pkt->net = primain->subnet;
#else
			pkt->net = secmain->subnet;
#endif
			pkt->flag |= p3PKT_P3DST;
		}
	} else if (iph->version == 6) {
	// TODO: Handle IPv6
	}

out:

	return;
} /* end p3_lookup */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <hr><b>p3_lookup: NULL packet data</b>
 * \par Description (ERR):
 * The packet provided to compare to the P3 route table was NULL.
 * \par Response:
 * Report the problem to Velocite Systems support.
 *
 */


/**
 * \par Function:
 * obfuscate
 *
 * \par Description:
 * Manipulate a packet to obfuscate it when encrypted.
 *
 * \par Inputs:
 * - pkt: A p3packet structure containing information about the packet.
 *
 * \par Outputs:
 * - int: Status:
 *   - 0: OK
 *   - <0: Error
 */

int obfuscate(p3packet *pkt)
{
	int stat = 0, bct;
	int psize, len, dloc, blks;
	unsigned char *pktdata = &pkt->packet[p3SESSION_HDR4];
	struct timeval now;

	if (pkt->work == NULL) {
		p3errmsg(p3MSG_ERR, "obfuscate: Work area is NULL\n");
		stat = -1;
		goto out;
	}

	// Number of blocks is variable
	do_gettimeofday(&now);
	// TODO: ===>> Handle IPv6 packet length
	psize = pktdata[2];
	psize <<= 8;
	psize |= pktdata[3];
	if (psize < p3PKT_MED) {
		if (psize < 40)
			blks = 2;
		else if (now.tv_usec & 2)
			blks = 2;
		else
			blks = 3;
	} else {
		blks = now.tv_usec & 7;
		if (blks == 0)
			blks = 4;
		else if (blks == 1)
			blks = 6;
	}
	if  ((blks * 3) > (((pkt->flag & p3PKT_SIZE) - p3SESSION_HDR4) - psize))
		blks = (((pkt->flag & p3PKT_SIZE) - p3SESSION_HDR4) - psize) / 3;

	if (blks <= 0) {
		sprintf(p3buf, "Obfuscate Blks <= 0 (%d, %d)\n",
			(pkt->flag & p3PKT_SIZE), psize);
		p3errmsg(p3MSG_DEBUG, p3buf);
		stat = -1;
		goto out;
	}

	// Get range of variable adjustment
	PW->i1 = psize / blks;
	PW->i2 = psize - 1;
	if (PW->i1 < 16)
		len = 0x3;
	else if (PW->i1 < 32)
		len = 0x7;
	else if (PW->i1 < 64)
		len = 0xf;
	else
		len = 0x1f;
	PW->bloc[0] = 0;
	bct = PW->i1;
	// Set variable block starting locations
	for (PW->idx1=1; PW->idx1 < blks; PW->idx1++) {
		PW->bloc[PW->idx1] = bct;
		bct += PW->i1;
		if (pktdata[PW->i2] & 2)
			PW->bloc[PW->idx1] -= pktdata[PW->i2] & len;
		else
			PW->bloc[PW->idx1] += pktdata[PW->i2] & len;
		PW->i2--;
	}
	// Set block sizes
	for (PW->idx1=0; PW->idx1 < (blks - 1); PW->idx1++) {
		PW->blen[PW->idx1] = PW->bloc[PW->idx1 + 1] - PW->bloc[PW->idx1];
	}
	PW->blen[PW->idx1] = psize - PW->bloc[PW->idx1];

	// Set starting block index
	if (blks < 3)
		PW->idx2 = 1;
	else
		PW->idx2 = blks - 1;
	bct = 1;
	PW->idx1 = 0;
	// ===>> Handle IPv6 P3 header size
	len = (pkt->flag & p3PKT_SIZE) - p3SESSION_HDR4;
sprintf(p3buf, "Blocks: %d, Sizes: Pkt %d Len %d, USec %x\n",
	blks, psize, len, now.tv_usec);
p3errmsg(p3MSG_DEBUG, p3buf);
	// Move data into work buffer
PW->ui2 = 16;
	while (PW->idx1 < len) {
if (PW->ui2-- == 0) {
	p3errmsg(p3MSG_DEBUG, "ERROR getting block\n");
	stat = -1;
	goto out;
}
		// Vary the block order (never use first block first)
		if (PW->idx1 > 0) {
			// Get next unused block
PW->i2 = 10;
			while (PW->blen[PW->idx2] == 0) {
if (PW->i2-- == 0) {
	p3errmsg(p3MSG_DEBUG, "ERROR getting block ID A\n");
	stat = -1;
	goto out;
}
				PW->idx2++;
				if (PW->idx2 == blks) {
					PW->idx2 = 0;
				}
			}
			if ((bct <<= 1) > now.tv_usec) 
				bct = 1;
			// Use current block
			if (!(bct & now.tv_usec)) {
				// Use next unused block
				if ((PW->idx2 += 1) == blks) {
					PW->idx2 = 0;
				}
PW->i2 = 10;
				while (PW->blen[PW->idx2] == 0) {
if (PW->i2-- == 0) {
	p3errmsg(p3MSG_DEBUG, "ERROR getting block ID B\n");
	stat = -1;
	goto out;
}
					PW->idx2++;
					if (PW->idx2 == blks) {
						PW->idx2 = 0;
					}
				}
			}
		}
		// Copy data into work buffer
		PW->buf[PW->idx1++] = (unsigned char) PW->idx2;
		if (PW->idx2 == (blks - 1))
			PW->i2 = PW->blen[PW->idx2] + (len - (psize + (blks * 3)));
		else
			PW->i2 = PW->blen[PW->idx2];
		PW->buf[PW->idx1++] = (unsigned char) ((PW->i2 & 0xff00) >> 8);
		PW->buf[PW->idx1++] = (unsigned char) (PW->i2 & 0xff);
sprintf(p3buf, "Idx %d Loc %d Len %d From %d\n",
	PW->idx2, PW->idx1, PW->blen[PW->idx2], PW->bloc[PW->idx2]);
p3errmsg(p3MSG_DEBUG, p3buf);
		memcpy(&PW->buf[PW->idx1], &pktdata[PW->bloc[PW->idx2]], PW->blen[PW->idx2]);
if (PW->blen[PW->idx2] <= 0) {
	p3errmsg(p3MSG_DEBUG, "ERROR in buf len\n");
	stat = -1;
	goto out;
}
		PW->idx1 += PW->blen[PW->idx2];
		// Add pad data to last block of real data
		if (PW->idx2 == (blks - 1)) {
			// ===>> Handle IPv6
			PW->i2 = (pktdata[0] & 0xf) << 2;
			// If dloc > 0 use data only, else include headers
			if (pktdata[9] == 6) {
				dloc = (pktdata[(PW->i2 + 9)] & 0xf0) >> 2;
				dloc += PW->i2;
				if ((psize - dloc) < 0x30)
					dloc = 0;
			} else {
				dloc = PW->i2;
				if ((psize - dloc) < 0x30)
					dloc = 0;
			}
			PW->i1 = len - (psize + (blks * 3));
			PW->i2 = (now.tv_usec & 0x7) + 7;
			PW->i3 = (now.tv_usec & 0x3) + 1;
sprintf(p3buf, "  PadSz %d Step %d PadBy %d\n", PW->i1, PW->i2, PW->i3);
p3errmsg(p3MSG_DEBUG, p3buf);
			while (PW->i1 > 0) {
				if (PW->i1 < PW->i3)
					PW->i3 = PW->i1;
				if ((PW->bloc[PW->idx2] + PW->i3) > psize)
					PW->bloc[PW->idx2] = dloc + (PW->bloc[PW->idx2] - psize);
if (PW->i3 <= 0) {
	p3errmsg(p3MSG_DEBUG, "ERROR in pad counter\n");
	stat = -1;
	goto out;
}
				// Add pad data
				memcpy(&PW->buf[PW->idx1], &pktdata[PW->bloc[PW->idx2]], PW->i3);
				PW->idx1 += PW->i3;
				PW->bloc[PW->idx2] += PW->i2 + PW->i3;
				PW->i1 -= PW->i3;
			}
		}
		// Block is complete
		PW->blen[PW->idx2] = 0;
	}

	// Return obfuscated packet and size
	memcpy(pktdata, PW->buf, len);

out:
	return(stat);
} /* end obfuscate */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <hr><b>obfuscate: Work area is NULL</b>
 * \par Description (ERR):
 * The obfuscation function expects a work area to handle the
 * obfuscation of packet data.  If this is not the case, there is
 * an application problem.
 * \par Response:
 * Report the problem to Velocite Systems support.
 *
 */

/**
 * \par Function:
 * deobfuscate
 *
 * \par Description:
 * Reassemble an obfuscated packet.
 *
 * \par Inputs:
 * - pkt: A p3packet structure containing information about the packet.
 *
 * \par Outputs:
 * - int: Status:
 *   - 0: OK
 *   - <0: Error
 */

int deobfuscate(p3packet *pkt)
{
	int stat = 0, len;
	unsigned char *pktdata = pkt->packet;

	if (pkt->work == NULL) {
		p3errmsg(p3MSG_ERR, "deobfuscate: Work area is NULL\n");
		stat = -1;
		goto out;
	}

	// Sort the blocks
	len = pkt->flag & p3PKT_SIZE;
	PW->idx1 = 0;
	for (PW->i1=0; PW->i1 < 8; PW->i1++) {
		PW->ui1 = (unsigned int) pktdata[PW->idx1++];
		PW->ui2 = (unsigned int) pktdata[PW->idx1++];
		PW->ui2 <<= 8;
		PW->ui2 |= (unsigned int) pktdata[PW->idx1++];
sprintf(p3buf, "Idx %d Loc %d Len %d\n", PW->ui1, PW->idx1, PW->ui2);
p3errmsg(p3MSG_DEBUG, p3buf);
		PW->dloc[PW->ui1] = &pktdata[PW->idx1];
		PW->blen[PW->ui1] = PW->ui2;
		PW->idx1 += PW->ui2;
		if (PW->idx1 >= len) {
			PW->i1++;
			break;
		}
	}
	if (PW->i1 < 8)
		PW->dloc[PW->i1] = NULL;

	// Reassemble the packet
	PW->idx1 = 0;
	for (PW->i1=0; PW->i1 < 8; PW->i1++) {
		if (PW->dloc[PW->i1] == NULL)
			break;
		memcpy(&PW->buf[PW->idx1], PW->dloc[PW->i1], PW->blen[PW->i1]);
		PW->idx1 += PW->blen[PW->i1];
	}
	memcpy(pktdata, PW->buf, len);
	PW->ui1 = (unsigned int) PW->buf[2];
	PW->ui1 <<= 8;
	PW->ui1 |= (unsigned int) PW->buf[3];
	pkt->flag &= ~p3PKT_SIZE;
	pkt->flag |= PW->ui1;

out:
	return(stat);
} /* end deobfuscate */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <hr><b>deobfuscate: Work area is NULL</b>
 * \par Description (ERR):
 * The deobfuscation function expects a work area to handle the
 * deobfuscation of packet data.  If this is not the case, there is
 * an application problem.
 * \par Response:
 * Report the problem to Velocite Systems support.
 *
 */

/**
 * \par Function:
 * p3send_control
 *
 * \par Description:
 * Build a control message and send it to a remote P3 host.  All
 * encryption is performed in this function.  The packet format
 * consists of the following:
 *
 * <pre>
 *     P3 Header      P3 Ctl Header     Control Message
 *                                     |---Encrypted---|
 *                  |------------Encrypted-------------|
 * IP Hdr + ESP Hdr  IP Hdr + UDP Hdr   Control Message
 * </pre>
 *
 * <b><i>Note that the control message is freed in this function.</i></b>
 *
 * \par Inputs:
 * - session: The remote host session structure
 * - cmsg: The control message to be sent
 *
 * \par Outputs:
 * - int: Status:
 *   - 0: OK
 *   - <0: Error
 */

#define CW pkt.work

int p3send_control(p3session *session, p3ctlmsg *cmsg)
{
	int i, j, newlen, stat = 0;
	p3packet pkt;
	struct iphdr *iph;
	struct udphdr *udph;

	memset(&pkt, 0, sizeof(p3packet));
	if (session->host->flag & p3HST_IPV4)
		j = p3SESSION_HDR4;
	else if (session->host->flag & p3HST_IPV6)
		j = p3SESSION_HDR6;
	else {
		p3errmsg(p3MSG_CRIT, "p3send_control: Invalid IP version\n");
		stat = -1;
		goto out;
	}
	// Align both the control message and the control packet on 16 byte boundary
	newlen = ((((cmsg->len + 0xf) & ~0xf) + p3SESSION_HDR4 + 0xf) & ~0xf);
	if ((newlen + 16) <= p3PKT_SMALL)
		newlen = p3PKT_SMALL + j;
	else if ((newlen + 16) <= p3PKT_MED)
		newlen = p3PKT_MED + j;
	else if ((newlen + 16) <= p3PKT_LARGE)
		newlen = p3PKT_LARGE + j;
	else {
		// TODO: Split control command into multiple packets
	}
	// Get work space with 2 data buffers
	i = sizeof(p3work) + (newlen << 1);
	if ((pkt.work = (p3work *) p3malloc(i)) == NULL) {
		p3errmsg(p3MSG_CRIT, "p3send_control: Failed to allocate P3 work area\n");
		stat = -1;
		goto out;
	}
	memset(pkt.work, 0, sizeof(p3work));
	CW->l = (unsigned long) CW + sizeof(p3work);
	CW->newbuf = (unsigned char *) CW->l;
	CW->l += newlen;
	CW->buf = (unsigned char *) CW->l;
	memcpy(CW->newbuf, &pkt.packet[p3SESSION_HDR4], CW->newlen);
	pkt.host = session->host;
	pkt.packet = CW->newbuf;
	pkt.flag = newlen;
	// TODO: Add pad characters to control message data
	// (Currently taking existing data.)
	memcpy(&CW->newbuf[p3SESSION_HDR4 + p3CONTROL_HDR4], cmsg->message, cmsg->len);
sprintf(p3buf, "Control Data: Len %d Seq %d:", cmsg->len, session->sseq);
for (CW->i1=0; CW->i1 < cmsg->len; CW->i1++) {
	if (!(CW->i1 & 3))
		sprintf(p3buf, "%s ", p3buf);
	if (!(CW->i1 & 15))
		sprintf(p3buf, "%s\n  ", p3buf);
	sprintf(p3buf, "%s%2.2x", p3buf, CW->newbuf[p3SESSION_HDR4 + p3CONTROL_HDR4 + CW->i1]);
}
sprintf(p3buf, "%s\n", p3buf);
p3errmsg(p3MSG_DEBUG, p3buf);
	// Encrypt the control message data
	i = (cmsg->len + 0xf) & ~0xf;
	if (p3_encrypt(&CW->newbuf[p3SESSION_HDR4 + p3CONTROL_HDR4], i, session->sseq,
			p3CTLENC1, &session->keymgmt) < 0) {
		p3errmsg(p3MSG_CRIT, "p3send_control: Error encrypting control message\n");
		stat = -1;
		goto out;
	}

	// Build the control message packet
sprintf(p3buf, "Set P3 control message header\n");
p3errmsg(p3MSG_DEBUG, p3buf);
	memcpy(&CW->newbuf[p3SESSION_HDR4], session->p3hdr, p3CONTROL_HDR4);
	// Initialize control packet header
	iph = (struct iphdr *) &CW->newbuf[p3SESSION_HDR4];
	i = ((cmsg->len + 0xf) & ~0xf) + p3SESSION_HDR4;
	iph->tot_len = htons(i);
	i = session->sseq + (p3SEQ_DIFF << 2);
	iph->id = htons(i);
	iph->protocol = 17;		//UDP
	p3SET_CHECKSUM_V4(iph);
	udph = (struct udphdr *) &CW->newbuf[p3SESSION_HDR4 + sizeof(struct iphdr)];
	udph->source = htons(session->host->port);
	udph->dest = htons(session->host->port);
	udph->len = htons((cmsg->len + 0xf) & ~0xf);
sprintf(p3buf, "Control Packet Header:");
for (CW->i1=0; CW->i1 < p3CONTROL_HDR4; CW->i1++) {
	if (!(CW->i1 & 3))
		sprintf(p3buf, "%s ", p3buf);
	if (!(CW->i1 & 15))
		sprintf(p3buf, "%s\n  ", p3buf);
	sprintf(p3buf, "%s%2.2x", p3buf, CW->newbuf[p3SESSION_HDR4 + CW->i1]);
}
sprintf(p3buf, "%s\n", p3buf);
p3errmsg(p3MSG_DEBUG, p3buf);

	// Initialize P3 header
	memcpy(CW->newbuf, session->p3hdr, p3SESSION_HDR4);
	iph = (struct iphdr *) CW->newbuf;
	iph->tot_len = htons(newlen);
	i = session->sseq + p3SEQ_DIFF;
	iph->id = htons(i);
	p3SET_CHECKSUM_V4(iph);
	CW->newbuf[p3SESSION_HDR4 - 4] = (session->sseq >> 24) & 0xff;
	CW->newbuf[p3SESSION_HDR4 - 3] = (session->sseq >> 16) & 0xff;
	CW->newbuf[p3SESSION_HDR4 - 2] = (session->sseq >> 8) & 0xff;
	CW->newbuf[p3SESSION_HDR4 - 1] = session->sseq & 0xff;

	// Obfuscate and encrypt control packet
sprintf(p3buf, "Obfuscate and encrypt ctl pkt: Seq %d\n", session->sseq);
p3errmsg(p3MSG_DEBUG, p3buf);
 	if (obfuscate(&pkt) < 0) {
		p3errmsg(p3MSG_ERR, "p3send_control: Error obfuscating control packet\n");
		stat = -1;
		goto out;
	}
	if (p3_encrypt(&CW->newbuf[p3SESSION_HDR4], (newlen - p3SESSION_HDR4),
			session->sseq, p3DATENC1, &session->keymgmt) < 0) {
		p3errmsg(p3MSG_ERR, "p3send_control: Error encrypting control packet\n");
		stat = -1;
		goto out;
	}
	p3lock(session->lock);
	session->sseq++;
	if (!session->sseq)
		session->sseq++;
	p3unlock(session->lock);
sprintf(p3buf, "Encrypted Control Packet (%d):", newlen);
for (CW->i1=0; CW->i1 < newlen; CW->i1++) {
	if (!(CW->i1 & 3))
		sprintf(p3buf, "%s ", p3buf);
	if (!(CW->i1 & 15))
		sprintf(p3buf, "%s\n  ", p3buf);
	sprintf(p3buf, "%s%2.2x", p3buf, CW->newbuf[CW->i1]);
}
sprintf(p3buf, "%s\n", p3buf);
p3errmsg(p3MSG_DEBUG, p3buf);

	// Send the packet
	if (session->flag & p3PSS_CFWD)
		pkt.flag |= p3PKT_CFWD;
	stat = p3send_packet((void *) &pkt);

out:
	p3free(cmsg);
	if (pkt.work != NULL)
		p3free(pkt.work);
	return (stat);
} /* end p3send_control */

