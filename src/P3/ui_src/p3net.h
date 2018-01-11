/**
 * \file net_mgmt.h
 * <h3>Header file for network management</h3>
 */

#ifndef _NET_MGMT_H
#define _NET_MGMT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "p3base.h"

/*****  CONSTANTS  *****/

#define NET_MSG_MAX	16

#define NET_SERVER   1
#define NET_CLIENT   2

#define NET_IPV4_SZ	16
#define NET_IPV6_SZ	40

/*****  MACROS  *****/

/**
 * Macro:
 *   NET_MESSAGE
 *
 * Description:
 *   Set the network message in the message array.
 *
 * Parameters:
 *   - net_msg: Message text
 */

#define NET_MESSAGE(net_msg) \
do { \
	if (p3midx < NET_MSG_MAX && (p3msgq[p3midx] = \
			(char *) p3malloc(strlen(net_msg) + 1)) != NULL) { \
		strcpy(p3msgq[p3midx], net_msg); \
		p3midx++; \
	} \
} while (0)

/**
 * Macro:
 *   NET_PRTMSG
 *
 * Description:
 *   Print all network messages in the message array.
 *
 * Parameters:
 *   - None
 */

#define NET_PRTMSG() \
do { \
	for (p3pidx=0; p3pidx < p3midx; p3pidx++) { \
		sprintf(p3buf, "%s\n", p3msgq[p3pidx]); \
		p3errmsg(p3MSG_NONE, p3buf); \
		free(p3msgq[p3pidx]); \
	} \
	if (p3midx) \
		fflush(stdout); \
} while (0)

/**
 * Macro:
 *   NET_LOGMSG
 *
 * Description:
 *   Log all network messages in the message array.
 *
 * Parameters:
 *   - None
 */

#define NET_LOGMSG() \
do { \
	for (p3pidx=0; p3pidx < p3midx; p3pidx++) { \
		sprintf(p3buf, "%s\n", p3msgq[p3pidx]); \
		p3errlog(p3MSG_NONE, p3buf); \
		free(p3msgq[p3pidx]); \
	} \
	if (p3midx) \
		fflush(stdout); \
} while (0)

/**
 * Macro:
 *   NET_SETDATA
 *
 * Description:
 *   Set the network structure data fields for a buffer
 *   to be transmitted.
 *
 * Parameters:
 *   - net: The network structure
 *   - buf: The buffer with a null terminated string to be sent
 *   - blen: The length of the data in the buffer
 */

#define NET_SETDATA(net, buf, blen) \
do { \
	net->data = (unsigned char *) buf; \
	net->len = blen; \
	net->idx = 0; \
} while (0)

/*****  DATA DEFINITIONS  *****/

typedef struct _p3network p3network;

/**
 * Structure:
 *   p3network
 *
 * Description:
 *   This structure maintains information about network connections.
 *   The information may be about a listener or a connection destination.
 */

struct _p3network {
    char				*addr;		/**< Host address string */
    int					port;		/**< Port (local or remote context) */
    int					sd;			/**< Socket descriptor */
	struct sockaddr_storage ss;		/**< Network address info space */
	unsigned char		*data;		/**< Data buffer */
	int					len;		/**< Length of data buffer */
	int					idx;		/**< Index of sent data */
	unsigned int		flag;		/**< Flag */
#define	NET_ERRNO		0x000000ff	/* Errno value field */
#define	NET_ST_UP		0x00000100	/* Status = UP */
#define	NET_ST_NEW		0x00000200	/* Status = Connection just established */
#define	NET_ST_DISC		0x00000400	/* Status = Disconnect requested */
#define NET_INCMPLT		0x00000800	/* Data operation is incomplete */
#define NET_V4ONLY		0x00001000	/* IPv6 support not available */
#define NET_V6ONLY		0x00002000	/* Provide IPv6 support only */
#define NET_NOBLK		0x00004000	/* Set sessions to non-blocking */
#define NET_RESERVE		0x00ff0000	/* Reserved for callback use */
};

/**
 * Macro:
 *   FREE_p3NETWORK
 *
 * Description:
 *   Free the p3network structure and data if it exists.
 *
 * Parameters:
 *   - net: Network structure.
 */

#define FREE_p3NETWORK(net) \
do { \
	if (net->data != NULL) \
		free(net->data); \
	free(net); \
} while (0)

/*****  PROTOTYPES  *****/

int p3listen(int type, p3network *net);
int p3listener(int sd, p3network *net);
int p3session(int type, p3network *net);
int net_transmit(p3network *net);
int net_receive(p3network *net);
int net_status(p3network *net);
int net_ready(p3network *net);

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _NET_MGMT_C
extern char *p3msgq[NET_MSG_MAX];
extern int p3midx;
extern int p3pidx;
/**< Callback for host validation */
extern int p3validate(p3network *net);
#endif

#endif /* _NET_MGMT_H */

