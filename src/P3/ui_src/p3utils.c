/**
 * \file p3utils.c
 * <h3>Protected Point to Point utils file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * Overview of the purpose of the functions in the source file.
 */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <p><hr><hr>
 * \section P3_UTILS P3 Utility Messages
 */

#define _p3_UTILS_C
#include "p3utils.h"
#include "p3config.h"
#include "p3command.h"

/** Global utilities structure */
p3util *util;

/** Working buffer */
char p3buf[p3BUFSIZE];

/** Error message buffers */
char ebuf1[(2 * p3BUFSIZE)];
char ebuf2[(2 * p3BUFSIZE)];
char errmsg[256];

/** Parser arrays are used for identifying multi-level commands */
char *level1[p3L1MAX];
char *level2[p3L2MAX];
char *level3[p3L3MAX];
char *level4[p3L4MAX];

/**
 * \par Function:
 * init_utils
 *
 * \par Description:
 * Initialize utilities for the P3 system.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_utils()
{
	int i, stat = 0;

	ebuf1[0] = '\0';
	ebuf2[0] = '\0';
	i = sizeof(p3util);
	if ((util = (p3util *) p3calloc(i)) == NULL) {
		fprintf(stderr, "init_utils: Failed to allocate p3 utilities data\
 structure: %s\n", strerror(errno));
		stat = -1;
		goto out;
	}
	util->message = ebuf1;

out:
	return(stat);
} /* end init_utils */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
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
 * p3errmsg
 *
 * \par Description:
 * Handle error messages for the P3 system.  The types of messages are
 * defined in the p3base.h include file.  They are:
 * - Critical:  Caused by system errors
 * - Error:  Caused by application errors
 * - Warning:  Does not cause application shutdown, but indicate the
 *   need for adminstrative correction
 * - Notice:  Information that is always reported, such as startup
 * - Information:  Information such as usage statistics.  This may be
 *   ignored using the -I command line argument
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
	int i;
	char msg_type[32];

	if (type == p3MSG_CRIT)
		strcpy(msg_type, "(CRITICAL)");
	else if (type == p3MSG_ERR)
		strcpy(msg_type, "(ERROR)");
	else if (type == p3MSG_WARN)
		strcpy(msg_type, "(WARNING)");
	else if (type == p3MSG_NOTICE)
		strcpy(msg_type, "(NOTICE)");
	else if (type == p3MSG_INFO) {
		if (util->flag & p3UTL_NINFO)
			goto out;
		strcpy(msg_type, "(INFORMATION)");
	}
	else if (type == p3MSG_DEBUG)
		strcpy(msg_type, "(DEBUG)");
	else if (type == p3MSG_NONE)
		strcpy(msg_type, "");
	else
		goto out;

	i = strlen(util->message) + strlen(msg_type) + strlen(message);
	if (i < sizeof(ebuf1)) {
		sprintf(util->message, "%s%s %s",
			util->message, msg_type, message);
	}

out:
	return;
} /* end p3errmsg */

/**
 * \par Function:
 * get_errmsg
 *
 * \par Description:
 * Return all errors accumulated during the current processing
 * cycle, and clear the alternate buffer for the next cycle.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - char *:  The current error messages
 */

char *get_errmsg()
{
	if (util->message == ebuf1) {
#ifdef _p3JNI
		strcpy(ebuf2, "ERROR");
#else
		ebuf2[0] = '\0';
#endif
		util->message = ebuf2;
		return (ebuf1);
	} else {
#ifdef _p3JNI
		strcpy(ebuf1, "ERROR");
#else
		ebuf1[0] = '\0';
#endif
		util->message = ebuf1;
		return (ebuf2);
	}
} /* end get_errmsg */

/**
 * \par Function:
 * p3errlog
 *
 * \par Description:
 * Handle error message logging for the P3 anonymous connection
 * handler.  The types of messages are defined in the p3base.h
 * include file.  They are:
 * - Critical:  Caused by system errors
 * - Error:  Caused by application errors
 * - Warning:  Does not cause application shutdown, but indicate the
 *   need for adminstrative correction
 * - Notice:  Information that is always reported, such as startup
 * - Information:  Information such as usage statistics.  This may be
 *   ignored using the -I command line argument
 *
 * \par Inputs:
 * - type: The type of the message
 * - message: The text of the message
 *
 * \par Outputs:
 * - None
 */

