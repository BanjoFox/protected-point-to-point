/* Version: mss_v5_3_1_13561 */
/*
 * aes_cmac.c
 *
 * AES-CMAC Implementation (RFC 4493)
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
#include "../common/merrors.h"

#include "../crypto/aes.h"
#include "../crypto/aes_cmac.h"

#if (!defined(__DISABLE_AES_CIPHERS__))

#include "../common/mdefs.h"
#include "../common/mrtos.h"
#include "../common/mstdlib.h"
#include "../common/debug_console.h"
#include "../crypto/aesalgo.h"


/*------------------------------------------------------------------*/

static void
AESCMAC_generateSubKeysAux( ubyte out[AES_BLOCK_SIZE],
                        const ubyte in[AES_BLOCK_SIZE],
                        const ubyte Rb[AES_BLOCK_SIZE])
{
    sbyte4 i;

    for (i = 0; i < AES_BLOCK_SIZE - 1; ++i)
    {
        out[i] = ((in[i] << 1) | (in[i+1] >> 7));
    }
    out[i] = (in[i] << 1);

    if (in[0] & 0x80)  /* need to XOR with the Rb constant */
    {
        for (i = 0; i < AES_BLOCK_SIZE; ++i)
        {
            out[i] ^= Rb[i];
        }
    }
    else
    {
        /* do something equivalent in time and energy
        to prevent side channel attacks */
        for (i = 0; i < AES_BLOCK_SIZE; ++i)
        {
            out[0] ^= Rb[0]; /* xoring one byte an even number of time */
        }
    }
}


/*------------------------------------------------------------------*/

static void
AESCMAC_generateSubKeys( aesCipherContext* pAESCtx, ubyte K1[AES_BLOCK_SIZE],
                        ubyte K2[AES_BLOCK_SIZE])
{
    ubyte zeroBlock[AES_BLOCK_SIZE] = {0};
    ubyte L[AES_BLOCK_SIZE];

    aesEncrypt(pAESCtx->rk, pAESCtx->Nr, zeroBlock, L);

    zeroBlock[AES_BLOCK_SIZE - 1] = 0x87; /* the Rb constant */

    AESCMAC_generateSubKeysAux( K1, L, zeroBlock);
    AESCMAC_generateSubKeysAux( K2, K1, zeroBlock);
}


/*------------------------------------------------------------------*/

extern MSTATUS
AESCMAC_init(MOC_SYM(hwAccelDescr hwAccelCtx) const ubyte *keyMaterial, sbyte4 keyLength, AESCMAC_Ctx *pCtx)
{
    MSTATUS status;

    if ( !pCtx || !keyMaterial)
        return ERR_NULL_POINTER;

    /* initialize the AES context */
    if (OK > ( status = AESALGO_makeAesKey(&pCtx->aesCtx, 8 * keyLength, keyMaterial,
                                           1, MODE_CBC)))
    {
        return status;
    }

    AES_OMAC_init( &pCtx->omacCtx);
    return OK;
}


/*------------------------------------------------------------------*/

extern MSTATUS
AESCMAC_update(MOC_SYM(hwAccelDescr hwAccelCtx) const ubyte* data, sbyte4 dataLength,
                                AESCMAC_Ctx* pCtx)
{
    if ( !data || !  pCtx)
        return ERR_NULL_POINTER;

    return AES_OMAC_update( MOC_SYM(hwAccelCtx) &pCtx->aesCtx,
                                &pCtx->omacCtx, data, dataLength);
}


/*------------------------------------------------------------------*/

extern MSTATUS
AESCMAC_final( MOC_SYM(hwAccelDescr hwAccelCtx) ubyte cmac[CMAC_RESULT_SIZE],
                        AESCMAC_Ctx* pCtx)
{
    if (!cmac || !pCtx)
        return ERR_NULL_POINTER;

    return AES_OMAC_final( MOC_SYM(hwAccelCtx) &pCtx->aesCtx,
                            &pCtx->omacCtx, cmac);
}


/* implementation */

/*---------------------------------------------------------------------------*/

