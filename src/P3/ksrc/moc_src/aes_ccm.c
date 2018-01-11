/* Version: mss_v5_3_1_13561 */
/*
 * aes_ccm.c
 *
 * AES-CCM Implementation (RFC 4309)
 *
 * Copyright Mocana Corp 2006-2007. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */

#if (defined(__ZEROIZE_TEST__))
#if (defined(__KERNEL__))
#include <linux/kernel.h>       /* for printk */
#define FIPS_PRINT              printk
#else
#include <stdio.h>              /* for printf */
#define FIPS_PRINT              printf
#endif
#endif /* (defined(__ZEROIZE_TEST__)) */

#include "../common/moptions.h"
#include "../common/mtypes.h"
#include "../common/mocana.h"
#include "../crypto/hw_accel.h"

#if (!defined(__DISABLE_AES_CIPHERS__))

#include "../common/mdefs.h"
#include "../common/merrors.h"
#include "../common/mrtos.h"
#include "../common/mstdlib.h"
#include "../common/debug_console.h"
#ifdef __ENABLE_MOCANA_FIPS_MODULE__
#include "../crypto/fips.h"
#endif
#include "../crypto/aesalgo.h"
#include "../crypto/aes.h"
#include "../crypto/aes_ctr.h"
#include "../crypto/aes_ccm.h"


/*------------------------------------------------------------------*/

static MSTATUS
AESCCM_validateParams( ubyte M, ubyte L,
                        ubyte* keyMaterial, const ubyte* nonce,
                        ubyte* eData, ubyte4 eDataLength,
                        const ubyte output[/*M*/],
                        ubyte* pMp)
{
    ubyte4  tempL;
    ubyte   Mp;

    if ( !pMp || !keyMaterial || !nonce || !eData || !output)
        return ERR_NULL_POINTER;

    /* eDataLength must be encoded in L bytes at max */
    for (tempL = L; ((tempL) && (eDataLength)); tempL--)
        eDataLength >>= 8;

    if (eDataLength)
        return ERR_INVALID_ARG;

    /* M must be 4,6,8,...., 16 */
    if (M&1)
        return ERR_INVALID_ARG;
    Mp = (M - 2) / 2;

    if ( Mp <= 0 || Mp > 7)
        return ERR_INVALID_ARG;

    /* L must be 2 to 8 */
    if ( L < 2 || L > 8)
        return ERR_INVALID_ARG;

    *pMp = Mp;
    return OK;
}


/*------------------------------------------------------------------*/

static void
AESCCM_authenticateAux( MOC_SYM(hwAccelDescr hwAccelCtx) ubyte4* rk, ubyte4 Nr,
                        ubyte* B, ubyte* X, const ubyte* data, ubyte4 dataLen)
{
    ubyte4 i;

    while ( dataLen > 16)
    {
        for ( i = 0; i < 16; ++i)
        {
            B[i] = (*data++) ^ X[i];
        }
        dataLen -= 16;
        aesEncrypt( rk, Nr, B, X);
    }

    if ( dataLen > 0)
    {
        for (i =0; i < dataLen; ++i)
        {
            B[i] = (*data++) ^ X[i];
        }
        for (; i < 16; ++i)
        {
            B[i] = 0 ^ X[i];
        }
        aesEncrypt( rk, Nr, B, X);
    }
}


/*------------------------------------------------------------------*/

