/* Version: mss_v5_3_1_13561 */
/*
 * aes_eax.h
 *
 * AES-EAX Implementation
 *
 * Copyright Mocana Corp 2006-2007. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 * The EAX mode of operation is not tied to the AES
 * cipher, it can be used with any cipher, but this
 * implementation does not parameterize the cipher
 * used, since AES is only used for this mode currently.
 * "Do the simplest thing that works".
 *
 */


/*------------------------------------------------------------------*/

#ifndef __AES_EAX_HEADER__
#define __AES_EAX_HEADER__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AES_EAX_Ctx
{
    AES_CTR_Ctx             aesCTRCtx;      /* used for encryption */
    AES_OMAC_Ctx            headerOMAC;         /* tweak = 1 */
    AES_OMAC_Ctx            cipherOMAC;         /* tweak = 2 */
    ubyte                   N[AES_BLOCK_SIZE];  /* OMAC of the nonce */
} AES_EAX_Ctx;

/*------------------------------------------------------------------*/

/*  Exposed Function prototypes  */
/* AES EAX -- cf original article for explanation  */
/* This API support online processing of data
i.e. after AES_EAX_init has been called, AES_EAX_updateHeader and
AES_EAX_encryptMessage (AES_EAX_decryptMessage) can be called multiple
time and in any order. To get the tag (MAC), the final call should be
AES_EAX_final. */
MOC_EXTERN MSTATUS AES_EAX_init(MOC_SYM(hwAccelDescr hwAccelCtx) const ubyte* keyMaterial,
                                ubyte4 keyLength,
                                const ubyte* nonce,
                                ubyte4 nonceLength,
                                AES_EAX_Ctx* pCtx);
MOC_EXTERN MSTATUS AES_EAX_updateHeader(MOC_SYM(hwAccelDescr hwAccelCtx) const ubyte* headerData,
                                    sbyte4 dataLength,
                                    AES_EAX_Ctx* pCtx);
MOC_EXTERN MSTATUS AES_EAX_encryptMessage(MOC_SYM(hwAccelDescr hwAccelCtx) ubyte* msgData,
                                       sbyte4 msgLen, AES_EAX_Ctx* pCtx);
#define AES_EAX_decryptMessage AES_EAX_encryptMessage /* encrypt and decrypt are the same */
MOC_EXTERN MSTATUS AES_EAX_final(MOC_SYM(hwAccelDescr hwAccelCtx) ubyte tag[/*tagLen*/],
                                        sbyte4 tagLen, AES_EAX_Ctx* pCtx);

/* For decryption/tage verification, 2 workflows:
 0) (common) AES_EAX_init
 Workflow a:
 1a) call AES_EAX_updateHeader AES_EAX_decryptMessage multiple time and in any order
 this will also get the plain text
 2a) call AES_EAX_final to get the tag and compare it with the provided tag
 Workflow b:
 1b) call AES_EAX_generateTag to get the tag and compare it with the provided tag
 2b) if the tag was verified call AES_EAX_getPlainText to get the plain text
 multiple calls are possible  to this function. AES_EAX_decrypt can also be called
 but is less efficient. In workflow b) AES_EAX_final should not be called: it will not
 give the correct result */

MOC_EXTERN MSTATUS AES_EAX_generateTag( MOC_SYM(hwAccelDescr hwAccelCtx)
                                   const ubyte* cipherText, sbyte4 cipherLen,
                                   const ubyte* header, sbyte4 headerLen,
                                   ubyte tag[/*tagLen*/], sbyte4 tagLen,
                                   AES_EAX_Ctx* pCtx);

MOC_EXTERN MSTATUS AES_EAX_getPlainText(  MOC_SYM(hwAccelDescr hwAccelCtx)
                                    ubyte* cipherText, sbyte4 cipherLen,
                                    AES_EAX_Ctx* pCtx);
#ifdef __cplusplus
}
#endif

#endif /* __AES_CMAC_HEADER__ */

