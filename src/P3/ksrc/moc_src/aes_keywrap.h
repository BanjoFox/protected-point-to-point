/* Version: mss_v5_3_1_13561 */
/*
 * aes_keywrap.h
 *
 * AES Key Wrap RFC 3394 Implementation
 *
 * Copyright Mocana Corp 2003-2006. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */


/*------------------------------------------------------------------*/

#ifndef __AESKEYWRAP_HEADER__
#define __AESKEYWRAP_HEADER__

#ifdef __cplusplus
extern "C" {
#endif


MOC_EXTERN MSTATUS
AESKWRAP_encrypt( MOC_SYM(hwAccelDescr hwAccelCtx) ubyte* keyMaterial,sbyte4 keyLength,
                  const ubyte* data, ubyte4 dataLen, ubyte * retData /* Shoudl be dataLen + 8 */);
MOC_EXTERN MSTATUS
AESKWRAP_decrypt(MOC_SYM(hwAccelDescr hwAccelCtx) ubyte* keyMaterial,sbyte4 keyLength,
                  const ubyte* data, ubyte4 dataLen, ubyte * retData /* dataLen - 8 */);


#ifdef __cplusplus
}
#endif

#endif