void p3errlog(int type, char *message)
{
	char msg_time[64], msg_type[32];
	time_t now = time(NULL);
	struct tm *msgtm;

	if (type == p3MSG_CRIT)
		strcpy(msg_type, "(CRITICAL)");
	else if (type == p3MSG_ERR)
		strcpy(msg_type, "(ERROR)");
	else if (type == p3MSG_WARN)
		strcpy(msg_type, "(WARNING)");
	else if (type == p3MSG_NOTICE)
		strcpy(msg_type, "(NOTICE)");
	else if (type == p3MSG_INFO) {
		if (util->flag & p3UTL_NINFO)
			goto out;
		strcpy(msg_type, "(INFORMATION)");
	}
	else if (type == p3MSG_DEBUG)
		strcpy(msg_type, "(DEBUG)");
	else if (type == p3MSG_NONE)
		strcpy(msg_type, "");
	else
		goto out;

	MSG_TIME(msg_time, &now, msgtm);
	// TODO: Write to message log or shared memory
	printf("%s (%s) %s", msg_time, msg_type, message);
	if (type == p3MSG_DEBUG)
		fflush(stdout);

out:
	return;
} /* end p3errlog */

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
 * get_level_idx
 *
 * \par Description:
 * Find a token in the supplied token list (which is sorted
 * alphabetically) and return its index.
 *
 * \par Inputs:
 * - tlist:	The token list to be searched.
 * - tsize:	The size of the token list.
 * - token:	The token to be found.
 *
 * \par Outputs:
 * - int: Status
 *   - >=0 = Token index
 *   - <0 = Token not found
 */

int get_level_idx(char *tlist[], int tsize, char *token)
{
	int tidx = -1, min = 0, max = tsize, mid = tsize >> 1;

	// Validate token
	if (token == NULL || token[0] == '\0')
		goto out;

	// Search token list
	while (max > min) {
		if (strcmp(token, tlist[mid]) == 0) {
			tidx = mid;
			goto out;
		} else if (strcmp(token, tlist[mid]) < 0) {
			max = mid;
			mid = max - ((max - min) >> 1);
		} else if (strcmp(token, tlist[mid]) > 0) {
			min = mid;
			mid = min + ((max - min) >> 1);
		}
		if ((min + 1) == max) {
			if (strcmp(token, tlist[min]) == 0) {
				tidx = min;
				goto out;
			} else if (max < tsize && strcmp(token, tlist[max]) == 0) {
					tidx = max;
					goto out;
			} else
				goto out;
		}
	}

out:
	return(tidx);
} /* end get_level_idx */

/**
 * \par Function:
 * get_comm_data
 *
 * \par Description:
 * Determine is the communication data buffer can be reused.
 * If not, release an existing one and get the appropriate
 * new buffer and set the length parameter.
 *
 * \par Inputs:
 * - comm: The P3 communication structure
 * - size: The size of data (including a NULL terminator) required
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = Success
 *   - <0 = Error
 */

