/**
 * \file s3base.h
 * <h3>Secure Storage Solution base header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _s3_BASE_H
#define _s3_BASE_H

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <features.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

/*****  CONSTANTS  *****/

#define s3VERSION		"Version 0.1.0"

#ifdef _s3_ENCRYPT_ENGINE_C
#define s3APPLICATION	"Velocite Systems S3 Encryption Engine"
#else
#ifdef _s3_AGENT_C
#define s3APPLICATION	"Velocite Systems S3 Agent"
#else
#define s3APPLICATION	"Velocite Systems S3 Key Server"
#endif

#define s3START			1			/* Start a feature */
#define s3STOP			2			/* Stop a feature */

#define s3MSG_CRIT		1			/**< Critical caused by system error */
#define s3MSG_ERR		2			/**< Error caused by application */
#define s3MSG_WARN		3			/**< Warning allows admin correction */
#define s3MSG_NOTICE	4			/**< Notices always displayed */
#define s3MSG_INFO		5			/**< Information for usage stats */

#if PATH_MAX < 4096
#define s3BUFSIZE		4096			/* Typical buffer size */
#else
#define s3BUFSIZE		PATH_MAX		/* Typical buffer size */
#endif

#define s3SHARE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

#if INT_MAX == 0x7fffffff
#define s3INT_SHIFT	2				/* 4 byte integer */
#define s3INT_BYTES	4				/* 4 byte integer */
#define s3INT_BITS		32				/* 32 bit integer */
#define s3INT_BSHIFT	5				/* 32 bit integer */
#elif INT_MAX == 0x7fff
#define s3INT_SHIFT	1				/* 2 byte integer */
#define s3INT_BYTES	2				/* 2 byte integer */
#define s3INT_BITS		16				/* 16 bit integer */
#define s3INT_BSHIFT	4				/* 16 bit integer */
#else
#define s3INT_SHIFT	3				/* 8 byte integer */
#define s3INT_BYTES	8				/* 8 byte integer */
#define s3INT_BITS		64				/* 64 bit integer */
#define s3INT_BSHIFT	6				/* 64 bit integer */
#endif

#if LONG_MAX == 0x7fffffff
#define s3LONG_SHIFT	2				/* 4 byte long integer */
#define s3LONG_BITS	32				/* 32 bit long integer */
#define s3LONG_BSHIFT	5				/* 32 bit long integer */
#else
#define s3LONG_SHIFT	3				/* 8 byte long integer */
#define s3LONG_BITS	64				/* 64 bit long integer */
#define s3LONG_BSHIFT	6				/* 64 bit long integer */
#endif

#define s3INT_MASK		~(sizeof(int) - 1)	/* Mask for integer boundary */
#define s3LONG_MASK		~(sizeof(long) - 1)	/* Mask for long integer boundary */
#define s3ADR_MASK		~(sizeof(char *) - 1)	/* Mask for pointer boundary */

#define s3_time_t		unsigned int

/**
 * Debugging:
 *   s3DBG_type
 *
 * Description:
 *   Fine tune debugging messages by setting the single constant
 *   - s3DEBUG
 */

#ifndef s3DEBUG
#define s3DEBUG			0x00000000
#endif

#define s3DEBUG_ERR		0x00000001		/* Logic errors */
#define s3DEBUG_WARN	0x00000002		/* Unexpected behavior */
#define s3DEBUG_END		0x00000004		/* Shutdown messages */
#define s3DEBUG_PROG	0x00000008		/* Trace progress */
#define s3DEBUG_STRU	0x00000010		/* Structure values */
#define s3DEBUG_DATA	0x00000020		/* Data values */
#define s3DEBUG_MEM		0x00000040		/* Memory */
#define s3DEBUG_PRGS	0x00000080		/* Progress */
#define s3DEBUG_PERF	0x00000100		/* Performance */

