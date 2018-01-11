/**
 * \file p3utils.c
 * <h3>Protected Point to Point utils file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * Overview of the purpose of the functions in the source file.
 */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <p><hr><hr>
 * \section P3_UTILS P3 Utility Messages
 */

#define _p3_UTILS_C
#include "p3utils.h"
#include "p3system.h"
#include "p3share.h"
#ifndef _p3_SECONDARY
#include "p3pri_key_server.h"
#endif

/** Global utilities structure */
p3util *p3utils = NULL;

/**
 * \par Function:
 * init_utils
 *
 * \par Description:
 * Initialize utilities for the P3 system.
 * 
 * \par Inputs:
 * - msgdir: Directory path for message logs.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_utils(char *msgdir)
{
	int i, stat = 0;
	char *filename;
	unsigned long l;
	FILE *p3dir;

	i = sizeof(p3util) + sizeof(p3mm_anchor);
	if ((p3utils = (p3util *) p3calloc(i)) == NULL) {
		sprintf(p3buf, "init_utils: Failed to allocate p3 utilities data\
 structure: %s\n", strerror(errno));
		p3errmsg(p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	l = (unsigned long) p3utils;
	l += sizeof(p3util);
	p3utils->anchor = (p3mm_anchor *) l;
	p3utils->msgdir = msgdir;

	if ((p3dir = fopen(p3utils->msgdir, "r")) == NULL) {
		sprintf(p3buf, "init_utils: Failed to access message\
 directory: %s\n", strerror(errno));
		p3errmsg(p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	fclose(p3dir);
	i = sizeof(p3MSG_FILE) + strlen(p3utils->msgdir) + 2;
	if ((filename = (char *) p3calloc(i)) == NULL) {
		sprintf(p3buf, "init_utils: Failed to allocate message filename: %s\n",
			strerror(errno));
		p3errmsg(p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	sprintf(filename, "%s/%s", p3utils->msgdir, p3MSG_FILE);
	if ((p3utils->msgfile = open(filename, p3MSG_FLAG, p3MSG_MODE)) < 0) {
		sprintf(p3buf, "init_utils: Failed to open message file: %s\n",
			strerror(errno));
		p3errmsg(p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	p3utils->flag |= p3UTL_MSGOP;

out:
	return(stat);
} /* end init_utils */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>init_utils: Failed to allocate main p3 utilities data structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 system attempts to allocate the utilities structure during
 * initialization.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * init_kernel_comm
 *
 * \par Description:
 * Initialize the buffer used for communication with the kernel module.
 *
 * <b><i>NOTE: The init_key_serv function is only called by a primary to
 * initialize the circular buffer for random numbers.</i></b>
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_kernel_comm()
{
	int stat = 0, fd;

p3errmsg (p3MSG_DEBUG, "Open RAM disk\n");
	if ((fd = open (P3DEVNAME, O_RDWR)) < 0) {
		sprintf (p3buf, "init_kernel_comm: Failed to open ramdisk:  %s\n",
				strerror (errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	p3utils->anchor->fd = fd;
	p3utils->anchor->pid = getpid();

#ifndef _p3_SECONDARY
p3errmsg (p3MSG_DEBUG, "MMap RAM disk\n");
	p3utils->anchor->kserv = (p3key_serv *) mmap (NULL, RAMDISK_SZ,
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (p3utils->anchor->kserv == (p3key_serv *) MAP_FAILED) {
		sprintf(p3buf, "init_kernel_comm: Failed to allocate p3 key server\
 structure: %s\n", strerror(errno));
		p3errmsg(p3MSG_CRIT, p3buf);
		close(fd);
		stat = -1;
		goto out;
	}
	// Initialize key server data structure and circular buffer
p3errmsg (p3MSG_DEBUG, "Init circular buffer\n");
	p3utils->anchor->kserv->cbuf_sz = RAMDISK_SZ - sizeof(p3key_serv);
	p3utils->anchor->kserv->head = p3utils->anchor->kserv->tail = 0;
p3errmsg (p3MSG_DEBUG, "Init key server\n");
	init_key_serv(p3utils->anchor->kserv);
#endif

out:
	return(stat);
} /* end init_kernel_comm */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>init_kernel_comm: Failed to open ramdisk: <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 system attempts to open the RAM disk device for mmap'ed communication
 * with the kernel module.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>init_kernel_comm: Failed to allocate p3 mmap anchor structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 system attempts to allocate the structure used for mmap'ed communication
 * with the kernel module.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>init_kernel_comm: Failed to initialize ramdisk device: <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 system attempts to send initialization information to the ramdisk device
 * to complete the initialization of the kernel module.  If this fails, there is
 * a system problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

unsigned short csum (unsigned short *buf, int nwords)
{
	unsigned long sum;
	for (sum = 0; nwords > 0; nwords--)
		sum += *buf++;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return ~sum;
}

/**
 * \par Function:
 * init_raw_socket
 *
 * \par Description:
 * Initialize the Raw socket used by the kernel module to send control messages.
 *
 * \par Inputs:
 * - host: P3host structure with destination information.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_raw_socket(p3host *host)
{
	int len, stat = 0, sd, sopt = 1;
	unsigned char rawmsg[64];
	struct sockaddr_in sa4;
	struct sockaddr_in6 sa6;
	struct ip *iph = (struct ip *) rawmsg;

	// Initialize Raw socket
	if ((sd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		sprintf (p3buf, "init_raw_socket: Failed to open Raw socket:  %s\n", strerror (errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	// Set socket option to build entire packet
	if(setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &sopt, sizeof(sopt)) < 0) {
		sprintf (p3buf, "init_raw_socket: Failed to set Raw socket options:  %s\n",
				strerror (errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	if (host->flag & p3HST_IPV4) {
		// Build socket address structure
		memset(&sa4, 0, sizeof(struct sockaddr_in));
		sa4.sin_family = AF_INET;
		memcpy(&sa4.sin_addr, &host->addr.v4, sizeof(struct in_addr));
		// Build packet
		len = 28;
		memset(rawmsg, 0, len);
		iph->ip_v = 4;
		iph->ip_hl = 5;
		iph->ip_len = len;
		iph->ip_ttl = 128;
		iph->ip_p = p3PROTO;
//		rawmsg[9] = p3PROTO;     // P3 private encryption protocol
		memcpy(&rawmsg[12], &p3utils->lochost.v4, sizeof(struct in_addr));
		memcpy(&rawmsg[16], &host->addr.v4, sizeof(struct in_addr));
		iph->ip_sum = csum ((unsigned short *) rawmsg, iph->ip_len >> 1);
	} else if (host->flag & p3HST_IPV6) {
		// TODO: Support IPv6 in init raw socket
		p3errmsg (p3MSG_ERR, "init_raw_socket: IPv6 not yet implemented\n");
		stat = -1;
		goto out;
	} else {
		p3errmsg (p3MSG_CRIT, "init_raw_socket: Invalid IP version\n");
		stat = -1;
		goto out;
	}
p3errmsg(p3MSG_DEBUG, "Send Raw packet:\n");
p3buf[0] = 0;
for (stat=0; stat < len; stat++) {
	if (!(stat & 3))
		sprintf(p3buf, "%s ", p3buf);
	sprintf(p3buf, "%s%2.2x", p3buf, rawmsg[stat]);
}
sprintf(p3buf, "%s\n", p3buf);
p3errmsg(p3MSG_DEBUG, p3buf);
stat = 0;
	if (sendto(sd, rawmsg, len, 0, (struct sockaddr *)&sa4, sizeof(sa4)) < 0) {
		sprintf (p3buf, "init_raw_socket: Failed to send packet to Raw socket:  %s\n",
				strerror (errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}

out:
	return(stat);
} /* end init_raw_socket */

/**
 * \par Function:
 * init_raw_socket_old
 *
 * \par Description:
 * Initialize the Raw socket used by the kernel module to send control messages.
 *
 * \par Inputs:
 * - ipver: The IP version of the interface to be used for the Raw socket.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_raw_socket_old(int ipver)
{
	int len, stat = 0, sd, sopt = 1;
	unsigned char rawmsg[64];
	struct sockaddr_in sa4;
	struct sockaddr_in6 sa6;

	// Initialize Raw socket
	if ((sd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		sprintf (p3buf, "init_raw_socket_old: Failed to open Raw socket:  %s\n", strerror (errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	// Set socket option to build entire packet
	if(setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &sopt, sizeof(sopt)) < 0) {
		sprintf (p3buf, "init_raw_socket_old: Failed to set Raw socket options:  %s\n",
				strerror (errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	if (ipver == 4) {
		// Build socket address structure
		memset(&sa4, 0, sizeof(struct sockaddr_in));
		sa4.sin_family = AF_INET;
//		memcpy(&sa4.sin_addr, &p3utils->lochost.v4, sizeof(struct in_addr));
		// Build packet
		len = 28;
		memset(rawmsg, 0, len);
		rawmsg[0] = 0x45;   // Set version and header length
		rawmsg[1] = 0;      // Type of service
		rawmsg[3] = 28;     // Total length
		rawmsg[6] = 0x40;   // Don't fragment
		rawmsg[8] = 128;    // TTL
		rawmsg[9] = 50;     // P3 uses modified ESP protocol
//		memcpy(&rawmsg[12], &p3utils->lochost.v4, sizeof(struct in_addr));
//		memcpy(&rawmsg[16], &p3utils->remhost.v4, sizeof(struct in_addr));
	} else if (ipver == 6) {
		// TODO: Support IPv6 in init raw socket
		p3errmsg (p3MSG_CRIT, "init_raw_socket_old: IPv6 not yet implemented\n");
		stat = -1;
		goto out;
	} else {
		p3errmsg (p3MSG_CRIT, "init_raw_socket_old: Invalid IP version\n");
		stat = -1;
		goto out;
	}
p3errmsg(p3MSG_DEBUG, "Send Raw packet\n");
	if (sendto(sd, rawmsg, len, 0, (struct sockaddr *)&sa4, sizeof(sa4)) < 0) {
		sprintf (p3buf, "init_raw_socket_old: Failed to send packet to Raw socket:  %s\n",
				strerror (errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}

out:
	return(stat);
} /* end init_raw_socket_old */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>init_raw_socket: Failed to open Raw socket: <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 system attempts to open the Raw socket for sending control packets.
 * from the kernel module.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>init_raw_socket: Failed to set Raw socket options:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 system attempts to set the socket options to control the Raw
 * socket.  If this fails, there is a system problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>init_raw_socket: Failed to send packet to Raw socket:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 system attempts to send a packet to the Raw socket to create
 * the structures needed by the kernel module.  If this fails, there
 * is a system problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>init_raw_socket: Invalid IP version</b>
 * \par Description (CRIT):
 * The IP version must be either 4 or 6.
 * \par Response:
 * Report the problem to Velocite Systems support.
 *
 */

/**
 * \par Function:
 * send_ioctl
 *
 * \par Description:
 * Send a buffer to the kernel module using an ioctl command.
 *
 * \par Inputs:
 * - buffer: The data to be sent.
 * - size: The size of the data buffer.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int send_ioctl(unsigned char *buffer, int size)
{
	int stat = 0, cmd;

	cmd = (int) P3_IOW(P3IOC_TYPE, 1, size);
	if ((stat = ioctl(p3utils->anchor->fd, cmd, buffer)) < 0) {
		sprintf(p3buf, "send_ioctl: Error sending ioctl: %s\n", strerror(errno));
		p3errmsg(p3MSG_CRIT, p3buf);
	}

	return (stat);
} /* end send_ioctl */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>send_ioctl: Error sending ioctl: <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 system communicates with the kernel module by sending ioctl commands
 * to the RAM disk.  If this fails, there is a system problem.
 * \par Response:
 * There should be a corresponding message in the system log for troubleshooting.
 * The possible error reasons are:
 * <ul>
 *   <li>Out of memory: The kernel was unable to allocate memory for the P3
 *      application.</li>
 *   <li>Invalid argument: There was an error in the configuration data not
 *      detected by the user space application parser.</li>
 *   <li>Operation not permitted: Encryption initialization failed.</li>
 * </ul>
 *
 */

/**
 * \par Function:
 * receive_ioctl
 *
 * \par Description:
 * Send a buffer to the kernel module using an ioctl command.
 *
 * \par Inputs:
 * - buffer: The data to be sent.
 * - size: The size of the data buffer.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int receive_ioctl(unsigned char *buffer, int size)
{
	int stat, cmd;

	cmd = (int) P3_IOR(P3IOC_TYPE, 1, size);
	if ((stat = ioctl(p3utils->anchor->fd, cmd, buffer)) < 0) {
		sprintf(p3buf, "send_ioctl: Error receiving ioctl: %s\n", strerror(errno));
		p3errmsg(p3MSG_CRIT, p3buf);
	}

	return (stat);
} /* end receive_ioctl */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>receive_ioctl: Error receiving ioctl: <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 system communicates with the kernel module by receiving ioctl commands
 * to the RAM disk.  If this fails, there is a system problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * p3errmsg
 *
 * \par Description:
 * Handle error messages for the P3 system.  The types of messages are
 * defined in the p3base.h include file.  They are:
 * <ul>
 *  <li>Critical:  Caused by system errors</li>
 *  <li>Error:  Caused by application errors</li>
 *  <li>Warning:  Does not cause application shutdown, but indicate the
 *    need for adminstrative correction</li>
 *  <li>Notice:  Information that is always reported, such as startup</li>
 *  <li>Information:  Information such as usage statistics.  This may be
 *    ignored using the -I command line argument</li>
 * </ul>
 * 
 * \par Inputs:
 * - type: The type of the message
 * - message: The text of the message
 *
 * \par Outputs:
 * - None
 */

void p3errmsg(int type, char *message)
{
	char msg_time[64], msg_type[32];
	time_t now = time(NULL);
	const struct tm mtm, *msgtm = &mtm;

	if (type == p3MSG_CRIT)
		strcpy(msg_type, "CRITICAL");
	else if (type == p3MSG_ERR)
		strcpy(msg_type, "ERROR");
	else if (type == p3MSG_WARN)
		strcpy(msg_type, "WARNING");
	else if (type == p3MSG_NOTICE)
		strcpy(msg_type, "NOTICE");
	else if (type == p3MSG_INFO) {
		if (p3utils->flag & p3UTL_NINFO)
			goto out;
		strcpy(msg_type, "INFORMATION");
	}
	else if (type == p3MSG_DEBUG)
		if (p3DEBUG) {
			strcpy(msg_type, "DEBUG");
		} else {
			goto out;
		}
	else
		goto out;

	// TODO: Write to message log if (p3utils->msgdir != NULL
	MSG_TIME(msg_time, &now, msgtm)
	if (p3utils != NULL && (p3utils->flag & p3UTL_MSGOP)) {
		write(p3utils->msgfile, message, strlen(message));
	} else
		fprintf(stderr, "%s (%s) %s", msg_time, msg_type, message);

out:
	return;
} /* end p3errmsg */

/**
 * \par Function:
 * isallnum
 *
 * \par Description:
 * Validate that a string is only base 10 digits.
 *
 * \par Inputs:
 * - parm: The string to be tested.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = Only digits
 *   - <0 = String contains non-digits
 */

int isallnum(char* parm)
{
	int i = 0, j = strlen(parm);

	if (!j)
		return -1;

	for (i = 0 ; i < j; i++) {
		if (!isdigit(parm[i]))
			return -1;
	}

	return 0;
}
/**
 * \par Function:
 * getsubnetcfg
 *
 * \par Description:
 * Allocate a subnet configuration structure and initialize to all zeros.
 *
 * \par Inputs:
 * - None.
 *
 * \par Outputs:
 * - p3subnetcfg *: The new subnet configuration structure or
 *   NULL if there is an error.
 */

p3subnetcfg *getsubnetcfg()
{
	p3subnetcfg *sncfg;

	if ((sncfg = (p3subnetcfg *) p3calloc(sizeof(p3subnetcfg))) == NULL) {
		sprintf(p3buf, "getsubnetcfg: Failed to allocate subnet configuration file: %s\n",
				strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
	}

	return sncfg;
} /* end getsubnetcfg */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>getsubnetcfg: Failed to allocate subnet configuration file:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 primary attempts to allocate the subnet configuration file.
 * If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 */