int get_comm_data(p3comm *comm, int size)
{
	int stat = 0;

	// Buffer is reusable
	if (comm->datlen >= size && comm->data != NULL)
		goto out;

	if (comm->data != NULL) {
		free(comm->data);
		comm->data = NULL;
	}
	comm->datlen = size;
	if (comm->data == NULL && (comm->data = (char *)
			p3malloc(comm->datlen)) == NULL) {
		sprintf(p3buf, "get_comm_data: Failed to allocate\
 communication data buffer: %s\n", strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		comm->datlen = 0;
		stat = -1;
	}

out:
	return(stat);
} /* end get_comm_data */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>get_comm_data: Failed to allocate communication buffer:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 administration library attempts to allocate a buffer to hold the
 * command data.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * parse_ipaddr
 *
 * \par Description:
 * Parse an IP adddress string and set the supplied network
 * structure and, if requested, mask.  The address string is
 * modified unless the p3ADR_STR flag is set.
 *
 * \par Inputs:
 * - address: The text version of the IP address.
 * - net: Pointer to an IP address structure.  Should be NULL if
 *     p3ADR_STR is set.
 * - subnet: Pointer to an IP subnet mask structure.  Should be NULL
 *     if p3ADR_STR is set or p3ADR_MASK is not set.
 * - flag: The requested operations, which may be OR'ed together.
 *   - p3HST_IPV4: IP address is for version 4 (mutually with p3HST_IPV6)
 *   - p3HST_IPV6: IP address is for version 6 (mutually with p3HST_IPV4)
 *   - p3ADR_MASK: The IP address is in CIDR format, so set the mask
 *   - p3ADR_STR: The IP adddress string is not modified
 * - emsg: Pointer to error message buffer.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = Success
 *   - <0 = Error
 */

int parse_ipaddr(char *address, void *net, void *subnet, int flag, char *emsg)
{
	int i, stat = 0, sn, alen;
	unsigned int ui;
	unsigned char *snaddr, *mask;
	char *paddr;
	union {
		struct in_addr	v4;
		struct in6_addr	v6;
	} addr;

	if (flag & p3ADR_STR) {
		strcpy(p3buf, address);
		paddr = p3buf;
	} else
		paddr = address;

	alen = strlen(paddr);
	for (i=0; i< alen; i++) {
		if (paddr[i] == '/') {
			paddr[i] = '\0';
			break;
		}
	}
	i++;

	if (flag & p3HST_IPV4) {
		if (net == NULL)
			net = &addr.v4;
		if (inet_pton(AF_INET, paddr, net) < 0) {
			sprintf(emsg, "Invalid IPv4 address: %s", strerror(errno));
			stat = -1;
			goto out;
		}
		// Set mask
		if (flag & p3ADR_MASK) {
			sn = 32;
			if (i < alen) {
				sn = strtol(&paddr[i], NULL, 10);
				if (!(0 < sn && sn < 33)) {
					sprintf(emsg, "Invalid IPv4 mask bits: %d", sn);
					stat = -1;
					goto out;
				}
			}
			if (!(flag & p3ADR_STR)) {
				mask = (unsigned char *) subnet;
				ui = 0x80;
				i = 0;
				while (sn) {
					if (ui == 0x80)
						mask[i] = 0;
					mask[i] |= ui;
					ui >>= 1;
					if (!ui) {
						i++;
						ui = 0x80;
					}
					sn--;
				}
				// Apply mask to subnet address
				snaddr = (unsigned char *) net;
				for (i=0; i < sizeof(struct in_addr); i++)
					snaddr[i] &= mask[i];
			}
		}
	} else if (flag & p3HST_IPV6) {
		if (net == NULL)
			net = &addr.v6;
		if (inet_pton(AF_INET6, paddr, net) < 0) {
			sprintf(emsg, "Invalid IPv6 address: %s", strerror(errno));
			stat = -1;
			goto out;
		}
		// Set mask
		if (flag & p3ADR_MASK) {
			sn = 64;
			if (i < alen) {
				sn = strtol(&paddr[i], NULL, 10);
				if (!(47 < sn && sn < 65)) {
					sprintf(emsg, "Invalid IPv6 mask bits: %d", sn);
					stat = -1;
					goto out;
				}
			}
			if (!(flag & p3ADR_STR)) {
				mask = (unsigned char *) subnet;
				for (i=0; i < 6; i++)
					mask[i] |= 0xff;
				ui = 0x80;
				sn -= 48;
				while (sn) {
					if (ui == 0x80)
						mask[i] = 0;
					mask[i] |= ui;
					ui >>= 1;
					if (!ui) {
						i++;
						ui = 0x80;
					}
					sn--;
				}
				// Apply mask to subnet address
				snaddr = (unsigned char *) net;
				for (i=0; i < sizeof(struct in6_addr); i++)
					snaddr[i] &= mask[i];
			}
		}
	} else {
		sprintf(emsg, "Unknown IP version");
		stat = -1;
		goto out;
	}

out:
	return(stat);
} /* end parse_ipaddr */