static MSTATUS
AESCCM_doAuthentication( MOC_SYM(hwAccelDescr hwAccelCtx) aesCTRCipherContext* pCtx,
                       ubyte Mp, ubyte L, const ubyte* nonce, const ubyte* eData,
                       ubyte4 eDataLength, const ubyte* aData, ubyte4 aDataLength,
                       ubyte T[16])
{
    ubyte B[16];
    sbyte4 i;
    ubyte4 temp;

    /* construct B_0 */
    /* byte 0 is flags */
    B[0] = 8 * Mp + L - 1;
    if ( aData && aDataLength)
        B[0] |= (1 << 6);

    /* other bytes are nonce + eDataLength */
    memcpy( B+1, nonce, 15-L);

    /* copy eDataLength in big-endian format */
    temp = eDataLength;
    for ( i = 0; i < L; ++i)
    {
        B[15-i] = (ubyte)(temp & 0xff);
        temp >>= 8;
    }


    /* X_1 = E(K, B_0) */
    aesEncrypt( pCtx->ctx.rk, pCtx->ctx.Nr, B, T);

    if ( aData && aDataLength)
    {
        if ( aDataLength < ( 1 << 16) - (1 << 8))
        {
            B[0] = (ubyte)((aDataLength >> 8) & 0xff);
            B[1] = (ubyte)((aDataLength) & 0xff);
            if ( aDataLength >= 14)
            {
                memcpy( B + 2, aData, 14);
                aData += 14;
                aDataLength -= 14;
            }
            else /* pad with 0 */
            {
                memcpy( B + 2, aData, aDataLength);
                memset( B + 2 + aDataLength, 0, 14-aDataLength);
                aDataLength = 0;
            }
        }
        else /* if ( aDataLength < ( 1 << 32)) */
        {
            B[0] = 0xFF;
            B[1] = 0xFE;
            BIGEND32( B + 2, aDataLength);
            memcpy( B + 6, aData, 10);
            aData += 10;
            aDataLength -= 10;
        }
        /* encrypt block */
        for ( i = 0; i < AES_BLOCK_SIZE; ++i)
        {
            B[i] ^= T[i];
        }
        aesEncrypt( pCtx->ctx.rk, pCtx->ctx.Nr, B, T);

        AESCCM_authenticateAux( MOC_SYM( hwAccelCtx) pCtx->ctx.rk, pCtx->ctx.Nr,
                            B, T, aData, aDataLength);
    }

    AESCCM_authenticateAux(  MOC_SYM( hwAccelCtx) pCtx->ctx.rk, pCtx->ctx.Nr,
                            B, T, eData, eDataLength);
    return OK;
}


/*------------------------------------------------------------------*/

static MSTATUS
AESCCM_doCTREncryption( MOC_SYM(hwAccelDescr hwAccelCtx) aesCTRCipherContext* pCtx,
                       ubyte M, ubyte L, const ubyte* nonce, ubyte* eData,
                       ubyte4 eDataLength, const ubyte T[/*M*/], ubyte U[/*M*/])

{
    sbyte4 i;
    ubyte* A;
    ubyte S[16];

    /*************************** Encryption ******************************/
    A = pCtx->u.counterBlock;
    A[0] = L-1;
    memcpy( A+1, nonce, 15-L);
    for ( i = 0; i < L; ++i)
    {
        A[15-i] = 0;
    }
    aesEncrypt( pCtx->ctx.rk, pCtx->ctx.Nr, A, S);
    for (i = 0; i < M; ++i)
    {
        U[i] = S[i] ^ T[i]; /* U = S_0 ^ T */
    }

    /* rest of encryption */
    /* increment the block first ! */
    A[15] = 1;
    return DoAESCTR(MOC_SYM( hwAccelCtx) pCtx, eData, eDataLength, 1, NULL);
}


/*------------------------------------------------------------------*/

