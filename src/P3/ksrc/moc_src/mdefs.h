/* Version: mss_v5_3_1_13561 */
/*
 * mdefs.h
 *
 * Mocana Definitions
 *
 * Copyright Mocana Corp 2003-2007. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */


/*------------------------------------------------------------------*/

#ifndef __MDEFS_HEADER__
#define __MDEFS_HEADER__

#ifndef NULL
#define NULL                            (0)
#endif

#ifndef MOC_UNUSED
#define MOC_UNUSED(X)
#endif

#define CR                              '\x0d'
#define LF                              '\x0a'
#define CRLF                            "\x0d\x0a"
#define TAB                             '\x09'
#define QMARK                           '\x3f'
#define SP                              '\x20'

#ifndef TRUE
#define TRUE                            (1)
#endif

#ifndef FALSE
#define FALSE                           (0)
#endif

#ifndef COUNTOF
#define COUNTOF(a)                      (sizeof(a)/sizeof(a[0]))
#endif

#ifndef OFFSETOF
#ifdef  offsetof
#define OFFSETOF(s,m)    offsetof(s,m)
#else
#define OFFSETOF(s,m)   (ubyte4)&(((s *)0)->m)
#endif
#endif

#ifndef MOCANA_UPPER_PRIVILEGE_PORT
#define MOCANA_UPPER_PRIVILEGE_PORT     (1024)
#endif

#endif /* __MDEFS_HEADER__ */
