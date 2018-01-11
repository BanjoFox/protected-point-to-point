/**
 * \file p3net.c
 * <h3>Protected Point to Point network functions file</h3>
 *
 * Copyright (C) Velocite 2010
 *
 * The network handler is used to manage communication between
 * P3 primaries and secondaries during anonymous connections.
 */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <p><hr><hr>
 * \section P3_NET P3 Network Messages
 */


#include "p3net.h"
#include "p3utils.h"

/** Network message queue */
char *p3msgq[NET_MSG_MAX];
/** Network message buffer */
char p3nmsg[128];
/** Index of network message queue */
int p3midx = 0;
/** Index of network message queue */
int p3pidx = 0;

#ifdef _p3_PRIMARY
/**
 * \par Function:
 *   p3listen
 *
 * \par Description:
 *   Open or close a listener socket.
 *
 * \par Inputs:
 * - type: Operation type
 *   - p3START:  Attempt to open a socket for listening
 *   - p3STOP:  Close a listeners socket
 * - net: Network structure defining local host
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = 0
 *   - <0 = Error
 *   - >0 = Informational
 */

int p3listen(int type, p3network *net)
{
	int stat = 0, opton = 1;
	int sd = 0, fc;
	struct addrinfo adin, *ai = &(adin), *tai, *tai1;
	struct sockaddr_in *sa_v4;
	struct sockaddr_in6 *sa_v6;

	p3midx = 0;

	if (net == NULL) {
		stat = -1;
		goto out;
	}

	// Start listening
	if (type == p3START) {
		// Get network address information
		memset(ai, 0, sizeof(adin));
		if (net->flag & NET_V4ONLY)
			ai->ai_family = PF_INET;
		else if (net->flag & NET_V6ONLY)
			ai->ai_family = PF_INET6;
		else
			ai->ai_family = PF_UNSPEC;
		ai->ai_socktype = SOCK_STREAM;
		ai->ai_flags = AI_PASSIVE | AI_NUMERICHOST;
		if ((stat = getaddrinfo(net->addr, NULL, ai, &tai)) != 0) {
			sprintf(p3nmsg, "(ERR) p3listen: Unable to get network address\
 information: %s", gai_strerror(stat));
			NET_MESSAGE(p3nmsg);
			stat = -1;
			goto out;
		}

		for (tai1 = tai; tai1; tai1 = tai1->ai_next) {
			if (tai1->ai_family == AF_INET) {
				sa_v4 = (struct sockaddr_in *) tai1->ai_addr;
				sa_v4->sin_port = ntohs((uint16_t) net->port);
			} else if (tai1->ai_family == AF_INET6) {
				sa_v6 = (struct sockaddr_in6 *) tai1->ai_addr;
				sa_v6->sin6_port = ntohs((uint16_t) net->port);
			}
			// Open a socket
			if ((sd = socket(tai1->ai_family, SOCK_STREAM, 0)) < 0) {
				net->flag |= errno;
				sprintf(p3nmsg, "(ERR) p3listen: Unable to open socket: %s",
					strerror(errno));
				NET_MESSAGE(p3nmsg);
				stat = 0;
				continue;
			// Reuse address to allow faster restart
			} else if ((stat = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,
						&opton, sizeof(opton))) < 0) {
				net->flag |= errno;
				sprintf(p3nmsg, "(ERR) p3listen: Unable to set socket '%d'\
 to reuse address: %s", sd, strerror(errno));
				NET_MESSAGE(p3nmsg);
				close(sd);
				stat = 0;
#ifdef IPV6_V6ONLY
			// Define support for IPV6 only
			} else if (tai1->ai_family == AF_INET6 &&
				(stat = setsockopt(sd, IPPROTO_IPV6, IPV6_V6ONLY,
						&opton, sizeof(opton))) < 0) {
				net->flag |= errno;
				sprintf(p3nmsg, "(ERR) p3listen: Unable to set socket '%d'\
 to IPV6 Only: %s", sd, strerror(errno));
				NET_MESSAGE(p3nmsg);
				close(sd);
				stat = 0;
#endif /* IPV6_V6ONLY */
			// Bind socket to address and port
			} else if ((stat = bind(sd, tai1->ai_addr, tai1->ai_addrlen)) < 0) {
				net->flag |= errno;
				sprintf(p3nmsg, "(ERR) p3listen: Unable to bind socket '%d':\
 %s", net->sd, strerror(errno));
				NET_MESSAGE(p3nmsg);
				close(sd);
				stat = 0;
			// Listen on socket
			} else if ((stat = listen(sd, SOMAXCONN)) < 0) {
				net->flag |= errno;
				sprintf(p3nmsg, "(ERR) p3listen: Unable to listen on socket\
 '%d': %s", sd, strerror(errno));
				NET_MESSAGE(p3nmsg);
				close(sd);
				stat = 0;
			// Set socket to non-blocking
			} else {
				fc = fcntl(sd, F_GETFL, 0);
				if ((stat = fcntl(sd, F_SETFL, fc | O_NONBLOCK)) < 0) {
					net->flag |= errno;
					sprintf(p3nmsg, "(WARN) p3listen: Unable to set socket\
 '%d' to nonblocking: %s", sd, strerror(errno));
					NET_MESSAGE(p3nmsg);
					stat = 1;
				}
				sprintf(p3nmsg, "(NOTE) Ready for requests on port '%d',\
 socket '%d'", net->port,  sd);
				NET_MESSAGE(p3nmsg);
				memcpy(&(net->ss), tai1, sizeof(net->ss));
			}
		}
		// Release addrinfo structures
		freeaddrinfo(tai);
		if (sd > 0)
			net->sd = sd;
		else {
			NET_MESSAGE("(ERR) p3listen: No sockets available for listener");
			stat = -1;
			goto out;
		}

	// Stop listening
	} else if (type == p3STOP) {
		if ((net->sd > 0) && ((stat = close(net->sd)) == 0)) {
			sprintf(p3nmsg, "(NOTE) p3listen: Listener on port '%d' socket\
 '%d' disconnected", net->port,  net->sd);
			NET_MESSAGE(p3nmsg);
		} else if (stat != 0) {
			sprintf(p3nmsg, "(ALERT) p3listen: Error stopping listener on\
 socket '%d'", net->sd);
			NET_MESSAGE(p3nmsg);
			stat = 1;
		}
		net->sd = 0;
	}

