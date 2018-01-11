/* Version: mss_v5_3_1_13561 */
/*
 * aes_cmac.h
 *
 * AES-CMAC Implementation
 *
 * Copyright Mocana Corp 2006-2007. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */

/*! \file aes_cmac.h AES developer API header.
This header file contains definitions, enumerations, structures, and function
declarations used for AES encryption and decryption.

\since 3.0.6
\version 5.0.5 and later

! Flags
There are no flag dependencies to enable the functions in this header file.

! External Functions
This file contains the following public ($extern$) function declarations:
- AESCMAC_init
- AESCMAC_update
- AESCMAC_final

*/


/*------------------------------------------------------------------*/

#ifndef __AES_CMAC_HEADER__
#define __AES_CMAC_HEADER__

#ifdef __cplusplus
extern "C" {
#endif

#define CMAC_RESULT_SIZE AES_BLOCK_SIZE

typedef struct AES_OMAC_Ctx
{
    ubyte                   currBlock[AES_BLOCK_SIZE];
    /* bytes received -- we delay the processing until more bytes are
    received or final is called */
    ubyte                   pending[AES_BLOCK_SIZE] ;
    /* length of bytes received above <= AES_BLOCK_SIZE */
    ubyte                   pendingLen;
} AES_OMAC_Ctx;

typedef struct AESCMAC_Ctx
{
    aesCipherContext        aesCtx;
    AES_OMAC_Ctx            omacCtx;
} AESCMAC_Ctx;

/*------------------------------------------------------------------*/

/*  Function prototypes  */
/* AES CMAC -- cf RFC 4493 for explanations of parameters. */

MOC_EXTERN MSTATUS AESCMAC_init(MOC_SYM(hwAccelDescr hwAccelCtx) const ubyte *keyMaterial, sbyte4 keyLength, AESCMAC_Ctx *pCtx);

MOC_EXTERN MSTATUS AESCMAC_update(MOC_SYM(hwAccelDescr hwAccelCtx) const ubyte* data, sbyte4 dataLength, AESCMAC_Ctx* pCtx);

MOC_EXTERN MSTATUS AESCMAC_final(MOC_SYM(hwAccelDescr hwAccelCtx) ubyte cmac[CMAC_RESULT_SIZE], AESCMAC_Ctx* pCtx);

/* reusable functions -- to be used internally by mocana only */
MOC_EXTERN MSTATUS AES_OMAC_init(AES_OMAC_Ctx* pOMACCtx);
MOC_EXTERN MSTATUS AES_OMAC_update(MOC_SYM(hwAccelDescr hwAccelCtx) aesCipherContext* pAESCtx, AES_OMAC_Ctx* pOMACCtx, const ubyte* data, sbyte4 dataLength);
MOC_EXTERN MSTATUS AES_OMAC_final( MOC_SYM(hwAccelDescr hwAccelCtx) aesCipherContext* pAESCtx, AES_OMAC_Ctx* pOMACCtx, ubyte cmac[CMAC_RESULT_SIZE]);

#ifdef __cplusplus
}
#endif

#endif /* __AES_CMAC_HEADER__ */

