/**
 * \file p3pri_key_server.h
 * <h3>Protected Point to Point key server header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3_PRI_KEY_SERV_H
#define _p3_PRI_KEY_SERV_H

/*****  INCLUDE FILES *****/

#include "p3crypto.h"

/*****  CONSTANTS  *****/

/*****  DATA DEFINITIONS  *****/

typedef struct _p3key_serv p3key_serv;

/**
 * Structure:
 * p3key_serv
 * 
 * \par Description:
 * The key server structure to maintain information about encyrption keys.
 */

struct _p3key_serv {
	int				head;		/**< Head pointer index for the circular buffer */
	int				tail;		/**< Tail pointer index for the circular buffer */
	int				cbuf_sz;	/**< Number of slots in the circular buffer */
};

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

int init_key_serv(p3key_serv *kserv);
int buffer_handler();

/*****  EXTERNAL DEFINITIONS  *****/

#endif /* _p3_PRI_KEY_SERV_H */