#if s3DEBUG_ERR == (s3DEBUG & s3DEBUG_ERR)
#define s3DBG_ERR		1
#endif
#if s3DEBUG_WARN == (s3DEBUG & s3DEBUG_WARN)
#define s3DBG_WARN		1
#endif
#if s3DEBUG_PRGS == (s3DEBUG & s3DEBUG_PRGS)
#define s3DBG_PRGS		1
#endif
#if s3DEBUG_MEM == (s3DEBUG & s3DEBUG_MEM)
#define s3DBG_MEM		1
#endif
#if s3DEBUG_DATA == (s3DEBUG & s3DEBUG_DATA)
#define s3DBG_DATA		1
#endif
#if s3DEBUG_STRU == (s3DEBUG & s3DEBUG_STRU)
#define s3DBG_STRU		1
#endif
#if s3DEBUG_PERF == (s3DEBUG & s3DEBUG_PERF)
#define s3DBG_PERF		1
#endif
#if s3DEBUG_END == (s3DEBUG & s3DEBUG_END)
#define s3DBG_END		1
#endif
#if s3DEBUG_PROG == (s3DEBUG & s3DEBUG_PROG)
#define s3DBG_PROG	1
#endif

/*****  MACROS  *****/

/**** TIME FORMATTING ****/

#define s3MTIME_SZ 32	/* Message time string size */

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
  strftime(time_buf, s3MTIME_SZ, "%b %e, %Y %H:%M:%S", tm_stru);

/**** WAIT MANAGEMENT ****/

#define s3MILLISEC(msec)	(msec * 1000)	/* Milliseconds in timeval struct */
#define s3MICROSEC(msec)	msec			/* Microseconds in timeval struct */

/**** QUEUING ****/

/**
 * Macro:
 *   s3ENQ_1ST
 *
 * Description:
 *   Enqueue an element at the head of a queue
 *
 * Parameters:
 *   - head: Pointer to head of queue
 *   - ele: Element to be queued
 *   - nxt: Name of forward pointer field
 *
 * Dependency:
 *   - s3MQUEUE: Constant definition for message queue anchor
 *   - msg_tbuf: Work buffer of size 1024
 */

#define s3ENQ_1ST(head, ele, nxt) \
    if (head != ele) \
    { \
      ele->nxt = head; \
      head = ele; \
    }

/**
 * Macro:
 *   s3ENQ_1ST_DBL
 *
 * Description:
 *   Enqueue an element at the head of a bidirectional queue
 *
 * Parameters:
 *   - head: Pointer to head of queue
 *   - ele: Element to be queued
 *   - nxt: Name of forward pointer field
 *   - prv: Name of backward pointer field
 *
 * Dependency:
 *   - s3MQUEUE: Constant definition for message queue anchor
 *   - msg_tbuf: Work buffer of size 1024
 */

#define s3ENQ_1ST_DBL(head, ele, nxt, prv) \
    if (head != ele) \
    { \
    	if ((ele->nxt = head) != NULL) \
	      ele->nxt->prv = ele; \
      head = ele; \
    }

/**
 * Macro:
 *   s3ENQ_LAST
 *
 * Description:
 *   Enqueue an element at the end of a queue
 *
 * Parameters:
 *   - head: Pointer to head of queue
 *   - ele: Element to be queued
 *   - tmpel: Temporary work element
 *   - nxt: Name of forward pointer field
 *
 * Dependency:
 *   - s3MQUEUE: Constant definition for message queue anchor
 *   - msg_tbuf: Work buffer of size 1024
 */

#define s3ENQ_LAST(head, ele, tmpel, nxt) \
  ele->nxt = NULL; \
    if (head == NULL) \
      head = ele; \
    else \
    { \
      tmpel = head; \
        while (tmpel->nxt != NULL && tmpel->nxt != ele) \
          tmpel = tmpel->nxt; \
        if (tmpel->nxt == NULL) \
          tmpel->nxt = ele; \
    }

/**
 * Macro:
 *   s3DEQ_1ST
 *
 * Description:
 *   Dequeue an element from the head of a queue
 *
 * Parameters:
 *   - head: Pointer to head of queue
 *   - ele: Element to be dequeued
 *   - nxt: Name of forward pointer field
 *
 * Dependency:
 *   - s3MQUEUE: Constant definition for message queue anchor
 *   - msg_tbuf: Work buffer of size 1024
 */

#define s3DEQ_1ST(head, ele, nxt) \
  ele = head; \
    if (ele != NULL) \
    { \
      head = ele->nxt; \
      ele->nxt = NULL; \
    }

