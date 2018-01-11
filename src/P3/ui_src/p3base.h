/**
 * \file p3base.h
 * <h3>Protected Point to Point product base header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3_BASE_H
#define _p3_BASE_H

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/param.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/stat.h>

/*****  CONSTANTS  *****/

#ifdef _p3_PRIMARY
#define p3APPLICATION	"p3primary"
#else
#define p3APPLICATION	"p3secondary"
#endif

#define p3VERSION		"Version 0.1.0"

/* Build shared library for JNI interface */
#define _p3JNI			1

#define p3START			1			/* Start a feature */
#define p3STOP			2			/* Stop a feature */

#define p3MSG_NONE		0			/**< No message level supplied */
#define p3MSG_CRIT		1			/**< Critical caused by system error */
#define p3MSG_ERR		2			/**< Error caused by application */
#define p3MSG_WARN		3			/**< Warning allows admin correction */
#define p3MSG_NOTICE	4			/**< Notices always displayed */
#define p3MSG_INFO		5			/**< Information for usage stats */
#define p3MSG_DEBUG		6			/**< Debugging messages */

#define p3HST_IPV4		0x00100000  /* Host address is IPv4 */
#define p3HST_IPV6		0x00200000  /* Host address is IPv6 */

#define PATH_SEPARATOR	'/'
#if PATH_MAX < 4096
#define p3BUFSIZE		4096			/* Typical buffer size */
#else
#define p3BUFSIZE		PATH_MAX		/* Typical buffer size */
#endif

#define p3MILLISEC(msec)	(msec * 1000)	/* Milliseconds in timeval struct */
#define p3MICROSEC(msec)	msec			/* Microseconds in timeval struct */

/**
 * Debugging:
 *   p3DBG_type
 *
 * Description:
 *   Fine tune debugging messages by setting the single constant
 *   - p3DEBUG
 */

#ifndef p3DEBUG
#define p3DEBUG			0x000000ff
#endif

#define p3DEBUG_ERR		0x00000001		/* Logic errors */
#define p3DEBUG_WARN	0x00000002		/* Unexpected behavior */
#define p3DEBUG_END		0x00000004		/* Shutdown messages */
#define p3DEBUG_PRGS	0x00000008		/* Trace progress */
#define p3DEBUG_STRU	0x00000010		/* Structure values */
#define p3DEBUG_DATA	0x00000020		/* Data values */
#define p3DEBUG_MEM		0x00000040		/* Memory */
#define p3DEBUG_PERF	0x00000080		/* Performance */

#if p3DEBUG_ERR == (p3DEBUG & p3DEBUG_ERR)
#define p3DBG_ERR		1
#endif
#if p3DEBUG_WARN == (p3DEBUG & p3DEBUG_WARN)
#define p3DBG_WARN		1
#endif
#if p3DEBUG_END == (p3DEBUG & p3DEBUG_END)
#define p3DBG_END		1
#endif
#if p3DEBUG_PRGS == (p3DEBUG & p3DEBUG_PRGS)
#define p3DBG_PRGS		1
#endif
#if p3DEBUG_STRU == (p3DEBUG & p3DEBUG_STRU)
#define p3DBG_STRU		1
#endif
#if p3DEBUG_DATA == (p3DEBUG & p3DEBUG_DATA)
#define p3DBG_DATA		1
#endif
#if p3DEBUG_MEM == (p3DEBUG & p3DEBUG_MEM)
#define p3DBG_MEM		1
#endif
#if p3DEBUG_PERF == (p3DEBUG & p3DEBUG_PERF)
#define p3DBG_PERF		1
#endif

  #ifdef _p3_PRIMARY
#define P3APP	"p3primary"
#define P3INIT	"p3primary_init"
#define P3EXIT	"p3primary_exit"
  #else
	#ifdef _p3_SECONDARY
#define P3APP	"p3secondary"
#define P3INIT	"p3secondary_init"
#define P3EXIT	"p3secondary_exit"
	#else
#define P3APP	"p3primaryplus"
#define P3INIT	"p3primaryplus_init"
#define P3EXIT	"p3primaryplus_exit"
	#endif
  #endif

