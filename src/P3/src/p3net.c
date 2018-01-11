/**
 * Temporary network management.
 */

#define _p3NET_MGMT_C 1
#include "p3net.h"

/** Temporary buffer for network management */
char msg_tbuf[1024];
/** String pointer for memory messages */
char *p3emsg;
/** Network message queue */
char *nm_msgq[p3NET_MSG_MAX];
/** Index of network message queue */
int nm_midx = 0;
/** Index of network message queue */
int nm_pidx = 0;

/**
 * Open a socket for listening
 *
 * Inputs:
 * - sl_type: Operation type
 *   - Types:
 *	 - p3START:  Attempt to open a socket for listening
 *	 - p3STOP:  Close a listeners socket
 * - sl_net: Network structure defining local host
 *
 * Outputs:
 * - stat: Status
 *   - 0 = OK
 *   - <0 = Error
 *   - >0 = Informational
 */

int p3listen(int sl_type, p3Network *sl_net)
{
  int stat = 0, sl_opton = 1;
  int sl_sd = 0, sl_fcntl;
  struct addrinfo sl_adin, *sl_ai = &(sl_adin), *tai, *tai1;
  struct sockaddr_in *sl_sa_v4;
  struct sockaddr_in6 *sl_sa_v6;

  nm_midx = 0;

	if (sl_net == NULL)
	{
	  stat = -1;
	  goto out;
	}

/* Start listening */
	if (sl_type == p3START)
	{
/* Get network address information */
	  memset(sl_ai, 0, sizeof(sl_adin));
		if (sl_net->flag & p3NET_V4ONLY)
		  sl_ai->ai_family = PF_INET;
		else if (sl_net->flag & p3NET_V6ONLY)
		  sl_ai->ai_family = PF_INET6;
		else
		  sl_ai->ai_family = PF_UNSPEC;
	  sl_ai->ai_socktype = SOCK_STREAM;
	  sl_ai->ai_flags = AI_PASSIVE;
		if ((stat = getaddrinfo(sl_net->addr, NULL, sl_ai, &tai)) != 0)
		{

		  sprintf(msg_tbuf, "(ERR) p3listen: Unable to get network address\
 information: %s", gai_strerror(stat));
		  p3NET_MESSAGE(msg_tbuf)
		  stat = -1;
		  goto out;
		}

		for (tai1 = tai; tai1; tai1 = tai1->ai_next)
		{
			if (tai1->ai_family == AF_INET)
			{
			  sl_sa_v4 = (struct sockaddr_in *) tai1->ai_addr;
			  sl_sa_v4->sin_port = ntohs((uint16_t) sl_net->port);
			}
			else if (tai1->ai_family == AF_INET6)
			{
			  sl_sa_v6 = (struct sockaddr_in6 *) tai1->ai_addr;
			  sl_sa_v6->sin6_port = ntohs((uint16_t) sl_net->port);
			}
/* Open a socket */
			if ((sl_sd = socket(tai1->ai_family, SOCK_STREAM, 0)) < 0)
			{
			  sl_net->flag |= errno;
			  sprintf(msg_tbuf, "(ERR) p3listen: Unable to open socket: %s",
					strerror(errno));
			  p3NET_MESSAGE(msg_tbuf)
			  stat = 0;
			  continue;
			}
/* Reuse address to allow faster restart */
			else if ((stat = setsockopt(sl_sd, SOL_SOCKET, SO_REUSEADDR,
						   &sl_opton, sizeof(sl_opton))) < 0)
			{
			  sl_net->flag |= errno;
			  sprintf(msg_tbuf, "(ERR) p3listen: Unable to set socket '%d'\
 to reuse address: %s", sl_sd, strerror(errno));
			  p3NET_MESSAGE(msg_tbuf)
			  close(sl_sd);
			  stat = 0;
			}
#ifdef IPV6_V6ONLY
/* Define support for IPV6 only */
			else if (tai1->ai_family == AF_INET6 &&
				(stat = setsockopt(sl_sd, IPPROTO_IPV6, IPV6_V6ONLY,
						   &sl_opton, sizeof(sl_opton))) < 0)
			{
			  sl_net->flag |= errno;
			  sprintf(msg_tbuf, "(ERR) p3listen: Unable to set socket '%d'\
 to IPV6 Only: %s", sl_sd, strerror(errno));
			  p3NET_MESSAGE(msg_tbuf)
			  close(sl_sd);
			  stat = 0;
			}
#endif /* IPV6_V6ONLY */
/* Bind socket to address and port */
			else if ((stat = bind(sl_sd, tai1->ai_addr, tai1->ai_addrlen)) < 0)
			{
			  sl_net->flag |= errno;
			  sprintf(msg_tbuf, "(ERR) p3listen: Unable to bind socket '%d':\
 %s", sl_net->sd, strerror(errno));
			  p3NET_MESSAGE(msg_tbuf)
			  close(sl_sd);
			  stat = 0;
			}
/* Listen on socket */
			else if ((stat = listen(sl_sd, SOMAXCONN)) < 0)
			{
			  sl_net->flag |= errno;
			  sprintf(msg_tbuf, "(ERR) p3listen: Unable to listen on socket\
 '%d': %s", sl_sd, strerror(errno));
			  p3NET_MESSAGE(msg_tbuf)
			  close(sl_sd);
			  stat = 0;
			}
/* Set socket to non-blocking */
			else
			{
			  sl_fcntl = fcntl(sl_sd, F_GETFL, 0);
				if ((stat = fcntl(sl_sd, F_SETFL, sl_fcntl | O_NONBLOCK)) < 0)
				{
				  sl_net->flag |= errno;
				  sprintf(msg_tbuf, "(WARN) p3listen: Unable to set socket\
 '%d' to nonblocking: %s", sl_sd, strerror(errno));
				  p3NET_MESSAGE(msg_tbuf)
				  stat = -1;
				}
			  sprintf(msg_tbuf, "(NOTE) Ready for requests on port '%d',\
 socket '%d'", sl_net->port,  sl_sd);
			  p3NET_MESSAGE(msg_tbuf)
			  memcpy(&(sl_net->ss), tai1, sizeof(sl_net->ss));
			}
		}
/* Release addrinfo structures */
	  freeaddrinfo(tai);
		if (sl_sd > 0)
		  sl_net->sd = sl_sd;
		else
		{
		  p3NET_MESSAGE("(ERR) p3listen: No sockets available for listener")
		  stat = -1;
		  goto out;
		}

	}

/* Stop listening */
	else if (sl_type == p3STOP)
	{
		if ((sl_net->sd > 0) && ((stat = close(sl_net->sd)) == 0))
		{
		  sprintf(msg_tbuf, "(NOTE) p3listen: Listener on port '%d' socket\
 '%d' disconnected", sl_net->port,  sl_net->sd);
		  p3NET_MESSAGE(msg_tbuf)
		}
		else if (stat != 0)
		{
		  sprintf(msg_tbuf, "(ALERT) p3listen: Error stopping listener on\
 socket '%d'", sl_net->sd);
		  p3NET_MESSAGE(msg_tbuf)
		  stat = -1;
		}
	  sl_net->sd = 0;
	}

out:
  return(stat);

} /* end p3listen */


