/**
 * \file s3session.h
 * <h3>Secure Storage Solution session header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _s3SS_H
#define _s3SS_H

/*****  INCLUDE FILES *****/

#include "s3base.h"

/*****  CONSTANTS  *****/

/*****  DATA DEFINITIONS  *****/

typedef struct _s3session s3session;

/**
 * Structure:
 * s3session
 * 
 * \par Description:
 * Description.
 */

struct _s3session {
	unsigned int	flag;
#define s3PSS_FLG1	0x00000001
#define s3PSS_FLG2	0x00000002
#define s3PSS_FLG3	0x00000004
};

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

int init_session();
int session_manager();

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _s3SS_C
extern s3session *session;
#endif

#endif /* _s3SS_H */
