/**
 * \file s3utils.h
 * <h3>Secure Storage Solution utilities header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _s3_UTILS_H
#define _s3_UTILS_H

/*****  INCLUDE FILES *****/

#include "s3base.h"

/*****  CONSTANTS  *****/

/*****  DATA DEFINITIONS  *****/

typedef struct _s3util s3util;

/**
 * Structure:
 * s3util
 * 
 * \par Description:
 * Description.
 */

struct _s3util {
	unsigned int	flag;
#define s3UTL_NINFO	0x00000001	/**< Ignore informational messages */
};

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

int init_utils();
void s3errmsg(int type, char *message);

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _s3_UTILS_C
extern s3util *s3utils;
#endif

#endif /* _s3_UTILS_H */