/**
 *  Function:
 *	p3listener
 *
 *\par Description:
 *  Accept or reject listen requests
 *
 * Inputs:
 * - ls_sd: Socket descriptor of connection request
 * - ls_net: Network structure of local host
 * - ls_hostlist: List of allowed remote hosts
 *
 * Outputs:
 * - stat: Status
 *   - 0 = OK
 *   - <0 = Error
 *   - >0 = New socket descriptor for host
 * - ls_net->flag & p3NET_ERRNO = errno if applicable
 */

int p3listener(int ls_sd, p3Network *ls_net, p3Network *ls_hostlist)
{
  int i, stat = 0;
  int new_sd;
  int ls_salen = sizeof(struct sockaddr_storage), ls_opton = 1;
  char *ls_host, *ls_port;
  struct sockaddr *ls_sa = (struct sockaddr *) &(ls_net->ss);
  p3Network *tnet;
  struct linger ls_linger;

  nm_midx = 0;

  ls_net->flag &= ~p3NET_ERRNO;
printf("DEBUG: Enter listener\n");

/* Accept the request */
	if ((new_sd = accept(ls_sd, ls_sa, &ls_salen)) <= 0 ) {
		if (new_sd < 0) {
		  ls_net->flag |= errno;
		  sprintf(msg_tbuf, "(WARN) p3listener: Error accepting connection:\
 %s", strerror(errno));
		  p3NET_MESSAGE(msg_tbuf)
		  stat = -1;
		}
	}

/* Find host description */
	if ((stat = getpeername(new_sd, ls_sa, &ls_salen)) < 0) {
	  ls_net->flag |= errno;
	  sprintf(msg_tbuf, "(WARN) p3listener: Error getting connected host\
 information: %s", strerror(errno));
	  p3NET_MESSAGE(msg_tbuf)
	  stat = -1;
	  goto out;
	}
	if ((ls_host = (char *) calloc(1, MAXHOSTNAMELEN+1)) == NULL) {
	  sprintf(msg_tbuf, "Failed to allocate host name buffer: %s",
				strerror(errno));
	  p3NET_MESSAGE(msg_tbuf)
	  stat = -1;
	  goto out;
	}
	if ((ls_port = (char *) calloc(1, 10)) == NULL) {
	  sprintf(msg_tbuf, "Failed to allocate port buffer: %s",
				strerror(errno));
	  p3NET_MESSAGE(msg_tbuf)
	  stat = -1;
	  goto out;
	}
	if ((stat = getnameinfo(ls_sa, ls_salen, ls_host, MAXHOSTNAMELEN+1,
		 ls_port, 10, (NI_NUMERICHOST|NI_NUMERICSERV))) < 0) {
	  ls_net->flag |= errno;
	  sprintf(msg_tbuf, "(ERR) p3listener: Error formatting connected\
 host information: %s", strerror(errno));
	  p3NET_MESSAGE(msg_tbuf)
	  stat = -1;
	  goto out;
	}
printf("DEBUG: Name info: %s\n", ls_host);

/* Validate request and initialize socket */
  tnet = ls_hostlist;
	while (tnet != NULL && stat == 0) {
	  i = strlen(tnet->addr);
		if (strncmp(ls_host, tnet->addr, i) == 0) {
/* Connection exists */
			if (tnet->sd != 0) {
				if (p3net_status(tnet) < 0) {
				  sprintf(msg_tbuf, "(WARN) p3listener: Session reset\
for new session with %s", ls_host);
				  p3NET_MESSAGE(msg_tbuf)
				  break;
				}
			  sprintf(msg_tbuf, "(WARN) p3listener: Session already\
established with %s", ls_host);
			  p3NET_MESSAGE(msg_tbuf)
			  stat = -1;
			  tnet = NULL;
/* Set values for new connection */
			} else {
printf("DEBUG: Host found\n");
			  tnet->sd = stat = new_sd;
			  tnet->flag &= ~p3NET_ST_DISC;
			  tnet->flag |= (p3NET_ST_UP | p3NET_ST_NEW);
			  break;
			}
		}
		if (stat == 0)
		  tnet = tnet->next;
	}
printf("DEBUG: Host not found\n");

/* Issue warning message if connection not accepted */
	if (tnet == NULL || stat < 0) {
	  sprintf(msg_tbuf, "(ALERT) p3listener: Connection not accepted\
from %s, port '%s' on socket '%d'", ls_host, ls_port, new_sd);
	  p3NET_MESSAGE(msg_tbuf)
	  shutdown(new_sd, SHUT_RDWR);
		if (stat >= 0)
		  stat = -1;
	  goto out;
	}

	  sprintf(msg_tbuf, "(NOTE) p3listener: Connection accepted from %s,\
port '%s' on socket '%d'", ls_host, ls_port, new_sd);
	  p3NET_MESSAGE(msg_tbuf)

/* Set to nonblocking */
	if (tnet->flag & p3NET_NOBLK) {
	  i = fcntl(tnet->sd, F_GETFL, 0);
		if (fcntl(tnet->sd, F_SETFL, i | O_NONBLOCK) < 0) {
		  sprintf(msg_tbuf, "(NOTE) p3listener: Session not set to\
nonblocking: %s", strerror(errno));
		  p3NET_MESSAGE(msg_tbuf)
		}
	}

/* For sessions that are ending, do not attempt to send remaining data */
  ls_linger.l_onoff = 1;
  ls_linger.l_linger = 0;
	if (setsockopt(tnet->sd, SOL_SOCKET, SO_LINGER,
		&ls_linger, sizeof(ls_linger)) < 0) {
	  sprintf(msg_tbuf, "(ERR) p3listener: Session not set for fast\
close: %s", strerror(errno));
	  p3NET_MESSAGE(msg_tbuf)
	}

/* Send KEEPALIVE every 5 min. to verify session still active */
	if (tnet->flag & p3NET_KPALIVE) {
	  ls_opton = 1;
	  i = 300;
		if (setsockopt(tnet->sd, SOL_SOCKET, SO_KEEPALIVE, &ls_opton,
			sizeof(int)) < 0) {
	  sprintf(msg_tbuf, "(WARN) p3listener: KEEPALIVE not activated:\
%s", strerror(errno));
		  p3NET_MESSAGE(msg_tbuf)
		} else if (setsockopt(tnet->sd, IPPROTO_TCP, TCP_KEEPIDLE, &i,
			sizeof(int)) < 0) {
		  sprintf(msg_tbuf, "(WARN) p3listener: KEEPALIVE idle time not set:\
%s", strerror(errno));
		  p3NET_MESSAGE(msg_tbuf)
		}
	}

out:
  return(stat);

} /* end p3listener */