/**
 * Macro:
 *   s3DEQ_1ST_DBL
 *
 * Description:
 *   Dequeue a doubly linked element from the head of a queue
 *
 * Parameters:
 *   - head: Pointer to head of queue
 *   - ele: Element to be dequeued
 *   - nxt: Name of forward pointer field
 *   - prv: Name of backward pointer field
 *
 * Dependency:
 *   - s3MQUEUE: Constant definition for message queue anchor
 *   - msg_tbuf: Work buffer of size 1024
 */

#define s3DEQ_1ST_DBL(head, ele, nxt, prv) \
  ele = head; \
    if (ele != NULL) \
    { \
      head = ele->nxt; \
    	if (ele->nxt != NULL) \
	      ele->nxt->prv = NULL; \
      ele->nxt = NULL; \
      ele->prv = NULL; \
    }

/**
 * Macro:
 *   s3DEQ_LAST
 *
 * Description:
 *   Dequeue an element from the end of a queue
 *
 * Parameters:
 *   - head: Pointer to head of queue
 *   - ele: Element to be dequeued
 *   - tmpel: Temporary work element
 *   - nxt: Name of forward pointer field
 *
 * Dependency:
 *   - s3MQUEUE: Constant definition for message queue anchor
 *   - msg_tbuf: Work buffer of size 1024
 */

#define s3DEQ_LAST(head, ele, tmpel, nxt) \
    if (head == NULL) \
      ele = NULL; \
    else \
    { \
      ele = head; \
        if (ele->nxt == NULL) \
          head = NULL; \
        else \
        { \
            while (ele->nxt != NULL) \
            { \
              tmpel = ele; \
              ele = ele->nxt; \
            } \
          tmpel->nxt = NULL; \
        } \
    }

/**
 * Macro:
 *   s3DEQ
 *
 * Description:
 *   Dequeue an element from anywhere in a queue
 *   NOTE:  If not found, tmpel returns NULL
 *
 * Parameters:
 *   - head: Pointer to head of queue
 *   - ele: Element to be queued
 *   - tmpel: Temporary work element
 *   - nxt: Name of forward pointer field
 *
 * Dependency:
 *   - s3MQUEUE: Constant definition for message queue anchor
 *   - msg_tbuf: Work buffer of size 1024
 */

#define s3DEQ(head, ele, tmpel, nxt) \
  tmpel = NULL; \
    if (ele != NULL && head == ele) \
    { \
      head = ele->nxt; \
      ele->nxt = NULL; \
      tmpel = ele; \
    } \
    else if (head != NULL) \
    { \
      tmpel = head; \
        while ((tmpel->nxt != NULL) && (tmpel->nxt != ele)) \
          tmpel = tmpel->nxt; \
        if (tmpel->nxt != NULL) \
        { \
          tmpel->nxt = ele->nxt; \
          ele->nxt = NULL; \
        } \
        else \
          tmpel = NULL; \
    }

/**
 * Macro:
 *   s3DEQ_DBL
 *
 * Description:
 *   Dequeue a doubly linked element from anywhere in a queue
 *
 * Parameters:
 *   - head: Pointer to head of queue
 *   - ele: Element to be dequeued
 *   - nxt: Name of forward pointer field
 *   - prv: Name of backward pointer field
 *
 * Dependency:
 *   - s3MQUEUE: Constant definition for message queue anchor
 *   - msg_tbuf: Work buffer of size 1024
 */

#define s3DEQ_DBL(head, ele, nxt, prv) \
    if (ele != NULL) \
    { \
		if (ele->prv != NULL) \
		  ele->prv->nxt = ele->nxt; \
		else if (head == ele) \
	      head = ele->nxt; \
		if (ele->nxt != NULL) \
		  ele->nxt->prv = ele->prv; \
	  ele->nxt = ele->prv = NULL; \
    }

/**
 * Macro:
 *   s3Q_LAST
 *
 * Description:
 *   Point to the last element in a queue
 *
 * Parameters:
 *   - head: Pointer to head of queue
 *   - ele: Element to be queued
 */

#define s3Q_LAST(head, ele, nxt) \
    if (head == NULL) \
      ele = NULL; \
    else \
    { \
      ele = head; \
        while (ele->nxt != NULL) \
          ele = ele->nxt; \
    }

#endif /* _s3_BASE_H */