out:
	return(stat);

} /* end p3listen */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>p3listen: Unable to get network address information:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 network handler is attempting to open a socket for listening
 * but was unable to get the local host address.
 * \par Response:
 * Troubleshoot the problem based on the error reason.
 *
 * <hr><b>p3listen: Unable to open socket:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 network handler is attempting to open a socket for listening
 * but failed.
 * \par Response:
 * Troubleshoot the problem based on the error reason.
 *
 * <hr><b>p3listen: Unable to set socket <i>socket</i> to reuse address:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 network handler attempts to set the socket option to reuse
 * addresses to make processing more efficient.
 * \par Response:
 * Troubleshoot the problem based on the error reason.
 *
 * <hr><b>p3listen: Unable to bind socket: <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 network handler is attempting to open a socket for listening
 * but failed.
 * \par Response:
 * Troubleshoot the problem based on the error reason.
 *
 * <hr><b>p3listen: Unable to listen on socket <i>socket</i>:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 network handler is attempting to open a socket for listening
 * but failed.
 * \par Response:
 * Troubleshoot the problem based on the error reason.
 *
 * <hr><b>p3listen: Unable to set socket <i>socket</i> to nonblocking:
 * <i>error reason</i></b>
 * \par Description (WARN):
 * The P3 network handler is attempting to open a socket for listening
 * but failed.
 * \par Response:
 * Troubleshoot the problem based on the error reason.
 *
 * <hr><b>p3listen: No sockets available for listener</b>
 * \par Description (ERR):
 * The system does not have any available sockets for listening.
 * \par Response:
 * Troubleshoot the operating system problem.
 *
 * <hr><b>p3listen: Ready for requests on port <i>port</i>,
 * socket <i>socket</i></b>
 * \par Description (NOTE):
 * The P3 network handler has successfully opened a socket for listening.
 * \par Response:
 * No action required.
 *
 * <hr><b>p3listen: Error stopping listener on socket <i>socket</i></b>
 * \par Description (ALERT):
 * There was an error closing a listening socket.  The application continues
 * running, but there may be problems later.
 * \par Response:
 * Troubleshoot the operating system problem.
 *
 * <hr><b>p3listen: Listener on port <i>port</i>,
 * socket <i>socket</i> disconnected</b>
 * \par Description (NOTE):
 * The P3 network handler has successfully closed a listening socket.
 * \par Response:
 * No action required.
 *
 */

