/**
 * \file p3kprimaryplus.h
 * <h3>Protected Point to Point product primary plus secondary main functions
 * header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3k_PRIMARYPLUS_H
#define _p3k_PRIMARYPLUS_H

/*****  INCLUDE FILES *****/

#include "p3kbase.h"
#include "p3kprimary.h"
#include "p3ksecondary.h"

/*****  CONSTANTS  *****/

/*****  DATA DEFINITIONS  *****/

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

extern int init_primaryplus (void);
extern int priplus_parse_config(void);

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _p3k_PRIMARYPLUS_C
extern p3pri_main *primain;
extern p3sec_main *secmain;
#endif

#endif /* _p3k_PRIMARYPLUS_H */
