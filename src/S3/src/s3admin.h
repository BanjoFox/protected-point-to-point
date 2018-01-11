/**
 * \file s3admin.h
 * <h3>Secure Storage Solution adminstrator interface header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _s3ADMIN_H
#define _s3ADMIN_H

/*****  INCLUDE FILES *****/

#include "s3base.h"

/*****  CONSTANTS  *****/

/*****  DATA DEFINITIONS  *****/

typedef struct _s3admin s3admin;

/**
 * Structure:
 * s3admin
 * 
 * \par Description:
 * Description.
 */

struct _s3admin {
	int				fifo_in;	/**< Read FIFO file descriptor */
	int				fifo_out;	/**< Write FIFO file descriptor */
	unsigned int	flag;
};

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

int init_admin();
int admin_handler();

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _s3ADMIN_C
extern s3admin *admin;
extern char *admbuf;
#endif

#endif /* _s3ADMIN_H */
