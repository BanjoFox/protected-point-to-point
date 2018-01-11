/* Version: mss_v5_3_1_13561 */
/*
 * aes_xcbc_mac_96.h
 *
 * AES-XCBC-MAC-96 and derived Implementation ( RFC 3566, RFC 3664, RFC 4434)
 *
 * Copyright Mocana Corp 2006-2007. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */


/*------------------------------------------------------------------*/

#ifndef __AES_XCBC_MAC_96_HEADER__
#define __AES_XCBC_MAC_96_HEADER__

#ifdef __cplusplus
extern "C" {
#endif

#define AES_XCBC_MAC_96_RESULT_SIZE     (12) /* 96 bits */
#define AES_XCBC_PRF_128_RESULT_SIZE    (AES_BLOCK_SIZE) /* 128 bits */

typedef struct AES_XCBC_MAC_96_Ctx
{
    aesCipherContext        keyAesCtx;  /* key K */
    aesCipherContext        aesCtx;     /* key K1, K2 or K3 */
    ubyte                   currBlock[AES_BLOCK_SIZE];
    /* bytes received -- we delay the processing until more bytes are
    received or final is called */
    ubyte                   pending[AES_BLOCK_SIZE] ;
    /* length of bytes received above <= AES_BLOCK_SIZE */
    ubyte                   pendingLen;
} AES_XCBC_MAC_96_Ctx, AES_XCBC_PRF_128_Ctx;


/*------------------------------------------------------------------*/

/*  Function prototypes  */
/* AES CMAC -- cf RFC 3566 for explanations of parameters. */
MOC_EXTERN MSTATUS AES_XCBC_MAC_96_init(MOC_SYM(hwAccelDescr hwAccelCtx) const ubyte keyMaterial[AES_BLOCK_SIZE],
                                        AES_XCBC_MAC_96_Ctx* pCtx);
MOC_EXTERN MSTATUS AES_XCBC_MAC_96_update(MOC_SYM(hwAccelDescr hwAccelCtx) const ubyte* data, sbyte4 dataLength,
                                        AES_XCBC_MAC_96_Ctx* pCtx);
MOC_EXTERN MSTATUS AES_XCBC_MAC_96_final( MOC_SYM(hwAccelDescr hwAccelCtx) ubyte cmac[AES_XCBC_MAC_96_RESULT_SIZE],
                                        AES_XCBC_MAC_96_Ctx* pCtx);

/* AES CMAC -- cf RFC 4434 */
MOC_EXTERN MSTATUS AES_XCBC_PRF_128_init(MOC_SYM(hwAccelDescr hwAccelCtx) const ubyte keyMaterial[/*keyLength*/],
                                        sbyte4 keyLength,
                                        AES_XCBC_PRF_128_Ctx* pCtx);
#define AES_XCBC_PRF_128_update     AES_XCBC_MAC_96_update
MOC_EXTERN MSTATUS AES_XCBC_PRF_128_update(MOC_SYM(hwAccelDescr hwAccelCtx) const ubyte* data, sbyte4 dataLength,
                                        AES_XCBC_PRF_128_Ctx* pCtx);
MOC_EXTERN MSTATUS AES_XCBC_PRF_128_final( MOC_SYM(hwAccelDescr hwAccelCtx) ubyte cmac[AES_XCBC_PRF_128_RESULT_SIZE],
                                        AES_XCBC_PRF_128_Ctx* pCtx);

#ifdef __cplusplus
}
#endif

#endif /* __AES_XCBC_MAC_96_HEADER__ */