/**
 * \par Function:
 *	p3listener
 *
 *\par Description:
 *   Accept or reject listen requests.  The callback routine, p3validate,
 *   is called with an allocated p3network structure which it must deallocate.
 *   It validates or rejects the connection request.  The p3validate function
 *   returns an integer with the following meanings:
 *   - 0 = Session accepted
 *   - <0 = Session rejected
 *   - >0 = Session already established
 *
 * \par Inputs:
 * - sd: Socket descriptor of connection request
 * - net: Network structure of local host
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = 0
 *   - <0 = Error or connection was refused
 *   - >0 = INFO, connection already established and caller
 *          should send a NOOP if net->sd > 0
 * - net->flag & NET_ERRNO = errno if applicable
 */

int p3listener(int sd, p3network *net)
{
	int i, stat = 1;
	int new_sd;
	unsigned int salen = sizeof(struct sockaddr_storage);
	char host[MAXHOSTNAMELEN+1];
	struct sockaddr *sa = (struct sockaddr *) &(net->ss);
	p3network *tnet;
	struct linger lr;

	p3midx = 0;
	net->flag &= ~NET_ERRNO;

	// Accept the request
	if ((new_sd = accept(sd, sa, &salen)) <= 0 ) {
		if (new_sd < 0) {
			net->flag |= errno;
			sprintf(p3nmsg, "(ERR) p3listener: Error accepting connection:\
 %s", strerror(errno));
			NET_MESSAGE(p3nmsg);
			stat = -1;
		}
		goto out;
	}

	if ((stat = getnameinfo(sa, salen, host, MAXHOSTNAMELEN+1,
			NULL, 0, NI_NUMERICHOST)) < 0) {
		net->flag |= errno;
		sprintf(p3nmsg, "(ERR) p3listener: Error formatting connected\
 host information: %s", strerror(errno));
		NET_MESSAGE(p3nmsg);
		stat = -1;
		goto out;
	}

	// Validate request and initialize socket
	if ((tnet = (p3network *) p3malloc(sizeof(p3network))) == NULL) {
		sprintf(p3nmsg, "p3listener: Failed to allocate network structure: %s",
				strerror(errno));
		NET_MESSAGE(p3nmsg);
		stat = -1;
		goto out;
	}
	tnet->addr = host;
	tnet->sd = new_sd;
	tnet->flag &= ~NET_ST_DISC;
	tnet->flag |= (NET_ST_UP | NET_ST_NEW);
	if ((stat = p3validate(tnet)) != 0) {
		// Connection exists
		if (stat > 0) {
			sprintf(p3nmsg, "(WARN) p3listener: Session already\
 established with %s", host);
			NET_MESSAGE(p3nmsg);
			shutdown(new_sd, SHUT_RDWR);
			free(tnet);
			stat = -1;
		// Connection rejected
		} else {
			sprintf(p3nmsg, "(ALERT) p3listener: Connection not accepted\
 from host %s", p3buf);
			NET_MESSAGE(p3nmsg);
			shutdown(new_sd, SHUT_RDWR);
			free(tnet);
		}
	}

	sprintf(p3nmsg, "(NOTE) p3listener: Connection accepted from %s,\
 on socket '%d'", host, new_sd);
	NET_MESSAGE(p3nmsg);

	// Set to nonblocking
	if (tnet->flag & NET_NOBLK) {
		i = fcntl(tnet->sd, F_GETFL, 0);
		if (fcntl(tnet->sd, F_SETFL, i | O_NONBLOCK) < 0) {
			sprintf(p3nmsg, "(NOTE) p3listener: Session not set to\
 nonblocking: %s", strerror(errno));
			NET_MESSAGE(p3nmsg);
		}
	}

	// For sessions that are ending, do not attempt to send remaining data
	lr.l_onoff = 1;
	lr.l_linger = 0;
	if (setsockopt(tnet->sd, SOL_SOCKET, SO_LINGER, &lr, sizeof(lr)) < 0) {
		sprintf(p3nmsg, "(ERR) p3listener: Session not set for fast\
 close: %s", strerror(errno));
		NET_MESSAGE(p3nmsg);
	}

out:
	return(stat);

} /* end p3listener */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>p3listener: Error accepting connection: <i>error reason</i></b>
 * \par Description (ERR):
 * The attempt to accept a connection failed for the displayed reason.
 * \par Response:
 *   Troubleshoot operating system or network problem.
 *
 * <hr><b>p3listener: Error formatting connected host information: <i>error reason</i></b>
 * \par Description (ERR):
 *   Attempt to accept connection failed for displayed reason
 * \par Response:
 *   Troubleshoot operating system or network problem.
 *
 * <hr><b>p3listener: Failed to allocate network structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 network listener attempts to allocate a network structure
 * for a connecting host.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>p3listener: Session reset for new session with <i>host</i></b>
 * \par Description (WARN):
 *   There was an existing session with the host requesting a new
 *   session.  However, checking the status indicated that the
 *   existing session had failed.  It is assumed that this is a
 *   valid attempt to re-establish the session.
 * \par Response:
 *   Verify problem is not an intrusion, and correct if it is.
 *
 * <hr><b>p3listener: Session already established with <i>host</i></b>
 * \par Description (WARN):
 *   A session already exists with the host requesting a new session.
 * \par Response:
 *   Verify problem is not an intrusion, and correct if it is.
 *
 * <hr><b>p3listener: Connection not accepted from <i>host</i> port '<i>port</i>'
 *  on socket '<i>socket</i>'</b>
 * \par Description (ALERT):
 *   Attempt to accept connection failed for displayed reason
 * \par Response:
 *   Troubleshoot operating system or network problem.
 *
 * <hr><b>p3listener: Connect accepted from <i>host</i>, port '<i>port</i>' on
 *   socket '<i>socket</i>'</b>
 * \par Description (NOTE):
 *   The connection has been accepted and is ready for data transfer.
 * \par Response:
 *   None.
 *
 * <hr><b>p3listener: Session not set to nonblocking: <i>error reason</i></b>
 * \par Description (NOTE):
 *   The session will wait for data reads and writes to complete before
 *   returning control to the application.  This will affect performance.
 * \par Response:
 *   Troubleshoot operating system based on the error reason.
 *
 * <hr><b>p3listener: Session not set for fast close: <i>error reason</i></b>
 * \par Description (ERR):
 *   The attempt to set the socket to close immediately failed.  The
 *   connection will continue, but if the are network problems, there
 *   may be problems with the application.
 * \par Response:
 *   Troubleshoot operating system or network problem.
 *
 *
 */