/**
 *  Function:
 *	 p3session
 *
 * \par Description:
 *  Establish a session with a remote host
 *
 * Inputs:
 * - ss_type: Operation type
 *   - p3START:  Attempt to establish a session
 *   - p3STOP:  Disconnect a session
 * - ss_net: Network information structure
 *
 * Outputs:
 * - stat: Status
 *   - 0 = OK
 *   - <0 = Error
 *   - >0 = Informational
 */

int p3session(int ss_type, p3Network *ss_net)
{
  int stat = 0, scount = 0;
  int ss_sd = 0, ss_fcntl;
  struct addrinfo ss_adin, *ss_ai = &(ss_adin), *tai, *tai1;
  struct sockaddr_in *ss_sa_v4;
  struct sockaddr_in6 *ss_sa_v6;

  nm_midx = 0;

	if (ss_net == NULL)
	{
	  stat = -1;
	  goto out;
	}

/* Start session */
	if (ss_type == p3START)
	{
/* Get network address information */
	  memset(ss_ai, 0, sizeof(ss_adin));
		if (ss_net->flag & p3NET_V4ONLY)
		  ss_ai->ai_family = PF_INET;
		else if (ss_net->flag & p3NET_V6ONLY)
		  ss_ai->ai_family = PF_INET6;
		else
		  ss_ai->ai_family = PF_UNSPEC;
	  ss_ai->ai_socktype = SOCK_STREAM;
	  ss_ai->ai_flags = AI_PASSIVE;
		if ((stat = getaddrinfo(ss_net->addr, NULL, ss_ai, &tai)) != 0)
		{
		  sprintf(msg_tbuf, "(ERR) p3session: Unable to get network address\
 information: %s", gai_strerror(stat));
		  p3NET_MESSAGE(msg_tbuf)
		  stat = -1;
		  goto out;
		}
		for (tai1 = tai; tai1 && !scount; tai1 = tai1->ai_next)
		{
/* Set port */
			if (tai1->ai_family == AF_INET)
			{
			  ss_sa_v4 = (struct sockaddr_in *) tai1->ai_addr;
			  ss_sa_v4->sin_port = ntohs((uint16_t) ss_net->port);
			}
			else if (tai1->ai_family == AF_INET6)
			{
			  ss_sa_v6 = (struct sockaddr_in6 *) tai1->ai_addr;
			  ss_sa_v6->sin6_port = ntohs((uint16_t) ss_net->port);
			}
/* Open a socket */
			if ((ss_sd = socket(tai1->ai_family, SOCK_STREAM, 0)) < 0)
			{
			  sprintf(msg_tbuf, "(ERR) p3session: Unable to get socket: %s",
					strerror(errno));
			  p3NET_MESSAGE(msg_tbuf)
			  stat = 0;
			  continue;
			}

/* Try to connect to specified host */
			if (connect(ss_sd, tai1->ai_addr, tai1->ai_addrlen) < 0)
			{
				if (ss_net->addr != NULL)
				  sprintf(msg_tbuf, "(ERR) Unable to connect to host '%s'",
						ss_net->addr);
				else
				  sprintf(msg_tbuf, "(ERR) Unable to connect to host UNKNOWN");
			  p3NET_MESSAGE(msg_tbuf)
			  stat = 0;
			  close(ss_sd);
			  continue;
			}
		  scount++;
		  memcpy(&(ss_net->ss), tai1, sizeof(ss_net->ss));
		}
/* Release addrinfo structures */
	  freeaddrinfo(tai);
		if (!scount)
		{
		  sprintf(msg_tbuf, "(WARN) p3session: Connection to host\
'%d' not established", ss_sd);
		  p3NET_MESSAGE(msg_tbuf);
		  stat = -1;
		  goto out;
		}
	  ss_net->sd = ss_sd;

/* Set socket to non-blocking */
	  ss_fcntl = fcntl(ss_net->sd, F_GETFL, 0);
	  fcntl(ss_net->sd, F_SETFL, ss_fcntl | O_NONBLOCK);
	  sprintf(msg_tbuf, "(NOTE) p3session: Session ready on socket '%d',\
 port '%d'", ss_net->sd, ss_net->port);
	  p3NET_MESSAGE(msg_tbuf)
	}

/* Stop session */
	if (ss_type == p3STOP)
	{
		if ((ss_net->sd > 0) &&
			((stat = shutdown(ss_net->sd, SHUT_RDWR)) == 0))
		{
		  sprintf(msg_tbuf, "(NOTE) p3session: Session on socket '%d'\
 disconnected", ss_net->sd);
		  p3NET_MESSAGE(msg_tbuf)
		}
		if (stat < 0)
		{
		  ss_net->flag |= errno;
		  sprintf(msg_tbuf, "(WARN) p3session: Error stopping session on\
 socket '%d'", ss_net->sd);
		  p3NET_MESSAGE(msg_tbuf)
		  stat = -1;
		}
	  ss_net->sd = 0;
	  ss_net->flag &= ~(p3NET_ST_UP | p3NET_ST_NEW);
	}

out:
  return(stat);

} /* end p3session */


