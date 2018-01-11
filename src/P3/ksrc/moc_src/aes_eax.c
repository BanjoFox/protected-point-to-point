/* Version: mss_v5_3_1_13561 */
/*
 * aes_eax.c
 *
 * AES-EAX Implementation
 *
 * Copyright Mocana Corp 2006-2007. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */


/*------------------------------------------------------------------*/

#include "../common/moptions.h"
#include "../common/mtypes.h"
#include "../common/mocana.h"
#include "../crypto/hw_accel.h"

#include "../common/mdefs.h"
#include "../common/merrors.h"
#include "../common/mstdlib.h"
#include "../common/mrtos.h"
#include "../crypto/aes.h"
#include "../crypto/aes_ctr.h"
#include "../crypto/aes_cmac.h"
#include "../crypto/aes_eax.h"

#if (!defined(__DISABLE_AES_CIPHERS__))

#include "../common/debug_console.h"
#include "../crypto/aesalgo.h"

/*----------------------------------------------------------------------------*/

static MSTATUS
AES_EAX_initOMAC( MOC_SYM(hwAccelDescr hwAccelCtx) aesCipherContext* pAESCtx,
                 AES_OMAC_Ctx* pOMACCtx,
                 ubyte tweak)
{
    MSTATUS status;
    ubyte tweakBlock[AES_BLOCK_SIZE] = {0};

    if ( OK > ( status = AES_OMAC_init( pOMACCtx)))
        return status;

    tweakBlock[AES_BLOCK_SIZE-1] = tweak;

    return AES_OMAC_update( MOC_SYM(hwAccelCtx) pAESCtx, pOMACCtx, tweakBlock, AES_BLOCK_SIZE);
}


/*----------------------------------------------------------------------------*/

extern MSTATUS
AES_EAX_init(MOC_SYM(hwAccelDescr hwAccelCtx) const ubyte* keyMaterial,
             ubyte4 keyLength, const ubyte* nonce, ubyte4 nonceLength,
             AES_EAX_Ctx* pCtx)
{
    MSTATUS         status;
    AES_OMAC_Ctx    nonceOMAC;

    if ( !pCtx || !keyMaterial)
        return ERR_NULL_POINTER;
    /* NOTE: nonce can be NULL */

    /* initialize the AES context */
    if (OK > ( status = AESALGO_makeAesKey(&pCtx->aesCTRCtx.ctx,
                                            keyLength * 8, keyMaterial,
                                           1, MODE_CBC)))
    {
        return status;
    }

    /* do a complete OMAC with the nonce and tweak = 0 */
    if ( OK > ( status = AES_EAX_initOMAC( MOC_SYM(hwAccelCtx)
                              &pCtx->aesCTRCtx.ctx, &nonceOMAC, 0)))
    {
        return status;
    }
    if ( nonce && nonceLength > 0)
    {
        if ( OK > ( status = AES_OMAC_update( MOC_SYM(hwAccelCtx)
                                            &pCtx->aesCTRCtx.ctx,
                                            &nonceOMAC, nonce, nonceLength)))
        {
            return status;
        }
    }
    if ( OK > ( status = AES_OMAC_final( MOC_SYM(hwAccelCtx)
                                        &pCtx->aesCTRCtx.ctx,
                                        &nonceOMAC, pCtx->N)))
    {
        return status;
    }

    /* use the result of the OMAC of the Nonce as the block for
    the AES Counter context */
    memcpy( pCtx->aesCTRCtx.u.counterBlock, pCtx->N, AES_BLOCK_SIZE);
    pCtx->aesCTRCtx.offset = 0;
    /* at this point the Counter Ctx has been initialized */

    /* initialize the rest = headerOMAC and cipherOMAC */
    if ( OK > ( status = AES_EAX_initOMAC( MOC_SYM(hwAccelCtx)
                              &pCtx->aesCTRCtx.ctx, &pCtx->headerOMAC, 1)))
    {
        return status;
    }

    if ( OK > ( status = AES_EAX_initOMAC( MOC_SYM(hwAccelCtx)
                              &pCtx->aesCTRCtx.ctx, &pCtx->cipherOMAC, 2)))
    {
        return status;
    }
    /* we can now proceed header and messages as they come */
    return OK;
}


/*---------------------------------------------------------------------------*/

extern MSTATUS
AES_EAX_updateHeader(MOC_SYM(hwAccelDescr hwAccelCtx) const ubyte* headerData,
                     sbyte4 dataLength, AES_EAX_Ctx* pCtx)
{
    if ( !headerData || !  pCtx)
        return ERR_NULL_POINTER;

    return AES_OMAC_update( MOC_SYM(hwAccelCtx) &pCtx->aesCTRCtx.ctx,
                            &pCtx->headerOMAC, headerData, dataLength);
}


/*---------------------------------------------------------------------------*/

