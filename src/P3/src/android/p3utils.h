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
#include "p3system.h"
#include <netinet/ip.h>
#include <netinet/in6.h>

/*****  CONSTANTS  *****/

#define P3DEVNAME "/dev/p3dev"
#define P3IOC_TYPE 'p'
#define P3_IOW(type,nr,size) _IOC(_IOC_WRITE,(type),(nr),size)
#define P3_IOR(type,nr,size) _IOC(_IOC_READ, (type),(nr),size)

  #ifdef _p3_PRIMARY
#define RAMDISK_SZ	0x1000	// !!! Temporary !!!
// #define RAMDISK_SZ	0x10000
  #else
	#ifdef _p3_SECONDARY
#define RAMDISK_SZ	0x1000
	#else
#define RAMDISK_SZ	0x20000
	#endif
  #endif

#define MM_CMD_INIT		"PID"

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
	p3mm_anchor 	*anchor;	/**< Memory map information */
	char			*msgdir;	/**< Directory path of message logs */
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} lochost;					/**< Local host address */
	union {
		struct in_addr  v4;
		struct in6_addr v6;
	} remhost;					/**< Remote host address for raw socket */
	unsigned int	flag;
#define p3UTL_NINFO	0x00000001	/* Ignore informational messages */
#define p3UTL_NWARN	0x00000002	/* Ignore warning messages */
};

/*****  MACROS  *****/

/**
 * Macro:
 * MM_READ_ERR
 *
 * \par Description:
 * If the mmap anchor has not been allocated or there is an error
 * reading from the RAM disk, then the caller must handle the error.
 * Otherwise, the amount a data read is returned.
 *
 * \par Parameters:
 * - buffer: An allocated buffer space.
 * - size: The amount of data to read, which must be less than or equal to
 *   the size of the buffer space.
 * - count: The amount of data successfully read.
 */
#define MM_READ_ERR(buffer, size, count) \
	if (p3utils->anchor == NULL || (count = read (p3utils->anchor->fd, buffer, size)) < 0)

/**
 * Macro:
 * MM_WRITE_ERR
 *
 * \par Description:
 * If the mmap anchor has not been allocated or there is an error
 * writing to the RAM disk, then the caller must handle the error.
 *
 * \par Parameters:
 * - buffer: The buffer of data to write.
 * - size: The amount of data to write, which must be less than or equal to
 *   the size of the buffer space.
 */
#define MM_WRITE_ERR(buffer, size) \
	if (p3utils->anchor == NULL || write(p3utils->anchor->fd, buffer, size) < 0)

/*****  PROTOTYPES  *****/

int init_utils(char *msgdir);
int init_kernel_comm();
int init_raw_socket();
int send_ioctl(unsigned char *buffer, int size);
int receive_ioctl(unsigned char *buffer, int size);
void p3errmsg(int type, char *message);
int isallnum(char* parm);

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _p3_UTILS_C
extern p3util *p3utils;
#endif

#endif /* _p3_UTILS_H */