extern MSTATUS
AES_OMAC_init(AES_OMAC_Ctx* pOMACCtx)
{
    if ( !pOMACCtx)
        return ERR_NULL_POINTER;

    pOMACCtx->pendingLen = 0;
    memset( pOMACCtx->currBlock, 0, AES_BLOCK_SIZE);
    return OK;
}


/*---------------------------------------------------------------------------*/

extern MSTATUS
AES_OMAC_update(MOC_SYM(hwAccelDescr hwAccelCtx) aesCipherContext* pAESCtx,
                    AES_OMAC_Ctx* pOMACCtx, const ubyte* data,
                    sbyte4 dataLength)
{
    sbyte4 i;

    if (!pAESCtx || !pOMACCtx || !data)
        return ERR_NULL_POINTER;

    if ( pOMACCtx->pendingLen > 0 )
    {
        while ( pOMACCtx->pendingLen < AES_BLOCK_SIZE && dataLength > 0)
        {
            pOMACCtx->pending[ pOMACCtx->pendingLen++] = *data++;
            dataLength--;
        }
        /* should we proceed ? only if full and more to do */
        if ( AES_BLOCK_SIZE == pOMACCtx->pendingLen && dataLength > 0)
        {
            pOMACCtx->pendingLen = 0;
            for (i = 0; i < AES_BLOCK_SIZE; ++i)
            {
                pOMACCtx->pending[i] ^= pOMACCtx->currBlock[i];
            }
            aesEncrypt(pAESCtx->rk, pAESCtx->Nr,
                        pOMACCtx->pending, pOMACCtx->currBlock);
        }
    }

    /* process all the bytes if there are more than AES_BLOCK_SIZE */
    while ( dataLength > AES_BLOCK_SIZE)
    {
        for (i = 0; i < AES_BLOCK_SIZE; ++i, dataLength--)
        {
            pOMACCtx->pending[i] = (pOMACCtx->currBlock[i] ^ (*data++));
        }
        aesEncrypt( pAESCtx->rk, pAESCtx->Nr,
                    pOMACCtx->pending, pOMACCtx->currBlock);
    }

    /* at this point, 1 < dataLength <= AES_BLOCK_SIZE */
    /* save the bytes that can't be processed in the pending array */
    while ( dataLength-- > 0)
    {
        pOMACCtx->pending[pOMACCtx->pendingLen++] = *data++;
    }
    return OK;
}


/*---------------------------------------------------------------------------*/

extern MSTATUS
AES_OMAC_final( MOC_SYM(hwAccelDescr hwAccelCtx) aesCipherContext* pAESCtx,
                AES_OMAC_Ctx* pOMACCtx,
                ubyte cmac[CMAC_RESULT_SIZE])
{
    const ubyte* subKey;
    sbyte4 i;
    ubyte K1[AES_BLOCK_SIZE];
    ubyte K2[AES_BLOCK_SIZE];

    if (!pAESCtx || !pOMACCtx || !K1 || !K2 || !cmac)
        return ERR_NULL_POINTER;

    AESCMAC_generateSubKeys( pAESCtx, K1, K2);

    /* which case are we in  */
    if ( AES_BLOCK_SIZE == pOMACCtx->pendingLen)
    {
        /* multiple of block size -> use K1 */
        subKey = K1;
    }
    else
    {
        /* pad and use K2 */
        pOMACCtx->pending[ pOMACCtx->pendingLen++] = 0x80;
        while ( pOMACCtx->pendingLen < AES_BLOCK_SIZE)
        {
            pOMACCtx->pending[pOMACCtx->pendingLen++] = 0;
        }
        subKey = K2;
    }

    for (i = 0; i < AES_BLOCK_SIZE; ++i)
    {
        pOMACCtx->pending[i] ^= subKey[i];
        pOMACCtx->pending[i] ^= pOMACCtx->currBlock[i];
    }
    aesEncrypt(pAESCtx->rk, pAESCtx->Nr, pOMACCtx->pending, cmac);

    return OK;
}


#endif /* (!defined(__DISABLE_AES_CIPHERS__) && !defined(__AES_HARDWARE_CIPHER__)) */