/**
 * P3 administration tokens are used in a URI format, separated by "/"
 * and top levels define categories of tokens and the lowest level
 * define the actual value.
 *
 * <i>NOTE: The list must be in alphabetical order to allow for
 * binary searches.</i>
 */

#define p3MAX_GROUPS		4
#define p3MAX_TOKENS		4

// NOTE: Insert new tokens in alphabetical order
#define p3L1CMD				0
#define p3L1CMD_TOK			"command"
#define p3L1FREE			(p3L1CMD + 1)
#define p3L1FREE_TOK		"free"
#define p3L1GETNEXT			(p3L1FREE + 1)
#define p3L1GETNEXT_TOK		"getnext"
#define p3L1GROUP			(p3L1GETNEXT + 1)
#define p3L1GROUP_TOK		"group"
#define p3L1LOCAL			(p3L1GROUP + 1)
#define p3L1LOCAL_TOK		"local"
#define p3L1REMOTE			(p3L1LOCAL + 1)
#define p3L1REMOTE_TOK		"remote"
#define p3L1SESSION			(p3L1REMOTE + 1)
#define p3L1SESSION_TOK		"session"
#define p3L1MAX				(p3L1SESSION + 1)
#if p3L1MAX > 127
#error Too many tokens in Level 1
#endif

// NOTE: Insert new tokens in alphabetical order
#define p3L2ADDRESS			0
#define p3L2ADDRESS_TOK		"address"
#define p3L2ANON			(p3L2ADDRESS + 1)
#define p3L2ANON_TOK		"anon"
#define p3L2ANONCMD			(p3L2ANON + 1)
#define p3L2ANONCMD_TOK		"anoncmd"
#define p3L2ANONIPVER		(p3L2ANONCMD + 1)
#define p3L2ANONIPVER_TOK	"anonipver"
#define p3L2ANONNET			(p3L2ANONIPVER + 1)
#define p3L2ANONNET_TOK		"anonnet"
#define p3L2ANONPORT		(p3L2ANONNET + 1)
#define p3L2ANONPORT_TOK	"anonport"
#define p3L2ANONSET			(p3L2ANONPORT + 1)
#define p3L2ANONSET_TOK		"anonset"
#define p3L2ARRAY			(p3L2ANONSET + 1)
#define p3L2ARRAY_TOK		"array"
#define p3L2CATEGORY		(p3L2ARRAY + 1)
#define p3L2CATEGORY_TOK	"category"
#define p3L2ENCRYPTION		(p3L2CATEGORY + 1)
#define p3L2ENCRYPTION_TOK	"encryption"
#define p3L2FIFO			(p3L2ENCRYPTION + 1)
#define p3L2FIFO_TOK		"fifo"
#define p3L2GENKEY			(p3L2FIFO + 1)
#define p3L2GENKEY_TOK		"genkey"
#define p3L2GET				(p3L2GENKEY + 1)
#define p3L2GET_TOK			"get"
#define p3L2GROUP			(p3L2GET + 1)
#define p3L2GROUP_TOK		"group"
#define p3L2HEARTBEAT		(p3L2GROUP + 1)
#define p3L2HEARTBEAT_TOK	"heartbeat"
#define p3L2HOSTNAME		(p3L2HEARTBEAT + 1)
#define p3L2HOSTNAME_TOK	"hostname"
#define p3L2IPVER			(p3L2HOSTNAME + 1)
#define p3L2IPVER_TOK		"ipver"
#define p3L2KEY				(p3L2IPVER + 1)
#define p3L2KEY_TOK			"key"
#define p3L2NAME			(p3L2KEY + 1)
#define p3L2NAME_TOK		"name"
#define p3L2PORT			(p3L2NAME + 1)
#define p3L2PORT_TOK		"port"
#define p3L2REKEY			(p3L2PORT + 1)
#define p3L2REKEY_TOK		"rekey"
#define p3L2SHUTDOWN		(p3L2REKEY + 1)
#define p3L2SHUTDOWN_TOK	"shutdown"
#define p3L2STATUS			(p3L2SHUTDOWN + 1)
#define p3L2STATUS_TOK		"status"
#define p3L2SUBNET			(p3L2STATUS + 1)
#define p3L2SUBNET_TOK		"subnet"
#define p3L2SYSCFG			(p3L2SUBNET + 1)
#define p3L2SYSCFG_TOK		"syscfg"
#define p3L2UPDATE			(p3L2SYSCFG + 1)
#define p3L2UPDATE_TOK		"update"
#define p3L2USER			(p3L2UPDATE + 1)
#define p3L2USER_TOK		"user"
#define p3L2MAX				(p3L2USER + 1)
#if p3L2MAX > 127
#error Too many tokens in Level 2
#endif