/**
 * Function:
 *   p3net_transmit
 *
 * \par Description
 *   - Send buffered data
 *
 * Inputs:
 * - ns_net: Network structure for remote host connection
 *
 * Outputs:
 * - stat: Status
 *   - 0 = OK
 *   - <0 = Error
 *   - >0 = Informational
 */

int p3net_transmit(p3Network *ns_net)
{
  int i, stat = 0, ns_size;
  p3NetData *ns_data;
  p3INIT_WAIT(ns_tv, 0)

	if ((stat = p3net_status(ns_net)) < 0)
	{
	  stat = -1;
	  goto out;
	}

  nm_midx = 0;

/* Send data as long as it is available */
  ns_data = ns_net->dataout;
	while (ns_data != NULL)
	{
	  ns_size = (ns_data->size & p3NET_BUFSZ) - ns_data->idx;
		if ((stat =
			write(ns_net->sd, &(ns_data->buffer[ns_data->idx]), ns_size)) < 0)
		{
/* Session has ended */
			if (errno == EPIPE)
			{
			  sprintf(msg_tbuf, "(WARN) p3net_transmit: Session ended");
			  p3NET_MESSAGE(msg_tbuf)
			  stat = -1;
			  p3session(p3STOP, ns_net);
			}
			else if (errno != EAGAIN)
			{
			  ns_net->flag |= errno;
			  sprintf(msg_tbuf, "(ERR) p3net_transmit: Error writing data:\
 %s", strerror(errno));
			  p3NET_MESSAGE(msg_tbuf)
			  stat = -1;
			  p3session(p3STOP, ns_net);
			}
/* Network is busy try again later */
			else
			{
			  ns_net->flag |= p3NET_INCMPLT;
			  stat = -1;
			}
		  goto out;
		}

/* Make sure connection is still valid */
	if ((i = p3net_status(ns_net)) < 0)
	{
	  stat = -1;
	  goto out;
	}

/* All data in current buffer sent, get next buffer */
		if (stat == ns_size)
		{
		  stat = 0;
		  ns_net->dataout = ns_net->dataout->next;
			if (ns_net->dataout == NULL)
			  ns_net->dataoutl = NULL;
		  freeNET_DATA(ns_data)
		  ns_data = ns_net->dataout;
		}
		else
		{
		  ns_data->idx += stat;
		  stat = 0;
		  ns_net->flag |= p3NET_INCMPLT;
		}
	}

out:
  return(stat);

} /* end p3net_transmit */


