/**
 * \file p3kbase.h
 * <h3>Protected Point to Point product base header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3k_BASE_H
#define _p3k_BASE_H

#include "p3linux.h"

/*****  CONSTANTS  *****/

#ifdef _p3_PRIMARY_C
#define p3APPLICATION	"Velocite Systems P3 Primary"
#else
#ifdef _p3_SECONDARY_C
#define p3APPLICATION	"Velocite Systems P3 Secondary"
#endif
#endif

#define p3VERSION		"Version 0.1.0"

#define p3START			1		/* Start a feature */
#define p3STOP			2		/* Stop a feature */

#define p3MSG_CRIT		1		/**< Critical caused by system error */
#define p3MSG_ERR		2		/**< Error caused by application */
#define p3MSG_WARN		3		/**< Warning allows admin correction */
#define p3MSG_NOTICE	4		/**< Notices always displayed */
#define p3MSG_INFO		5		/**< Information for usage stats */
#define p3MSG_DEBUG		6		/**< Debugging messages */

#define PATH_SEPARATOR	"/"
#if PATH_MAX < 4096
#define p3BUFSIZE		4096			/* Typical buffer size */
#else
#define p3BUFSIZE		PATH_MAX		/* Typical buffer size */
#endif

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

/*****  MACROS  *****/

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
	kmalloc(size, GFP_ATOMIC)

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
	kzalloc(size, GFP_ATOMIC)

/**
 * Macro:
 *   p3free
 *
 * Description:
 *   Free buffer space
 *
 * Parameters:
 *   - buffer: Pointer to buffer to be freed
 */

#define p3free(buffer) \
	kfree(buffer)

/**** WAIT MANAGEMENT ****/

#define p3MILLISEC(msec)	(msec * 1000)	/* Milliseconds in timeval struct */
#define p3MICROSEC(msec)	msec			/* Microseconds in timeval struct */

/*****  DATA DEFINITIONS  *****/

typedef struct _p3cluster p3cluster;
typedef struct _p3chost p3chost;

/**
 * Structure:
 * p3cluster
 *
 * \par Description:
 * The clustering and failover definitions.
 */

struct _p3cluster {
	p3chost			*chost;		/**< Cluster host list */
	unsigned int	flag;
#define p3CLS_RR	0x00000001	/* Use round robin load balancing */
#define p3CLS_IREC	0x00000002	/* Immediate recovery */
};

/**
 * Structure:
 * p3chost
 *
 * \par Description:
 * The clustering host definitions.
 */

struct _p3chost {
	p3chost			*next;		/**< List pointer */
	unsigned int	flag;
#define p3CHS_PRI	0x00000001	/* Host is primary in cluster */
};

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _p3_PRIMARY_C
  #ifndef _p3_SECONDARY_C
    #ifndef _p3_PRIMARYPLUS_C
extern char *p3buf;
    #endif
  #endif
#endif

#endif /* _p3k_BASE_H */