// NOTE: Insert new tokens in alphabetical order
#define p3L3ACCEPT			0
#define p3L3ACCEPT_TOK		"accept"
#define p3L3ADDRESS			(p3L3ACCEPT + 1)
#define p3L3ADDRESS_TOK		"address"
#define p3L3BUILDARRAY		(p3L3ADDRESS + 1)
#define p3L3BUILDARRAY_TOK	"build_array"
#define p3L3CONNECT			(p3L3BUILDARRAY + 1)
#define p3L3CONNECT_TOK		"connect"
#define p3L3CONTROL			(p3L3CONNECT + 1)
#define p3L3CONTROL_TOK		"control"
#define p3L3DATA			(p3L3CONTROL + 1)
#define p3L3DATA_TOK		"data"
#define p3L3DIRECTORY		(p3L3DATA + 1)
#define p3L3DIRECTORY_TOK	"directory"
#define p3L3FAIL			(p3L3DIRECTORY + 1)
#define p3L3FAIL_TOK		"fail"
#define p3L3FLUSHARRAY		(p3L3FAIL + 1)
#define p3L3FLUSHARRAY_TOK	"flush_array"
#define p3L3IFCONFIG		(p3L3FLUSHARRAY + 1)
#define p3L3IFCONFIG_TOK	"ifconfig"
#define p3L3IPVER			(p3L3IFCONFIG + 1)
#define p3L3IPVER_TOK		"ipver"
#define p3L3LENGTH			(p3L3IPVER + 1)
#define p3L3LENGTH_TOK		"length"
#define p3L3LEVEL			(p3L3LENGTH + 1)
#define p3L3LEVEL_TOK		"level"
#define p3L3LOCALARRAY		(p3L3LEVEL + 1)
#define p3L3LOCALARRAY_TOK	"local_array"
#define p3L3LOGS			(p3L3LOCALARRAY + 1)
#define p3L3LOGS_TOK		"logs"
#define p3L3MEMORY			(p3L3LOGS + 1)
#define p3L3MEMORY_TOK		"memory"
#define p3L3NAME			(p3L3MEMORY + 1)
#define p3L3NAME_TOK		"name"
#define p3L3NEW				(p3L3NAME + 1)
#define p3L3NEW_TOK			"new"
#define p3L3PACKETS			(p3L3NEW + 1)
#define p3L3PACKETS_TOK		"packets"
#define p3L3PASSWORD		(p3L3PACKETS + 1)
#define p3L3PASSWORD_TOK	"password"
#define p3L3PERFORM			(p3L3PASSWORD + 1)
#define p3L3PERFORM_TOK		"perform"
#define p3L3PORT			(p3L3PERFORM + 1)
#define p3L3PORT_TOK		"port"
#define p3L3REPORT			(p3L3PORT + 1)
#define p3L3REPORT_TOK		"report"
#define p3L3ROUTE			(p3L3REPORT + 1)
#define p3L3ROUTE_TOK		"route"
#define p3L3S2UNAME			(p3L3ROUTE + 1)
#define p3L3S2UNAME_TOK		"s2uname"
#define p3L3SIZE			(p3L3S2UNAME + 1)
#define p3L3SIZE_TOK		"size"
#define p3L3START			(p3L3SIZE + 1)
#define p3L3START_TOK		"start"
#define p3L3STATUS			(p3L3START + 1)
#define p3L3STATUS_TOK		"status"
#define p3L3SUBNET			(p3L3STATUS + 1)
#define p3L3SUBNET_TOK		"subnet"
#define p3L3TIME			(p3L3SUBNET + 1)
#define p3L3TIME_TOK		"time"
#define p3L3U2SNAME			(p3L3TIME + 1)
#define p3L3U2SNAME_TOK		"u2sname"
#define p3L3UID				(p3L3U2SNAME + 1)
#define p3L3UID_TOK			"uid"
#define p3L3USE				(p3L3UID + 1)
#define p3L3USE_TOK			"use"
#define p3L3MAX				(p3L3USE + 1)
#if p3L3MAX > 127
#error Too many tokens in Level 3
#endif