#endif

/**
 * \par Function:
 *   p3session
 *
 * \par Description:
 *   Establish or close a session with a remote host.
 *
 * \par Inputs:
 * - type: Operation type
 *   - p3START:  Attempt to establish a session
 *   - p3STOP:  Disconnect a session
 * - net: Remote host network information structure.
 *        <i>The p3network->addr field must be set prior to
 *        calling this function.</i>
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = 0
 *   - <0 = Error
 *   - >0 = Informational
 */

int p3session(int type, p3network *net)
{
	int stat = 0, scount = 0;
	int sd = 0, fc;
	struct addrinfo adin, *ai = &(adin), *tai, *tai1;
	struct sockaddr_in *sa_v4;
	struct sockaddr_in6 *sa_v6;

	p3midx = 0;

	// Start session
	if (type == p3START) {
	// Get network address information
		memset(ai, 0, sizeof(adin));
		if (net->flag & NET_V4ONLY)
			ai->ai_family = PF_INET;
		else if (net->flag & NET_V6ONLY)
			ai->ai_family = PF_INET6;
		else
			ai->ai_family = PF_UNSPEC;
		ai->ai_socktype = SOCK_STREAM;
		ai->ai_flags = AI_PASSIVE;
		if ((stat = getaddrinfo(net->addr, NULL, ai, &tai)) != 0) {
			sprintf(p3nmsg, "(ERR) p3session: Unable to get network address\
 information: %s", gai_strerror(stat));
			NET_MESSAGE(p3nmsg);
			stat = 1;
			goto out;
		}
		for (tai1 = tai; tai1 && !scount; tai1 = tai1->ai_next) {
			// Set port
			if (tai1->ai_family == AF_INET) {
				sa_v4 = (struct sockaddr_in *) tai1->ai_addr;
				sa_v4->sin_port = ntohs((uint16_t) net->port);
			} else if (tai1->ai_family == AF_INET6) {
				sa_v6 = (struct sockaddr_in6 *) tai1->ai_addr;
				sa_v6->sin6_port = ntohs((uint16_t) net->port);
			}
			// Open a socket
			if ((sd = socket(tai1->ai_family, SOCK_STREAM, 0)) < 0) {
				sprintf(p3nmsg, "(ERR) p3session: Unable to get socket: %s",
					strerror(errno));
				NET_MESSAGE(p3nmsg);
				stat = 0;
				continue;
			}

			// Try to connect to specified host
			if (connect(sd, tai1->ai_addr, tai1->ai_addrlen) < 0) {
				sprintf(p3nmsg, "(ERR) Unable to connect to host '%s': %s",
					net->addr, strerror(errno));
				NET_MESSAGE(p3nmsg);
				stat = 0;
				close(sd);
				continue;
			}
			scount++;
			memcpy(&(net->ss), tai1, sizeof(net->ss));
		}
		// Release addrinfo structures
		freeaddrinfo(tai);
		if (!scount) {
			NET_MESSAGE("(WARN) p3session: Connection to host '%d' not\
 established");
			stat = 1;
			goto out;
		}
		net->sd = sd;

		// Set socket to non-blocking
		fc = fcntl(net->sd, F_GETFL, 0);
		fcntl(net->sd, F_SETFL, fc | O_NONBLOCK);
		sprintf(p3nmsg, "(NOTE) p3session: Session ready on socket '%d',\
 port '%d'", net->sd, net->port);
		NET_MESSAGE(p3nmsg);
	}

	// Stop session
	if (type == p3STOP) {
		if ((net->sd > 0) &&
			((stat = shutdown(net->sd, SHUT_RDWR)) == 0))
		{
			sprintf(p3nmsg, "(NOTE) p3session: Session on socket '%d'\
 disconnected", net->sd);
			NET_MESSAGE(p3nmsg);
		}
		if (stat < 0) {
			net->flag |= errno;
			sprintf(p3nmsg, "(WARN) p3session: Error stopping session on\
 socket '%d'", net->sd);
			NET_MESSAGE(p3nmsg);
			stat = 1;
		}
		net->sd = 0;
		net->flag &= ~(NET_ST_UP | NET_ST_NEW);
	}

out:
	return(stat);

} /* end p3session */

 /**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>p3session: Unable to get network address information: <i>error reason</i></b>
 * \par Description (ERR):
 *   Attempt to get network address information failed for displayed reason.
 * \par Response:
 *   Troubleshoot operating system or network problem.
 *
 * <hr><b>p3session: Unable to get socket: <i>error reason</i></b>
 * \par Description (ERR):
 *   Attempt to get network session socket failed.
 * \par Response:
 *   Troubleshoot operating system or network problems
 *
 * <hr><b>p3session: Unable to connect to host '<i>host</i>'</b>
 * \par Description (ERR):
 *   Attempt to connect to specified host failed.
 * \par Response:
 *   Troubleshoot operating system or network problems
 *
 * <hr><b>p3session: Connection to host '<i>host</i>' not established</b>
 * \par Description (WARN):
 *   Network connection attempt failed, see previous messages for details
 * \par Response:
 *   Troubleshoot operating system or network problem.
 *
 * <hr><b>p3session: Session ready on socket '<i>socket</i>', port '<i>port</i>'</b>
 * \par Description (NOTE):
 *   Network session is connected and ready for data.
 * \par Response:
 *   None
 *
 * <hr><b>p3session: Session on socket '<i>socket</i>' disconnected</b>
 * \par Description (NOTE):
 *   Network session has been disconnected.
 * \par Response:
 *   None
 *
 * <hr><b>p3session: Session on socket '<i>socket</i>' not disconnected</b>
 * \par Description (WARN):
 *   Network session has been disconnected.
 * \par Response:
 *   Verify problem is not an intrusion, and correct
 *
 */

