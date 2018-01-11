/**
 * \file p3kconnect.h
 * <h3>Protected Point to Point network header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3kCONNECT_H
#define _p3kCONNECT_H

/*****  INCLUDE FILES *****/

#include "p3kbase.h"
#include "p3kcrypto.h"

/*****  CONSTANTS  *****/

/*****  DATA DEFINITIONS  *****/

typedef struct _p3host p3host;
typedef struct _p3session p3session;
typedef struct _p3route p3route;
typedef struct _p3net p3net;
typedef struct _p3ctlmsg p3ctlmsg;
typedef struct _p3packet p3packet;
typedef struct _p3work p3work;

/**
 * Structure:
 * p3host
 *
 * \par Description:
 * Configuration information for remote P3 hosts.
 */

struct _p3host {
	p3host			*next;		/*<< Timer queue pointer */
	p3host			*hlist;		/*<< TODO: Change list to Red Black tree */
	p3net			*net;		/*<< P3 host network information */
	p3net			*subnet;	/*<< Array of subnets */
	p3session		*session;	/*<< P3 host session information */
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} addr;						/*<< Host address (IPv4 or IPv6) */
	int				hb_wait;	/*<< Period between heartbeats in seconds */
	int				hb_fail;	/*<< Length of heartbeat failure in seconds */
	int				port;		/*<< Primary listener port */
#define p3PRI_PORT		5653	/* Default */
	unsigned int	flag;
#define p3HST_ID	0x000fffff	/* Host ID */
#define p3HST_IPVER	0x00300000	/* IP version field */
#define p3HST_IPV4	0x00100000	/* Host address is IPv4 */
#define p3HST_IPV6	0x00200000	/* Host address is IPv6 */
#define p3HST_ARRAY	0x00400000	/* Key arrays allowed */
#define p3HST_KTYPE	0x0f000000	/* Key type field */
#define p3HST_KTSHF	24			/* Field shift amount */
#define p3HST_SNETS	0xf0000000	/* Number of subnets */
#define p3HST_SNSHF	28			/* Field shift amount */
};

/**
 * Structure:
 * p3session
 * 
 * \par Description:
 * The structure used by both the primary and secondary session manager.
 */

struct _p3session {
	p3host			*host;		/*<< The owning host */
	p3keymgmt		keymgmt;	/*<< Keys to be managed for a session */
	unsigned char	*keylist;	/*<< Key array */
	int				listsize;	/*<< Size of key array */
	int				dindex;		/*<< Data key index */
	int				cindex;		/*<< Control key index */
	// NOTE: The P3 session sequence starts at 1.  0 is used internally.
	unsigned int	sseq;		/*<< Sequence for data sent using key 1 */
#define p3SEQ_DIFF	0xa65d
	unsigned int	rID0;		/*<< Sequence start for data received using key 0 */
	unsigned int	rID1;		/*<< Sequence start for data received using key 1 */
#ifndef _p3_SECONDARY
	time_t			dikey;		/*<< Next time to use data index */
	time_t			cikey;		/*<< Next time to use control index */
	int				ditime;		/*<< Period to rekey data from list */
	int				citime;		/*<< Period to rekey control from list */
	time_t			rekey;		/*<< Next time to rekey */
	int				rk_wait;	/*<< Period to initiate rekeying in seconds */
#endif
	p3lock			lock;		/*<< Session lock */
/* There are 2 P3 headers.  They are both the same size because the ESP header
 * and the UDP header are the same size.  The constant fields in the IP header
 * of the P3 header are built when the session is created and the rest are set
 * to zero.  The ESP/UDP headers are cleared and appropriately filled in for
 * each packet.
 * - P3 session header:  This is prepended to every P3 packet
 * - P3 control header:  This is prepended to P3 control messages.  Note that
 *   the total packet len must be a multiple of 16.
 */
	unsigned char	*p3hdr;		/*<< P3 network header */
#define p3HDR_SIZE		8		/* The size of the P3 encryption (ESP) header */
#define p3SESSION_HDR4	(20 + p3HDR_SIZE)
#define p3SESSION_HDR6	(40 + p3HDR_SIZE)
#define p3CONTROL_HDR4	28		/* IPv4 + UDP header */
#define p3CONTROL_HDR6	48		/* IPv6 + UDP header */
// P3 header flags
#define p3HDR_FLAG0     8		/* Subtract from p3SESSION_HDRn */
#define p3HDR_FLAG1     7		/* Subtract from p3SESSION_HDRn */
#define p3HDR_FLAG2     6		/* Subtract from p3SESSION_HDRn */
#define p3HDR_FLAG3     5		/* Subtract from p3SESSION_HDRn */
#define p3HDR_FORWARD   4       /* Encrypted packet should be forwarded */
	int				hdrlen;		/*<< Length of P3 network header */
	unsigned int	flag;
#define p3PSS_KTYPE	0x0000000f	/* Key type (see p3crypto.h) */
#define p3PSS_REKEY	0x00000020	/* Session being rekeyed */
#define p3PSS_DRDY	0x00000040	/* New data key has been retrieved */
#define p3PSS_CRDY	0x00000080	/* New control key has been retrieved */
#define p3PSS_CFWD	0x00000100	/* Control packet on forwarded link */
// reserved p3HST_IPV4	0x00100000	/* Host address is IPv4 */
// reserved p3HST_IPV6	0x00200000	/* Host address is IPv6 */
};

