/* Version: mss_v5_3_1_13561 */
/*
 * aes_ccm.h
 *
 * AES-CCM Implementation
 *
 * Copyright Mocana Corp 2006-2007. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */

/*! \file aes_ccm.h AES-CCM developer API header.
This header file contains definitions, enumerations, structures, and function
declarations used for AES-CCM operations.

\since 3.0.6
\version 5.0.5 and later

! Flags
No flag definitions are required to use this file.

! External Functions
This file contains the following public ($extern$) function declarations:
- AESCCM_encrypt
- AESCCM_decrypt
*/


/*------------------------------------------------------------------*/

#ifndef __AES_CCM_HEADER__
#define __AES_CCM_HEADER__

#ifdef __cplusplus
extern "C" {
#endif


/*------------------------------------------------------------------*/

/*  Function prototypes  */
/* AES Counter with CBC-MAC (CCM) -- cf RFC 3610 for explanations of parameters. encryption is in place */
/* the U buffer must be M bytes long */

/* AES Counter with CBC-MAC (CCM) encrypts and protects a data buffer. */
MOC_EXTERN MSTATUS  AESCCM_encrypt(MOC_SYM(hwAccelDescr hwAccelCtx) ubyte M, ubyte L, ubyte* keyMaterial, sbyte4 keyLength,
                                    const ubyte* nonce, ubyte* encData, ubyte4 eDataLength,
                                    const ubyte* authData, ubyte4 aDataLength, ubyte U[/*M*/]);

/* AES Counter with CBC-MAC (CCM) decrypt and authenticates a data buffer. */
MOC_EXTERN MSTATUS  AESCCM_decrypt(MOC_SYM(hwAccelDescr hwAccelCtx) ubyte M, ubyte L, ubyte* keyMaterial, sbyte4 keyLength,
                                    const ubyte* nonce, ubyte* encData, ubyte4 eDataLength,
                                    const ubyte* authData, ubyte4 aDataLength, const ubyte U[/*M*/]);

#ifdef __cplusplus
}
#endif

#endif /* __AES_CCM_HEADER__ */