/**
 * \par Function:
 *   net_transmit
 *
 * \par Description:
 *   Send buffered data.  If there are multiple data buffers queued,
 *   as many as possible are sent.  If there is more data to be sent,
 *   an informational code is returned.  If there is an error with the
 *   connection, the socket is closed and an error returned.
 *
 * \par Inputs:
 * - net: Network structure for remote host connection
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = 0
 *   - <0 = Error
 *   - >0 = INFO, connection would block, try again later
 */

int net_transmit(p3network *net)
{
	int i, stat = 0;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	if ((stat = net_status(net)) < 0) {
		stat = 1;
		goto out;
	}
	p3midx = 0;

	// Send data as long as it is available
	while (net->idx < net->len) {
		if ((stat = write(net->sd, &(net->data[net->idx]),
						(net->len - net->idx))) < 0) {
			// Session has ended
			if (errno == EPIPE) {
				sprintf(p3nmsg, "(WARN) net_transmit: Session ended");
				NET_MESSAGE(p3nmsg);
				stat = 1;
				p3session(p3STOP, net);
			} else if (errno != EAGAIN) {
				net->flag |= errno;
				sprintf(p3nmsg, "(ERR) net_transmit: Error writing data:\
 %s", strerror(errno));
				NET_MESSAGE(p3nmsg);
				stat = 1;
				p3session(p3STOP, net);
			// Network is busy try again later
			} else {
				net->flag |= NET_INCMPLT;
				stat = 1;
			}
			goto out;
		} else {
			net->idx += stat;
		}

		// Make sure connection is still valid
		if ((i = net_status(net)) < 0) {
			stat = 1;
			goto out;
		}
	}

out:
	return(stat);

} /* end net_transmit */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>net_transmit: Session ended</b>
 * \par Description (WARN):
 *   The network session ended, which is not expected behavior from
 *   the perspective of the transmitter.
 * \par Response:
 *   Troubleshoot network or remote host error.
 *
 * <hr><b>net_transmit: Error writing data: <i>error reason</i></b>
 * \par Description (ERR):
 *   A network error was detected while trying to send data.
 * \par Response:
 *   Troubleshoot system errors.
 *
 */

