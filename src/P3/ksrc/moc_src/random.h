/* Version: mss_v5_3_1_13561 */
/*
 * random.h
 *
 * Random Number FIPS-186 Factory
 *
 * Copyright Mocana Corp 2003-2007. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */
/*! \file random.h Random Number Generator (RNG) API header.
This header file contains definitions, enumerations, structures, and function
declarations used by the Random Number Generator Module.

\since 1.41
\version 3.06 and later

! Flags
Whether the following flag is defined determines which functions
are declared:
$__DISABLE_MOCANA_ADD_ENTROPY__$

! External Functions
This file contains the following public ($extern$) function declarations:
- RANDOM_acquireContext
- RANDOM_addEntropyBit
- RANDOM_newFIPS186Context
- RANDOM_numberGenerator
- RANDOM_releaseContext
- RANDOM_rngFun
*/


/*------------------------------------------------------------------*/

#ifndef __RANDOM_HEADER__
#define __RANDOM_HEADER__

#ifdef __cplusplus
extern "C" {
#endif

#define RANDOM_CONTEXT(X)       (X)->pRandomContext


/*------------------------------------------------------------------*/

typedef void            randomContext;
typedef sbyte4          (*RNGFun)(void* rngFunArg, ubyte4 length, ubyte *buffer);
MOC_EXTERN randomContext*  g_pRandomContext;


/*------------------------------------------------------------------*/

MOC_EXTERN MSTATUS RANDOM_acquireContext(randomContext **pp_randomContext);

MOC_EXTERN MSTATUS RANDOM_releaseContext (randomContext **pp_randomContext);

#ifndef __DISABLE_MOCANA_ADD_ENTROPY__
MOC_EXTERN MSTATUS RANDOM_addEntropyBit(randomContext *pRandomContext, ubyte entropyBit);
#endif /*__DISABLE_MOCANA_ADD_ENTROPY__*/

MOC_EXTERN MSTATUS RANDOM_numberGenerator(randomContext *pRandomContext, ubyte *pBuffer, sbyte4 bufSize);

MOC_EXTERN MSTATUS RANDOM_KSrcGenerator(randomContext *pRandomContext, ubyte buffer[40]);

MOC_EXTERN sbyte4 RANDOM_rngFun(void* rngFunArg, ubyte4 length, ubyte *buffer);

MOC_EXTERN MSTATUS RANDOM_newFIPS186Context( randomContext **ppRandomContext, ubyte b, const ubyte pXKey[/*b*/], sbyte4 seedLen, const ubyte pXSeed[/*seedLen*/]);

#ifdef __cplusplus
}
#endif

#endif /* __RANDOM_HEADER__ */
