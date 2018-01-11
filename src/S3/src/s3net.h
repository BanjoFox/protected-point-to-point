/**
 * \file s3net.h
 * <h3>Secure Storage Solution network header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _s3NET_H
#define _s3NET_H

/*****  INCLUDE FILES *****/

#include "s3base.h"

/*****  CONSTANTS  *****/

/*****  DATA DEFINITIONS  *****/

typedef struct _s3net s3net;

/**
 * Structure:
 * s3net
 * 
 * \par Description:
 * Description.
 */

struct _s3net {
	unsigned int	flag;
#define s3PNET_FLG1	0x00000001
};

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

int init_net();
int s3_listen();

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _s3NET_C
extern s3net *net;
#endif

#endif /* _s3NET_H */