/**
 * \par Function:
 *   net_receive
 *
 * \par Description:
 *   Receive data into data buffer.  Each message is prefixed with a 2 octet
 *   length field which is read first.  Then a data buffer is allocated for
 *   the entire message.  If there is an errror with the connection, the
 *   socket is closed and an error returned.
 *
 * \par Inputs:
 * - net: Network structure for remote host connection
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = 0
 *   - <0 = Error
 */

int net_receive(p3network *net)
{
	int i, stat = 0, loop = 100, numfds = net->sd + 1;
	int idx = 0, len = 2;
	unsigned char msg_size[4], *buf;
	fd_set fdset;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	if ((stat = net_status(net)) < 0)
		goto out;

	p3midx = 0;
	for (i=0; i < 4; i++)
		msg_size[i] = '\0';
	// TODO: Reuse data buffer if possible
	net->len = 0;
	net->idx = 0;
	if (net->data != NULL) {
		free(net->data);
		net->data = NULL;
	}

	// Receive data as long as it is available
	while (1) {
		// Get inbound data length
		if (!net->len) {
			buf = &msg_size[idx];
		// Get Network data buffer
		} else {
			if (net->data == NULL &&
					(net->data = (unsigned char *) p3malloc(net->len)) == NULL) {
				sprintf(p3nmsg, "net_receive: Failed to allocate data buffer: %s",
						strerror(errno));
				NET_MESSAGE(p3nmsg);
				stat = -1;
				goto out;
			}
			buf = &net->data[net->idx];
			len = net->len;
			idx = net->idx;
		}

		// Get data
		if ((stat = read(net->sd, buf, len)) <= 0) {
			// Session error
			if (stat < 0) {
				net->flag |= errno;
				sprintf(p3nmsg, "(ERR) net_receive: Error reading data: %s",
						strerror(errno));
				NET_MESSAGE(p3nmsg);
			}
			p3session(p3STOP, net);
			stat = -1;
			goto out;
		} else {
			if (!net->len) {
				idx += stat;
				if (idx == len) {
					net->len = buf[0];
					net->len <<= 8;
					net->len |= buf[1];
					if (!net->len) {
						sprintf(p3nmsg, "(ERR) net_receive: Invalid inbound\
 data length: %d", net->len);
						NET_MESSAGE(p3nmsg);
						stat = -1;
						p3session(p3STOP, net);
						goto out;
					}
				}
				continue;
			} else {
				net->idx += stat;
				if (net->idx == net->len) {
					stat = 0;
					goto out;
				}
			}
		}

		// Wait for remaining data (100 loops * 100 msec = 10 sec)
rec_wait:
		if (!loop--) {
			sprintf(p3nmsg, "(ERR) net_receive: Session timed out");
			NET_MESSAGE(p3nmsg);
			stat = -1;
			p3session(p3STOP, net);
			goto out;
		}
		FD_ZERO(&fdset);
		FD_SET(net->sd, &fdset);
		tv.tv_sec = 0;
		tv.tv_usec = p3MILLISEC(100);
		if ((stat = select(numfds, &fdset, 0, 0, &tv)) < 0) {
			net->flag |= errno;
			sprintf(p3nmsg, "(WARN) net_receive: Error waiting for data:\
 %s", strerror(errno));
			NET_MESSAGE(p3nmsg);
			stat = -1;
			p3session(p3STOP, net);
			goto out;
		} else if (!(FD_ISSET(net->sd, &fdset)))
			goto rec_wait;
	}

out:
	if ((stat = net_status(net)) < 0)
		stat = 1;

	return(stat);

} /* end net_receive */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>net_receive: Failed to allocate receive structure:
 *   <i>error reason</i></b>
 * \par Description (ERR):
 *   The network manager uses system memory.  If the receive structure
 *   cannot be allocated, then the application will have to be restarted.
 * \par Response:
 *   Troubleshoot the problem based on the error reason.
 *
 * <hr><b>net_receive: Error reading data: <i>error reason</i></b>
 * \par Description (ERR):
 *   A network error was detected while trying to receive data.
 * \par Response:
 *   Troubleshoot system error based on system code.
 *
 * <hr><b>net_receive: Session ended while receiving data</b>
 * \par Description (ERR):
 *   A network error was detected while trying to receive data.
 * \par Response:
 *   Troubleshoot system error based on system code.
 *
 * <hr><b>net_receive: Error waiting for data: <i>error reason</i></b>
 * \par Description (WARN):
 *   A network error was detected while checking for more data to receive.
 * \par Response:
 *   Troubleshoot system problem based on the error reason.
 *
 */

