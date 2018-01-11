/**
 * \file p3knet.h
 * <h3>Protected Point to Point network header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3kNET_H
#define _p3kNET_H

/*****  INCLUDE FILES *****/

#include "p3kbase.h"
#include "p3kconnect.h"

/*****  CONSTANTS  *****/

#define p3PROTO			61		/*<< The IP protocol used by P3 */

#define p3PKTS_NOMOD	0x00	/**< Packet is unmodified */
#define p3PKTS_ADDHDR	0x01	/**< Packet header added (implies encryption) */
#define p3PKTS_RMVHDR	0x02	/**< Packet header removed (implies decryption) */
#define p3PKTS_CONTROL	0x04	/**< Control packet handled by P3 processing */
#define p3PKTS_RAWSOCK	0x08	/**< Packet is establishing a raw socket */
#define p3PKTS_CHKSUM	0x10	/**< Handle checksum for stack */
#define p3PKTS_NEW		0x100	/**< New packet created */

#define p3PKT_SMALL		176		/**< Fixed size of small packet */
#define p3PKT_MED		640		/**< Fixed size of medium packet */
#define p3PKT_LARGE		1440	/**< Fixed size of large packet */
#define p3PKT_MAX		1500	/**< Maximum size of packet */
#define p3PKT_MAXSZ		2048	/**< Maximum size of packet buffer */

/* Network utility function types */
#define p3TCP_CHECK		1	/**< Set a TCP checksum */
#define p3TCP_CHECK_ADD	2	/**< Set a TCP checksum for an added MSS field */
#define p3GET_MTU		3	/**< Get the MTU for an interface */
#define p3SET_DEVIN		4	/**< Set OS dependent inbound info */
#define p3SET_DEVOUT	5	/**< Set OS dependent outbound info */
#define p3SET_RAW		6	/**< Set OS dependent info from raw socket */
#define p3SET_FORWARD	7	/**< Set device info for forwarded packet */

#define p3IP4_ID		4	/**< IPv4 identifier field offset */
#define p3IP4_SADDR		12	/**< IPv4 source address field offset */
#define p3IP4_DADDR		16	/**< IPv4 destination address field offset */

#define p3MSS			536		/**< Minimum segment size */
#define p3MSS_MAX		1440	/**< Maximum segment size */
#define p3EXTRA_V4		52		/**< Extra buffer space needed for IPv4 P3 packet */
#define p3MSS_V4		p3MSS_MAX - p3EXTRA_V4	/**< Maximum IPv4 segment size */
#define p3EXTRA_V6		72		/**< Extra buffer space needed for IPv6 P3 packet */
#define p3MSS_V6		p3MSS_MAX - p3EXTRA_V6	/**< Maximum IPv6 segment size */

/*****  DATA DEFINITIONS  *****/

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

extern int init_p3net(void);
int build_p3table(p3net *net, int ipver);
extern int packet_handler(p3packet *pkt, void *p3sys_net);
void p3_lookup(p3packet *pkt);
extern unsigned char *encrypt_packet(p3session *p3sess, unsigned char *packet);
extern unsigned char *decrypt_packet(p3session *p3sess, unsigned char *packet);
int obfuscate(p3packet *pkt);
int deobfuscate(p3packet *pkt);
int p3send_control(p3session *session, p3ctlmsg *cmsg);

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _p3NET_C
extern p3route *ipv4route, *ipv6route;
extern p3host *p3hosts;
#endif


#endif /* _p3kNET_H */

