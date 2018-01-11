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

#ifndef _p3_SECONDARY
#include "p3pri_key_server.h"
#endif

/*****  CONSTANTS  *****/

#ifdef _p3_PRIMARY
#define p3APPLICATION	"Velocite Systems P3 Primary"
#else
#ifdef _p3_SECONDARY
#define p3APPLICATION	"Velocite Systems P3 Secondary"
#endif
#endif

#define p3VERSION		"Version 0.1.0"

#define p3START			1			/* Start a feature */
#define p3STOP			2			/* Stop a feature */

#define p3MSG_CRIT		1			/**< Critical caused by system error */
#define p3MSG_ERR		2			/**< Error caused by application */
#define p3MSG_WARN		3			/**< Warning allows admin correction */
#define p3MSG_NOTICE	4			/**< Notices always displayed */
#define p3MSG_INFO		5			/**< Information for usage stats */
#define p3MSG_DEBUG		6			/**< Debugging messages */

#define PATH_SEPARATOR	'/'
#if PATH_MAX < 4096
#define p3BUFSIZE		4096			/* Typical buffer size */
#else
#define p3BUFSIZE		PATH_MAX		/* Typical buffer size */
#endif

#define p3SHARE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

#if INT_MAX == 0x7fffffff
#define p3INT_SHIFT	2				/* 4 byte integer */
#define p3INT_BYTES	4				/* 4 byte integer */
#define p3INT_BITS		32				/* 32 bit integer */
#define p3INT_BSHIFT	5				/* 32 bit integer */
#elif INT_MAX == 0x7fff
#define p3INT_SHIFT	1				/* 2 byte integer */
#define p3INT_BYTES	2				/* 2 byte integer */
#define p3INT_BITS		16				/* 16 bit integer */
#define p3INT_BSHIFT	4				/* 16 bit integer */
#else
#define p3INT_SHIFT	3				/* 8 byte integer */
#define p3INT_BYTES	8				/* 8 byte integer */
#define p3INT_BITS		64				/* 64 bit integer */
#define p3INT_BSHIFT	6				/* 64 bit integer */
#endif

#if LONG_MAX == 0x7fffffff
#define p3LONG_SHIFT	2				/* 4 byte long integer */
#define p3LONG_BITS	32				/* 32 bit long integer */
#define p3LONG_BSHIFT	5				/* 32 bit long integer */
#else
#define p3LONG_SHIFT	3				/* 8 byte long integer */
#define p3LONG_BITS	64				/* 64 bit long integer */
#define p3LONG_BSHIFT	6				/* 64 bit long integer */
#endif

#define p3INT_MASK		~(sizeof(int) - 1)	/* Mask for integer boundary */
#define p3LONG_MASK		~(sizeof(long) - 1)	/* Mask for long integer boundary */
#define p3ADR_MASK		~(sizeof(char *) - 1)	/* Mask for pointer boundary */

#define p3_time_t		unsigned int

/**
 * Debugging:
 *   p3DBG_type
 *
 * Description:
 *   Fine tune debugging messages by setting the single constant
 *   - p3DEBUG
 */

// TODO: Use fine tuned debugging in code
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

/**** TIME FORMATTING ****/

#define p3MTIME_SZ 32	/* Message time string size */

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
  tm_stru = gmtime(time_val); \
  strftime(time_buf, p3MTIME_SZ, "%b %e, %Y %H:%M:%S", tm_stru);

/**** WAIT MANAGEMENT ****/

#define p3MILLISEC(msec)	(msec * 1000)	/* Milliseconds in timeval struct */
#define p3MICROSEC(msec)	msec			/* Microseconds in timeval struct */

/*****  DATA DEFINITIONS  *****/

/*****  EXTERNAL DEFINITIONS  *****/

#endif /* _p3_BASE_H */