/**
 * \par Function:
 *   net_status
 *
 * \par Description:
 *   Check the status of the connection.  If an error is found, close
 *   the socket and return an error.
 *
 * \par Inputs:
 * - net: Network structure for remote host connection
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = 0
 *   - <0 = Error
 */

int net_status(p3network *net)
{
	int stat = 0;
	unsigned int soval, solen = sizeof(int);

	if ((stat = getsockopt(net->sd, SOL_SOCKET, SO_ERROR,
		&soval, &solen)) < 0) {
		sprintf(p3nmsg, "(ERR) net_status: Failed to get error value for\
 Socket %d: %s\n", net->sd, strerror(errno));
		NET_MESSAGE(p3nmsg);
		stat = -1;
	} else if (soval) {
		sprintf(p3nmsg, "(WARN) net_status: Connection error: %s\n",
			strerror(soval));
		NET_MESSAGE(p3nmsg);
		stat = -1;
	}
	if (stat < 0)
		p3session(p3STOP, net);

	return(stat);

} /* end net_status */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>net_status: Failed to get error value for Socket:
 *   <i>error reason</i></b>
 * \par Description (ERR):
 *   If a connection is lost outside of a transfer, then the status
 *   should be available through the getsockopt call.  However, the attempt
 *   to get the status failed.  It is assumed that the session has failed,
 *   and it is handled that way.
 * \par Response:
 *   Troubleshoot system problem based on the error reason.
 *
 * <hr><b>net_status: Connection error: <i>error reason</i></b>
 * \par Description (WARN):
 *   If a data connection is lost outside of a transfer, then the status
 *   should be available through the getsockopt call.  All errors result
 *   in the session being shut down but the Spooler continues processing.
 * \par Response:
 *   Troubleshoot network problem based on the error reason.
 *
 */

/**
 * \par Function:
 *   net_ready
 *
 * \par Description:
 *   Test for whether data is ready on a connection.
 *
 * \par Inputs:
 * - net: Network structure for remote host connection
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = No data ready
 *   - 1 = Data ready
 *   - <0 = Error
 */

int net_ready(p3network *net)
{
	int stat = 0;
	int sd = net->sd, numfds = sd + 1;
	fd_set fdset;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	FD_ZERO(&fdset);
	FD_SET(sd, &fdset);
	if ((stat = select(numfds, &fdset, 0, 0, &tv)) < 0) {
		sprintf(p3nmsg, "(WARN) net_ready: Error checking for data: %s",
			strerror(errno));
		NET_MESSAGE(p3nmsg);
	} else if (FD_ISSET(net->sd, &fdset))
		stat = 1;

	return(stat);

} /* end net_ready */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>net_ready: Error checking for data: <i>error reason</i></b>
 * \par Description (WARN):
 *   A network error was detected while checking for more data to receive.
 * \par Response:
 *   Troubleshoot system problem based on the error reason.
 *
 */
