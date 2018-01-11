/* Version: mss_v5_3_1_13561 */
/*
 * aes_ecb.h
 *
 * AES Implementation
 *
 * Copyright Mocana Corp 2009. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */

/*! \file aes_ecb.h AES developer API header.
This header file contains definitions, enumerations, structures, and function
declarations used for AES encryption and decryption.

\since 5.0.5
\version 5.0.5 and later

! Flags
There are no flag dependencies to enable the functions in this header file.

! External Functions
This file contains the following public ($extern$) function declarations:
- CreateAESECBCtx
- DeleteAESECBCtx
- DoAESECB

*/


/*------------------------------------------------------------------*/

#ifndef __AES_ECB_HEADER__
#define __AES_ECB_HEADER__

#ifdef __cplusplus
extern "C" {
#endif


/*------------------------------------------------------------------*/

/*  Function prototypes  */

MOC_EXTERN BulkCtx CreateAESECBCtx(MOC_SYM(hwAccelDescr hwAccelCtx) ubyte* keyMaterial, sbyte4 keyLength, sbyte4 encrypt);

MOC_EXTERN MSTATUS DeleteAESECBCtx(MOC_SYM(hwAccelDescr hwAccelCtx) BulkCtx *ctx);

MOC_EXTERN MSTATUS DoAESECB(MOC_SYM(hwAccelDescr hwAccelCtx) BulkCtx ctx, ubyte* data, sbyte4 dataLength, sbyte4 encrypt, ubyte* iv);

#ifdef __cplusplus
}
#endif

#endif /* __AES_ECB_HEADER__ */

