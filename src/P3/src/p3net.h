/**
 * \file p3net.h
 * <h3>Protected Point to Point network header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3NET_H
#define _p3NET_H

/*****  INCLUDE FILES *****/

#include "p3base.h"
// #include <netinet/ip.h>
// #include <netinet/ip6.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/shm.h>

/*****  CONSTANTS  *****/

#define p3P3NAME	"localhost"
#define p3P3ADDR	"127.0.0.1"
#define p3P3PORT	5653
#define p3PROTO		61

#define p3DATA_SIZE	4096
#define p3NET_MSG_MAX	16
#define p3ADDR_SZ	64		/* Length of network address in text */

#define p3NET_SERVER	1
#define p3NET_CLIENT	2

#ifdef _p3TEMP_PRIMARY_C
#define INFILE	"primary_input"
#define OUTFILE	"primary_output"
#else
#define INFILE	"secondary_input"
#define OUTFILE	"secondary_output"
#endif

#define p3START		1
#define p3STOP			2

/*****  DATA DEFINITIONS  *****/

typedef struct _p3host p3host;
typedef struct _p3net p3net;
typedef struct _p3NetData	p3NetData;
typedef struct _p3Network	p3Network;

/**
 * Structure:
 * p3host
 * 
 * \par Description:
 * The definition for a remote host.
 */

struct _p3host {
	p3host			*next;	/*<< List pointer */
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} addr;						/*<< Host address (IPv4 or IPv6) */
	p3net			*net;		/*<< Remote network list */
#ifndef _p3_PRIMARY
	int				rekey;		/*<< Period to initiate rekeying in seconds */
	int				hb_wait;	/*<< Period between heartbeats in seconds */
	int				hb_fail;	/*<< Length of heartbeat failure in seconds */
#endif
	unsigned int	flag;
#define p3HST_ID	0x000fffff	/* Host ID */
#define p3HST_IPV4	0x00100000	/* Host address is IPv4 */
#define p3HST_IPV6	0x00200000	/* Host address is IPv6 */
#define p3HST_ARRAY	0x00400000	/* Key arrays allowed */
#define p3HST_KTYPE	0x0f000000	/* Key type field */
#define p3HST_KTSHF	24			/* Field shift amount */
};

/**
 * Structure:
 * p3net
 * 
 * \par Description:
 * The network information for the P3 routing table.  There are two
 * routing tables, one each for IPv4 and IPv6, so the network type
 * does not need to be defined in the structure.
 */

struct _p3net {
	p3net				*next;	/*<< List pointer */
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} net;						/*<< Network address (IPv4 or IPv6) */
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} mask;						/*<< Network mask (IPv4 or IPv6) */
};

/**
 * Structure:
 *   p3NetData
 *
 * Description:
 *   This structure maintains information about network connections.
 *   The information may be about a listener or a connection destination.
 */

struct _p3NetData {
	p3NetData			*next;		/**< Queueing */
    int					idx;		/**< Data start */
    unsigned int		size;		/**< Data length */
#define p3NET_BUFSZ		0x3fffffff	/* Buffer size field */
#define p3NET_BUFSHM	0x40000000	/* Buffer in shared memory */
#define p3NET_1BUF		0x80000000	/* Buffer allocated with structure */
#define p3RECV_SZ		(4096 - sizeof(p3NetData))
    unsigned char		*buffer;	/**< Data buffer */
};

/**
 * Macro:
 *   freeNET_DATA
 *
 * Description:
 *   Free p3NetData structure and all allocated fields.
 *
 * Parameters:
 *   - freestru: Plugin structure to be freed.
 */

#define freeNET_DATA(freestru) \
    if (freestru->buffer != NULL) { \
		if (freestru->size & p3NET_BUFSHM) \
		  shmdt(freestru->buffer); \
	    else if (!(freestru->size & p3NET_1BUF)) \
		  free(freestru->buffer); \
	} \
  free(freestru);

/**
 * Structure:
 *   p3Network
 *
 * Description:
 *   This structure maintains information about network connections.
 *   The information may be about a listener or a connection destination.
 */