#define p3SESSION_SIZE	(sizeof(p3session) + (p3KMG_KEYS * sizeof(p3key)))

/**
 * Structure:
 * p3route
 *
 * \par Description:
 * The route table for determining which P3 host to send packets to.
 */

struct _p3route {
	p3net			**nets;		/*<< P3 networks */
	int				netsz;		/*<< Number of networks in table */
#define P3MAXROUTE	8
	unsigned int	flag;
//reserve p3IP_TYPE	0x30000000	/* IP version field */
};

/**
 * Structure:
 * p3net
 *
 * \par Description:
 * The P3 route table entry.
 */

struct _p3net {
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} net;							/*<< Subnet address (IPv4 or IPv6) */
	struct in_addr		mask;		/*<< Subnet mask (IPv4) */
	p3host				*host;		/*<< Remote host entry for route */
	void				*netdata;	/*<< OS dependent network information */
	unsigned int		flag;
#define p3NET_DEVI		0x00000001	/* OS dependent inbound data set */
#define p3NET_DEVO		0x00000002	/* OS dependent outbound data set */
#define p3NET_RAW		0x00000004	/* OS dependent raw socket data set */
#define p3NET_ACT		0x00000010	/* Network is active */
// #define p3HST_IPVER	0x00300000	/* IP version field */
// #define p3HST_IPV4	0x00100000	/* Host address is IPv4 */
// #define p3HST_IPV6	0x00200000	/* Host address is IPv6 */
};

/**
 * Structure:
 * p3ctlmsg
 * 
 * \par Description:
 * The P3 control message.
 */

struct _p3ctlmsg {
	unsigned char	*message;	/*<< Message data including length */
	int				len;		/*<< Length of message buffer */
};

/**
 * Structure:
 * p3packet
 *
 * \par Description:
 * Information about a single packet.
 */

struct _p3packet {
	unsigned char	*packet;	/*<< Packet data, including headers */
	p3net			*net;		/*<< Packet destination network */
	p3host			*host;		/*<< Packet source P3 host */
	p3work			*work;		/*<< Packet handler work fields */
	int				netdata;	/*<< Interface to net_utils function */
	unsigned int	flag;
#define p3PKT_SIZE	0x0000ffff	/* Size of packet data */
#define p3PKT_OP	0x00070000	/* Operation flags */
#define p3PKT_P3SRC	0x00010000	/* Source address is P3 host */
#define p3PKT_P3DST	0x00020000	/* Destination address is P3 subnet */
#define p3PKT_CFWD	0x00040000	/* Control packet on forwarded link */
#define p3PKT_SRSUB	0x00100000	/* Packet source is subnet */
#define p3PKT_SRP3	0x00200000	/* Packet source is P3 host */
#define p3PKT_DSSUB	0x00400000	/* Packet destination is subnet */
#define p3PKT_DSP3	0x00800000	/* Packet destination is P3 host */
};

/**
 * Structure:
 * p3work
 *
 * \par Description:
 * Work fields for handling data to reduce stack requirements.
 */

struct _p3work {
	unsigned char	*newbuf;
	unsigned char	*buf;
	struct tcphdr	*tcph;
	struct udphdr	*udph;
	unsigned char	*dloc[8];
	p3ctlmsg		ctlmsg;
	int				newlen;
	int				blen[8];
	int				bloc[8];
	int				idx1;
	int				idx2;
	int				i1;
	int				i2;
	int				i3;
	unsigned int	ui1;
	unsigned int	ui2;
	unsigned long	l;
};

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

/*****  EXTERNAL DEFINITIONS  *****/

#endif /* _p3kCONNECT_H */