/**
 * Function:
 *   p3net_receive
 *
 * \par Description
 *   - Receive data into component buffers
 *
 * Inputs:
 * - nr_net: Network structure for remote host connection
 *
 * Outputs:
 * - stat: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int p3net_receive(p3Network *nr_net)
{
  int stat = 0, nr_numfds = nr_net->sd + 1;
  p3NetData *nr_data;
  fd_set nr_fdset;
  p3INIT_WAIT(nr_tv, 0)

	if ((stat = p3net_status(nr_net)) < 0)
	{
	  stat = -1;
	  goto out;
	}

  nm_midx = 0;

/* Receive data as long as it is available */
	while (1)
	{
/* Get new Network data structure */
		if (nr_net->datainl == NULL ||
			(nr_net->datainl->size & p3NET_BUFSZ) == p3RECV_SZ)
		{
			if ((nr_data = (p3NetData *)
				malloc(sizeof(p3NetData) + p3RECV_SZ)) == NULL)
			{
		  sprintf(msg_tbuf, "p3net_receive: Failed to allocate receive\
 structure: %s", strerror(errno));
			  p3NET_MESSAGE(msg_tbuf)
			  stat = -1;
			  goto out;
			}
/* Set new Network data fields */
		  memset(nr_data, 0, sizeof(p3NetData));
		  nr_data->buffer = (unsigned char *) nr_data + sizeof(p3NetData);
		  nr_data->size = p3NET_1BUF;
/* Add Network data structure to queue */
			if (nr_net->datainl == NULL)
			  nr_net->datain = nr_net->datainl = nr_data;
			else
			{
			  nr_net->datainl->next = nr_data;
			  nr_net->datainl = nr_data;
			}
		}
		else
		  nr_data = nr_net->datainl;

/* Get data */
		if ((stat = read(nr_net->sd,
			&(nr_data->buffer[(nr_data->size & p3NET_BUFSZ)]),
			(p3RECV_SZ - (nr_data->size & p3NET_BUFSZ)))) < 0)
		{
		  nr_net->flag |= errno;
		  sprintf(msg_tbuf, "(ERR) p3net_receive: Error reading data: %s",
					strerror(errno));
		  p3NET_MESSAGE(msg_tbuf)
		  stat = -1;
		  p3session(p3STOP, nr_net);
		  goto out;
		}
/* Session ended */
		else if (!stat)
		{
		  nr_net->flag |= errno;
		  sprintf(msg_tbuf, "(ERR) p3net_receive: Error reading data: %s",
					strerror(errno));
		  p3NET_MESSAGE(msg_tbuf)
		  stat = -1;
		  p3session(p3STOP, nr_net);
		  goto out;
		}

/* Check for more data ready */
	  nr_data->size += stat;
/* Prepare socket descriptor set */
	  FD_ZERO(&nr_fdset);
	  FD_SET(nr_net->sd, &nr_fdset);
	  nr_tv.tv_usec = p3MICROSEC(1);
		if ((stat = select(nr_numfds, &nr_fdset, 0, 0, &nr_tv)) < 0)
		{
		  nr_net->flag |= errno;
		  sprintf(msg_tbuf, "(WARN) p3net_receive: Error checking for data:\
 %s", strerror(errno));
		  p3NET_MESSAGE(msg_tbuf)
		  goto out;
		}
		else if (!(FD_ISSET(nr_net->sd, &nr_fdset)))
		  goto out;
	}

out:
	if ((stat = p3net_status(nr_net)) < 0)
	  stat = -1;

  return(stat);

} /* end p3net_receive */

/**
 * Function:
 *   p3net_status
 *
 * \par Description
 *   - Receive data into component buffers
 *
 * Inputs:
 * - ns_net: Network structure for remote host connection
 *
 * Outputs:
 * - stat: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int p3net_status(p3Network *ns_net)
{
  int stat = 0;
  int ns_soval, ns_solen = sizeof(int);

	if ((stat = getsockopt(ns_net->sd, SOL_SOCKET, SO_ERROR,
		&ns_soval, &ns_solen)) < 0)
	{
	  sprintf(msg_tbuf, "(ERR) p3net_status: Failed to get error value for\
 Socket %d: %s\n", ns_net->sd, strerror(errno));
	  p3NET_MESSAGE(msg_tbuf)
	  stat = -1;
	}
	else if (ns_soval)
	{
	  sprintf(msg_tbuf, "(WARN) p3net_status: Connection error: %s\n",
			strerror(ns_soval));
	  p3NET_MESSAGE(msg_tbuf)
	  stat = -1;
	}
	if (stat < 0)
	  p3session(p3STOP, ns_net);

  return(stat);

} /* end p3net_status */
