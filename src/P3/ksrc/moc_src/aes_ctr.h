/* Version: mss_v5_3_1_13561 */
/*
 * aes_ctr.h
 *
 * AES-CTR Implementation
 *
 * Copyright Mocana Corp 2006-2009. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */

/*! \file aes_ctr.h AES developer API header.
This header file contains definitions, enumerations, structures, and function
declarations used for AES encryption and decryption.

\since 3.0.6
\version 5.0.5 and later

! Flags
There are no flag dependencies to enable the functions in this header file.

! External Functions
This file contains the following public ($extern$) function declarations:
- CreateAESCTRCtx
- DeleteAESCTRCtx
- DoAESCTR

*/


/*------------------------------------------------------------------*/

#ifndef __AES_CTR_HEADER__
#define __AES_CTR_HEADER__

#ifdef __cplusplus
extern "C" {
#endif


/*------------------------------------------------------------------*/

/*  The structure for key information */
typedef struct aesCTRCipherContext
{
    aesCipherContext    ctx;
    union
    {
        ubyte               counterBlock[AES_BLOCK_SIZE];   /* counter block */
        ubyte4              ctr[4];
    } u;
    ubyte               encBlock[AES_BLOCK_SIZE];       /* encrypted counter block */
    ubyte               offset;                         /* offset of unused byte in the encBlock */
} aesCTRCipherContext, AES_CTR_Ctx;



/*------------------------------------------------------------------*/

/*  Function prototypes  */
/* for AES CTR, the keyMaterial is key + block --- iv is NOT used in DoAESCTR */
/* for RFC3686, construct the block and do not pass the IV as the iv argument */
/* the block is incremented by 1 for each encryption so as to be compatible with RFC 3686
and the EAX mode -- */

MOC_EXTERN BulkCtx  CreateAESCTRCtx(MOC_SYM(hwAccelDescr hwAccelCtx) const ubyte* keyMaterial, sbyte4 keyLength, sbyte4 encrypt);

MOC_EXTERN MSTATUS  DeleteAESCTRCtx(MOC_SYM(hwAccelDescr hwAccelCtx) BulkCtx *ctx);

/* data length does not need to be a mulitple of AES_BLOCK_SIZE */
MOC_EXTERN MSTATUS  DoAESCTR(MOC_SYM(hwAccelDescr hwAccelCtx) BulkCtx ctx, ubyte* data, sbyte4 dataLength, sbyte4 encrypt, ubyte* iv);

/* this might be easier to use in some cases */
MOC_EXTERN MSTATUS AESCTRInit(MOC_SYM(hwAccelDescr hwAccelCtx) AES_CTR_Ctx* aesCtrCtx, const ubyte* keyMaterial, sbyte4 keyLength, const ubyte initCounter[AES_BLOCK_SIZE]);

/* data length does not need to be a mulitple of AES_BLOCK_SIZE */
/* limit specifies the last byte to increment -> AES_BLOCK_SIZE -> all bytes will be incremented
0 -> no bytes will be incremented */
MOC_EXTERN MSTATUS  DoAESCTREx(MOC_SYM(hwAccelDescr hwAccelCtx) BulkCtx ctx, ubyte* data, sbyte4 dataLength, sbyte4 encrypt, ubyte* iv, sbyte4 limit);


#ifdef __ENABLE_MOCANA_IPSEC_SERVICE__
MOC_EXTERN BulkCtx  CreateAesCtrCtx(MOC_SYM(hwAccelDescr hwAccelCtx) ubyte* keyMaterial,
                                sbyte4 keyLength, sbyte4 encrypt);
MOC_EXTERN MSTATUS  DoAesCtr(MOC_SYM(hwAccelDescr hwAccelCtx) BulkCtx ctx, ubyte* data,
                         sbyte4 dataLength, sbyte4 encrypt, ubyte* iv);
#endif


#ifdef __cplusplus
}
#endif

#endif /* __AES_CTR_HEADER__ */