extern MSTATUS
AES_EAX_getPlainText(  MOC_SYM(hwAccelDescr hwAccelCtx)
                        ubyte* cipherText, sbyte4 cipherLen,
                        AES_EAX_Ctx* pCtx)
{
    if ( !cipherText || !pCtx)
        return ERR_NULL_POINTER;

    return DoAESCTR( MOC_SYM(hwAccelCtx) &pCtx->aesCTRCtx,
                        cipherText, cipherLen, 1, NULL);
}


/*---------------------------------------------------------------------------*/

extern MSTATUS
AES_EAX_encryptMessage(MOC_SYM(hwAccelDescr hwAccelCtx) ubyte* msgData,
                       sbyte4 msgLen, AES_EAX_Ctx* pCtx)
{
    MSTATUS status;

    if ( !msgData || !pCtx)
        return ERR_NULL_POINTER;

    if ( OK > (status = DoAESCTR( MOC_SYM(hwAccelCtx) &pCtx->aesCTRCtx,
                                    msgData, msgLen, 1, NULL)))
    {
        return status;
    }

    /* add the cipher to the OMAC */
    return AES_OMAC_update(MOC_SYM(hwAccelCtx)
                           &pCtx->aesCTRCtx.ctx, &pCtx->cipherOMAC,
                            msgData, msgLen);
}


/*---------------------------------------------------------------------------*/

extern MSTATUS
AES_EAX_final( MOC_SYM(hwAccelDescr hwAccelCtx) ubyte tag[/*tagLen*/],
              sbyte4 tagLen, AES_EAX_Ctx* pCtx)
{
    MSTATUS status;
    ubyte omacRes[ CMAC_RESULT_SIZE];
    sbyte4 i;

    if (!tag || !pCtx)
        return ERR_NULL_POINTER;

    /* tag is the first tagLen byte of
        OMAC(nonce) ^ OMAC(cipher) ^ OMAC(header) */

    if ( tagLen > AES_BLOCK_SIZE || tagLen < 0)
    {
        return ERR_INVALID_ARG;
    }


    /* complete the two OMAC */
    if ( OK > ( status = AES_OMAC_final( MOC_SYM(hwAccelCtx)
                                        &pCtx->aesCTRCtx.ctx,
                                        &pCtx->cipherOMAC,
                                        omacRes)))
    {
        return status;
    }
    for ( i = 0; i < tagLen; ++i)
    {
        tag[i] = (omacRes[i] ^ (pCtx->N[i]));
    }
    if ( OK > ( status = AES_OMAC_final( MOC_SYM(hwAccelCtx)
                                        &pCtx->aesCTRCtx.ctx,
                                        &pCtx->headerOMAC,
                                        omacRes)))
    {
        return status;
    }

    for ( i = 0; i < tagLen; ++i)
    {
        tag[i] ^= omacRes[i];
    }

    return OK;
}


/*---------------------------------------------------------------------------*/

extern MSTATUS
AES_EAX_generateTag( MOC_SYM(hwAccelDescr hwAccelCtx)
                   const ubyte* cipherText, sbyte4 cipherLen,
                   const ubyte* header, sbyte4 headerLen,
                   ubyte tag[/*tagLen*/], sbyte4 tagLen,
                   AES_EAX_Ctx* pCtx)
{
    MSTATUS status;
    ubyte omacRes[ CMAC_RESULT_SIZE];
    sbyte4 i;

    if (!cipherText || !tag || !pCtx)
        return ERR_NULL_POINTER;

    /* header can be NULL */

    /* tag is the first tagLen byte of
        OMAC(nonce) ^ OMAC(cipher) ^ OMAC(header) */
    if ( tagLen > AES_BLOCK_SIZE || tagLen < 0)
    {
        return ERR_INVALID_ARG;
    }

    /* do the two complete OMAC operation */
    if ( OK > ( status = AES_OMAC_update( MOC_SYM(hwAccelCtx)
                                            &pCtx->aesCTRCtx.ctx,
                                            &pCtx->cipherOMAC,
                                            cipherText, cipherLen)))
    {
        return status;
    }

    if ( OK > ( status = AES_OMAC_final( MOC_SYM(hwAccelCtx)
                                        &pCtx->aesCTRCtx.ctx,
                                        &pCtx->cipherOMAC,
                                        omacRes)))
    {
        return status;
    }
    for ( i = 0; i < tagLen; ++i)
    {
        tag[i] = (omacRes[i] ^ (pCtx->N[i]));
    }

    if ( header)
    {
        if ( OK > ( status = AES_OMAC_update( MOC_SYM(hwAccelCtx)
                                            &pCtx->aesCTRCtx.ctx,
                                            &pCtx->headerOMAC,
                                            header, headerLen)))
        {
            return status;
        }

        if ( OK > ( status = AES_OMAC_final( MOC_SYM(hwAccelCtx)
                                        &pCtx->aesCTRCtx.ctx,
                                        &pCtx->headerOMAC,
                                        omacRes)))
        {
            return status;
        }

        for ( i = 0; i < tagLen; ++i)
        {
            tag[i] ^= omacRes[i];
        }
    }
    return OK;
}


#endif /* (!defined(__DISABLE_AES_CIPHERS__) && !defined(__AES_HARDWARE_CIPHER__)) */