extern MSTATUS
AESCCM_encrypt(MOC_SYM(hwAccelDescr hwAccelCtx) ubyte M, ubyte L,
               ubyte* keyMaterial, sbyte4 keyLength,
               const ubyte* nonce, ubyte* eData, ubyte4 eDataLength,
               const ubyte* aData, ubyte4 aDataLength, ubyte U[/*M*/])
{
    ubyte               T[16];
    ubyte               Mp;
    aesCTRCipherContext ctx;
    MSTATUS             status;

#ifdef __ENABLE_MOCANA_FIPS_MODULE__
    if (OK != gFIPS_powerupStatus)
        return gFIPS_powerupStatus;
#endif /* __ENABLE_MOCANA_FIPS_MODULE__ */

    if (OK > (status = AESCCM_validateParams(M, L, keyMaterial, nonce,
                                             eData, eDataLength, U, &Mp)))
    {
        goto exit;
    }

    /* create AES context */
    memset((ubyte *)&ctx, 0x00, sizeof(ctx));
    if (OK > ( status = AESALGO_makeAesKey(&ctx.ctx, 8 * keyLength, keyMaterial, 1, MODE_ECB)))
        goto exit;

    /*************************** Authentication **************************/
    if ( OK > ( status = AESCCM_doAuthentication( MOC_SYM(hwAccelCtx) &ctx,
                                    Mp, L, nonce, eData, eDataLength, aData, aDataLength, T)))
    {
        goto exit;
    }

    /*************************** Encryption ******************************/

    status = AESCCM_doCTREncryption(MOC_SYM(hwAccelCtx) &ctx, M, L,
                                    nonce, eData, eDataLength, T, U);

exit:
#ifdef __ZEROIZE_TEST__
    {
        int counter = 0;

        FIPS_PRINT("\nAESCCM - Before Zeroization\n");
        for (counter = 0; counter < sizeof(aesCTRCipherContext); counter++)
        {
            FIPS_PRINT("%02x", *(((ubyte *)&ctx) + counter));
        }
    }
#endif

    memset((ubyte *)&ctx, 0x00, sizeof(ctx));

#ifdef __ZEROIZE_TEST__
    {
        int counter = 0;

        FIPS_PRINT("\nAESCCM - After Zeroization\n");
        for( counter = 0; counter < sizeof(aesCTRCipherContext); counter++)
        {
            FIPS_PRINT("%02x", *(((ubyte *)&ctx) + counter));
        }
    }
#endif

    return status;
}


/*------------------------------------------------------------------*/

extern MSTATUS
AESCCM_decrypt(MOC_SYM(hwAccelDescr hwAccelCtx) ubyte M, ubyte L,
               ubyte* keyMaterial, sbyte4 keyLength,
               const ubyte* nonce, ubyte* eData, ubyte4 eDataLength,
               const ubyte* aData, ubyte4 aDataLength, const ubyte U[/*M*/])
{
    ubyte               T1[16];
    ubyte               T2[16];
    ubyte               Mp;
    sbyte4              resCmp;
    aesCTRCipherContext ctx;
    MSTATUS             status;

#ifdef __ENABLE_MOCANA_FIPS_MODULE__
    if (OK != gFIPS_powerupStatus)
        return gFIPS_powerupStatus;
#endif /* __ENABLE_MOCANA_FIPS_MODULE__ */

    if (OK > (status = AESCCM_validateParams(M, L, keyMaterial, nonce,
                                             eData, eDataLength, U, &Mp)))
    {
        goto exit;
    }

    /* create AES context */
    memset((ubyte *)&ctx, 0x00, sizeof(ctx));
    if (OK > ( status = AESALGO_makeAesKey(&ctx.ctx, 8 * keyLength, keyMaterial, 1, MODE_ECB)))
        goto exit;

    /**************** Decryption *****************************/
    if (OK > (status = AESCCM_doCTREncryption(MOC_SYM(hwAccelCtx) &ctx, M, L,
                       nonce, eData, eDataLength, U, T1)))
    {
        goto exit;
    }


    /**************** Authentication *************************/
    if (OK > ( status = AESCCM_doAuthentication(MOC_SYM(hwAccelCtx) &ctx,
                                                Mp, L, nonce, eData, eDataLength, aData, aDataLength, T2)))
    {
        goto exit;
    }

    /* verify T1 == T2 */
    memcpy( T1, T2, M, &resCmp);

    status = (resCmp) ? ERR_AES_CCM_AUTH_FAIL : OK;

exit:
#ifdef __ZEROIZE_TEST__
    {
        int counter = 0;
        FIPS_PRINT("\nAESCCM - Before Zeroization\n");
        for( counter = 0; counter < sizeof(aesCTRCipherContext); counter++)
        {
            FIPS_PRINT("%02x",*(((ubyte *)&ctx) + counter));
        }
    }
#endif

    memset((ubyte *)&ctx, 0x00, sizeof(ctx));

#ifdef __ZEROIZE_TEST__
    {
        int counter = 0;
        FIPS_PRINT("\nAESCCM - After Zeroization\n");
        for( counter = 0; counter < sizeof(aesCTRCipherContext); counter++)
        {
            FIPS_PRINT("%02x",*(((ubyte *)&ctx) + counter));
        }
    }
#endif

    return status;
}


#endif /* (!defined(__DISABLE_AES_CIPHERS__) && !defined(__AES_HARDWARE_CIPHER__)) */