// NOTE: Insert new tokens in alphabetical order
#define p3L4ADDRESS			0
#define p3L4ADDRESS_TOK		"address"
#define p3L4IPVER			(p3L4ADDRESS + 1)
#define p3L4IPVER_TOK		"ipver"
#define p3L4PACKETS			(p3L4IPVER + 1)
#define p3L4PACKETS_TOK		"packets"
#define p3L4TIME			(p3L4PACKETS + 1)
#define p3L4TIME_TOK		"time"
#define p3L4MAX				(p3L4TIME + 1)
#if p3L4MAX > 127
#error Too many tokens in Level 4
#endif

/*****  MACROS  *****/
/**
 * Macro:
 *   set_token
 *
 * Description:
 *   Set a token in a token list array.  Note that the last token
 *   in the list should be set using strcpy because setting the
 *   next field would overwrite the array.
 *
 * Parameters:
 *   - list: The token list array
 *   - index:  The index of the token
 *   - token:  The value of the token
 */

#define set_token(list, index, token) \
	do { \
		strcpy(list[index], token); \
		list[index + 1] = list[index] + sizeof(token); \
	} while (0)

/**
 * Macro:
 *   p3malloc
 *
 * Description:
 *   Allocate buffer space
 *
 * Parameters:
 *   - size:  The size, in bytes, to be allocated
 */

#define p3malloc(size) \
	malloc(size)

/**
 * Macro:
 *   p3calloc
 *
 * Description:
 *   Allocate buffer space
 *
 * Parameters:
 *   - size:  The size, in bytes, to be allocated
 */

#define p3calloc(size) \
	calloc(1, size)

/**
 * Macro:
 *   p3signal
 *
 * Description:
 *   Set interrupt handler for a specific signal.
 *
 * Parameters:
 *   - sgnl: Signal
 *   - fnctn: Interrupt handler function
 *   - sgact: Sigaction structure
 *   - st: Status variable
 */

#define p3signal(sgnl, fnctn, sgact, st) \
	do { \
		sgact.sa_handler = fnctn; \
		sigemptyset(&sgact.sa_mask); \
		sgact.sa_flags = SA_RESETHAND; \
		st = sigaction(sgnl, &sgact, NULL); \
	} while (0)

/*****  DATA DEFINITIONS  *****/

typedef struct _p3comm p3comm;
typedef struct _p3stringlist p3stringlist;

/**
 * Structure:
 * p3comm
 *
 * \par Description:
 * The administration communication structure maintains information
 * exchanged between the administration library and the components
 * performing administration tasks.
 *
 * <i>Note that the datlen value is the length of the data buffer,
 * including the NULL termination character.</i>
 */

struct _p3comm {
	char	tokens[p3MAX_TOKENS];	/**< List of tokens in request/response */
	int		ID;						/**< Command ID for tracking */
	int		id1;					/**< ID depends on token */
	int		id2;					/**< ID depends on token */
	int		datlen;					/**< Length of data string, including NULL */
	char	*data;					/**< Data string */
};

/**
 * Structure:
 * p3stringlist
 *
 * \par Description:
 * A simple list of strings for tempoarily storing data of unknown length
 * being accumulated in portions.
 */

struct _p3stringlist {
	p3stringlist	*next;	/**< List pointer */
	int				len;	/**< Length of data string */
	char			*data;	/**< String data */
};

#ifdef _p3_PRIMARY
typedef struct _p3share p3share;

/**
 * Structure:
 * p3share
 *
 * \par Description:
 * The anchor for a block of shared memory.
 */

struct _p3share {
	p3share				*next;		/**< Shared block list pointer */
	p3share				*tail;		/**< End of list pointer */
	int					shmid;		/**< Shared memory ID of next block */
	int					used;		/**< The amount of space used in this block */
	unsigned int		libflag;	/**< Flags for library */
	unsigned int		extflag;	/**< Flags for external process */
#define p3SHR_NEXTAT	0x00000001	/* Next shared block attached */
};
#endif

#endif /* _p3_BASE_H */