struct _p3Network {
	p3Network			*next;		/**< Queueing */
    char				*addr;		/**< Host address string */
    int					port;		/**< Port (local or remote context) */
    int					sd;			/**< Socket descriptor */
	struct sockaddr_storage ss;		/**< Network address info space */
	p3NetData			*datain;	/**< Queue of input data buffers */
	p3NetData			*datainl;	/**< Last input data buffer */
	p3NetData			*dataout;	/**< Queue of output data buffers */
	p3NetData			*dataoutl;	/**< Last output data buffer */
	unsigned int		flag;		/**< Flag */
#define	p3NET_ERRNO		0x0000ff	/* Errno value field */
#define	p3NET_ST_UP		0x000100	/* Status = UP */
#define	p3NET_ST_NEW	0x000200	/* Status = Connection just established */
#define	p3NET_ST_DISC	0x000400	/* Status = Disconnect requested */
#define p3NET_UNIQ		0x001000	/* Only single connection allowed */
#define p3NET_INCMPLT	0x002000	/* Data operation is incomplete */
#define p3NET_V4ONLY	0x004000	/* IPv6 support not available */
#define p3NET_V6ONLY	0x008000	/* Provide IPv6 support only */
#define p3NET_NOBLK		0x020000	/* Set sessions to non-blocking */
#define p3NET_KPALIVE	0x040000	/* Set session KEEPALIVE */
};

/*****  MACROS  *****/

/**
 * Macro:
 * p3GET_HOST_KEYTYPE
 *
 * \par Description:
 * Get the remote host key type.
 *
 * \par Parameters:
 * - flag: The flag field in the remote host structure.
 */
#define p3GET_HOST_KEYTYPE(flag) \
	((flag >> p3HST_KTSHF) & 0x0f)

/**
 * Macro:
 * p3SET_HOST_KEYTYPE
 *
 * \par Description:
 * Set the remote host key type.
 *
 * \par Parameters:
 * - flag: The flag field in the remote host structure.
 * - value: The key type value to be set.
 */
#define p3SET_HOST_KEYTYPE(flag, value) \
	flag &= ~(0x0f << p3HST_KTSHF); \
	flag |= value << p3HST_KTSHF;

/**
 * Macro:
 *   p3NET_MESSAGE
 *
 * Description:
 *   Set the network message in the message array.
 *
 * Parameters:
 *   - net_msg: Message text
 */

#define p3NET_MESSAGE(net_msg) \
fprintf(stderr, "NET %s\n", net_msg);

/**
 * Macro:
 *   p3DATA_RESTART
 *
 * Description:
 *   Set data structure to retransmit data from the beginning.
 *
 * Parameters:
 *   - dr_data: Data structure
 */

#define p3DATA_RESTART(dr_data) \
  dr_data->idx = 0;

/**** WAIT MANAGEMENT ****/

#define p3MILLISEC(msec)   (msec * 1000)   /* Milliseconds in timeval struct */
#define p3MICROSEC(msec)   msec            /* Microseconds in timeval struct */

/**
 * Macro:
 *   p3INIT_WAIT
 *
 * Description:
 *   Initialize timeval struct for wait time
 *
 * Parameters:
 *   - time_val: Structure name
 *   - init_val: Initial microsecond time value
 */

#define p3INIT_WAIT(time_val, init_val) \
  struct timeval time_val; \
  time_val.tv_sec = 0; \
  time_val.tv_usec = init_val;

/**
 * Macro:
 *   p3SET_WAIT
 *
 * Description:
 *   Set wait time for new select call
 *
 * Parameters:
 *   - time_val: Structure name
 *   - tval: Temp int
 */

#define p3SET_WAIT(time_val, tval) \
  gettimeofday(&time_val, NULL); \
  tval = (time_val.tv_usec & 0xfffff) >> 12; \
  time_val.tv_sec = 0; \
  time_val.tv_usec = p3MILLISEC(tval);

/*****  PROTOTYPES  *****/

int p3listen(int sl_type, p3Network *sl_net);
int p3listener(int ls_sd, p3Network *ls_net, p3Network *ls_hostlist);
int p3session(int ss_type, p3Network *ss_net);
int p3net_transmit(p3Network *ns_inet);
int p3net_receive(p3Network *nr_inet);
int p3net_status(p3Network *ns_net);

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _p3NET_MGMT_C
extern char *nm_msgq[p3NET_MSG_MAX];
extern int nm_midx;
extern int nm_pidx;
#endif

#endif /* _p3NET_H */

