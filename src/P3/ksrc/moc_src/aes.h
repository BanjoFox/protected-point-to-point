/* Version: mss_v5_3_1_13561 */
/*
 * aes.h
 *
 * AES Implementation
 *
 * Copyright Mocana Corp 2003-2007. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */

/*! \file aes.h AES developer API header.
This header file contains definitions, enumerations, structures, and function
declarations used for AES encryption and decryption.

\since 1.41
\version 3.06 and later

! Flags
No flag definitions are required to use this file.

! External Functions
This file contains the following public ($extern$) function declarations:
- CreateAESCtx
- DeleteAESCtx
- DoAES

*/


/*------------------------------------------------------------------*/

#ifndef __AES_HEADER__
#define __AES_HEADER__

#ifdef __cplusplus
extern "C" {
#endif

#define AES_MAXNR               14
#define AES_BLOCK_SIZE          (16)


/*------------------------------------------------------------------*/

/*  The structure for key information */
typedef struct
{
    sbyte4              encrypt;                        /* Key used for encrypting or decrypting? */
    sbyte4              mode;                           /* MODE_ECB, MODE_CBC, or MODE_CFB1 */
    sbyte4              keyLen;                         /* Length of the key  */
    sbyte4              Nr;                             /* key-length-dependent number of rounds */
    ubyte4              rk[4*(AES_MAXNR + 1)];          /* key schedule */
    ubyte4              ek[4*(AES_MAXNR + 1)];          /* CFB1 key schedule (encryption only) */

} aesCipherContext;


/*------------------------------------------------------------------*/

/* internal prototypes */
MOC_EXTERN MSTATUS AESALGO_makeAesKey(aesCipherContext *pAesContext, sbyte4 keyLen, const ubyte *keyMaterial, sbyte4 encrypt, sbyte4 mode);
MOC_EXTERN MSTATUS AESALGO_blockEncrypt(aesCipherContext *pAesContext, ubyte* iv, ubyte *input, sbyte4 inputLen, ubyte *outBuffer, sbyte4 *pRetLength);
MOC_EXTERN MSTATUS AESALGO_blockDecrypt(aesCipherContext *pAesContext, ubyte* iv, ubyte *input, sbyte4 inputLen, ubyte *outBuffer, sbyte4 *pRetLength);


/*------------------------------------------------------------------*/

/*  Function prototypes  */

MOC_EXTERN BulkCtx  CreateAESCtx(MOC_SYM(hwAccelDescr hwAccelCtx) ubyte* keyMaterial, sbyte4 keyLength, sbyte4 encrypt);

MOC_EXTERN MSTATUS  DeleteAESCtx(MOC_SYM(hwAccelDescr hwAccelCtx) BulkCtx ctx);

MOC_EXTERN MSTATUS  DoAES       (MOC_SYM(hwAccelDescr hwAccelCtx) BulkCtx ctx, ubyte* data, sbyte4 dataLength, sbyte4 encrypt, ubyte* iv);

#ifdef __cplusplus
}
#endif

#endif /* __AES_HEADER__ */

