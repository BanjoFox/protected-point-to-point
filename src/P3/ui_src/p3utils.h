/**
 * \file p3utils.h
 * <h3>Protected Point to Point utils header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3_UTILS_H
#define _p3_UTILS_H

/*****  INCLUDE FILES *****/

#include "p3base.h"

/*****  CONSTANTS  *****/

#define p3MTIME_SZ	32		/* Message time string size */
#define p3ADR_MASK	0x01	/* IP address is in CIDR format */
#define p3ADR_STR	0x02	/* IP address string is not modified */

/*****  MACROS  *****/

/**
 * Macro:
 *   MSG_TIME
 *
 * Description:
 *   Set message timestamp string:
 *   - Mon day, year hh:mm:ss
 *
 * Parameters:
 *   - time_buf: String buffer
 *   - time_val: System time_t value
 *   - tm_stru: Work tm structure
 */

#define MSG_TIME(time_buf, time_val, tm_stru) \
	do { \
		tm_stru = gmtime(time_val); \
		strftime(time_buf, p3MTIME_SZ, "%b %e, %Y %H:%M:%S", tm_stru); \
	} while (0)

/**
 * Macro:
 *   next_token
 *
 * Description:
 *   Get the next token being parsed.  If there is no token,
 *   the pointer is set to NULL.
 *
 * Parameters:
 *   - tstr:  Pointer to the current token
 *   - eol:  End of definition or command line
 */

#define next_token(tstr, eol) \
	do { \
		tstr += strlen(tstr); \
		while (tstr < eol && tstr[0] == '\0') { \
			tstr++; \
		} \
		if (tstr >= eol) \
			tstr = NULL; \
	} while (0)

/**
 * Macro:
 *   NOT_TIME
 *
 * Description:
 *   Test a number for a valid time (in seconds) range.
 *
 * Parameters:
 *   - time: Number to test
 */

#define NOT_TIME(time) \
	!(600 < time && time < 2592000)

/**
 * Macro:
 *   NOT_PACKETS
 *
 * Description:
 *   Test a number for a valid packet count range.
 *
 * Parameters:
 *   - time: Number to test
 */

#define NOT_PACKETS(packet) \
	!(1024 < packet && packet < 0x80000000)

/**
 * Macro:
 *   NOT_PORT
 *
 * Description:
 *   Test a number for a valid port range.
 *
 * Parameters:
 *   - port: Number to test
 */

#define NOT_PORT(port) \
	!(1024 < port && port < 65536)

/*****  DATA DEFINITIONS  *****/

typedef struct _p3util p3util;

/**
 * Structure:
 * p3util
 *
 * \par Description:
 * Description.
 */

struct _p3util {
	char			*message;		/**< Error message buffer */
	unsigned int	flag;
#define p3UTL_NINFO	0x00000001		/* Ignore informational messages */
#define p3UTL_NWARN	0x00000002		/* Ignore warning messages */
};

/*****  PROTOTYPES  *****/

int init_utils();
void p3errmsg(int type, char *message);
char *get_errmsg();
void p3errlog(int type, char *message);
int isallnum(char* parm);
int get_level_idx(char *level[], int tsize, char *token);
int get_comm_data(p3comm *comm, int size);
int parse_ipaddr(char *address, void *net, void *subnet, int flag, char *emsg);

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _p3_UTILS_C
extern p3util *util;
extern char p3buf[p3BUFSIZE];
extern char errmsg[256];
extern char *level1[];
extern char *level2[];
extern char *level3[];
extern char *level4[];
#endif

#endif /* _p3_UTILS_H */
