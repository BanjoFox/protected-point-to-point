/**
 * \file p3anonymous.h
 * <h3>Protected Point to Point product anonymous header file</h3>
 *
 * Copyright (C) Velocite 2010
 */

#ifndef _p3_ANON_H
#define _p3_ANON_H

#include "p3utils.h"
#include "p3config.h"
#include "p3command.h"
#include "p3net.h"
#include <sys/ipc.h>
#include <sys/shm.h>

/*****  CONSTANTS  *****/

#define p3ANON_FNAME	"p3anon.temp"	/* Temporary storage of anonymous data */

#define p3SHARE_SIZE	0x10000		/* Size of shared blocks (64K) */
#define p3SHARE_START	0x3000000	/* Starting location of shared blocks */

#define p3FIFO_READ		(O_RDONLY | O_NONBLOCK)
#define p3FIFO_WRITE	(O_WRONLY | O_NONBLOCK)
#define p3FIFO_MODE		(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define p3SHARE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

/*****  MACROS  *****/

/*****  DATA DEFINITIONS  *****/

/*****  PROTOTYPES  *****/

int init_anonymous();

#ifdef _p3_PRIMARY
int anon_handler();
int get_anon_share(p3share *share, int new);
void *p3shalloc(int size);
int attach_shares();
int save_anonymous();
void anon_shutdown(int status);
#endif

#ifdef _p3_SECONDARY
int anon_connect();
#endif

/*****  EXTERNAL DEFINITIONS  *****/

#endif /* _p3_ANON_H */
