/* Version: mss_v5_3_1_13561 */
/*
 * aes_xcbc_mac_96.c
 *
 * AES-XCBC-MAC-96 and derived Implementation ( RFC 3566, RFC 3664, RFC 4434)
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
#include "../crypto/aes.h"
#include "../crypto/aes_xcbc_mac_96.h"

#if (!defined(__DISABLE_AES_CIPHERS__))

#include "../common/mrtos.h"
#include "../common/mstdlib.h"
#include "../common/debug_console.h"
#include "../crypto/aesalgo.h"


/*---------------------------------------------------------------------------*/

static void
AES_XCBC_MAC_96_genKey( aesCipherContext* pCtx, ubyte keyType,
                       ubyte key[AES_BLOCK_SIZE])
{
    ubyte keyGen[AES_BLOCK_SIZE];

    memset( keyGen, keyType, AES_BLOCK_SIZE);
    aesEncrypt(pCtx->rk, pCtx->Nr, keyGen, key);
}

/*---------------------------------------------------------------------------*/

extern MSTATUS
AES_XCBC_MAC_96_init(MOC_SYM(hwAccelDescr hwAccelCtx)
                     const ubyte keyMaterial[AES_BLOCK_SIZE],
                     AES_XCBC_MAC_96_Ctx* pCtx)
{
    MSTATUS status;
    ubyte   key[AES_BLOCK_SIZE];

    if ( !pCtx || !keyMaterial)
        return ERR_NULL_POINTER;

    /* initialize the 1st AES context */
    if (OK > ( status = AESALGO_makeAesKey(&pCtx->keyAesCtx, 128, keyMaterial,
                                           1, MODE_CBC)))
    {
        return status;
    }

    /* initialize the 2nd context with K1 (cf. RFC 3566) */
    AES_XCBC_MAC_96_genKey( &pCtx->keyAesCtx, 1, key);

    if (OK > ( status = AESALGO_makeAesKey(&pCtx->aesCtx, 128, key,
                                           1, MODE_CBC)))
    {
        return status;
    }

    /* initialize the rest */
    pCtx->pendingLen = 0;
    memset( pCtx->currBlock, 0, AES_BLOCK_SIZE);

    return OK;
}


/*---------------------------------------------------------------------------*/

extern MSTATUS
AES_XCBC_PRF_128_init(MOC_SYM(hwAccelDescr hwAccelCtx)
                      const ubyte keyMaterial[/*keyLength*/],
                      sbyte4 keyLength,
                      AES_XCBC_PRF_128_Ctx* pCtx)
{
    ubyte key[ AES_BLOCK_SIZE];
    sbyte4 i;

    /* basically identical to MAC_96 but allows key shorter or longer
    than 128 bits -- 16 bytes */
    if ( AES_BLOCK_SIZE == keyLength)
    {
        return AES_XCBC_MAC_96_init( MOC_SYM( hwAccelCtx) keyMaterial, pCtx);
    }
    else if ( AES_BLOCK_SIZE > keyLength)
    {
        /* pad with zero */
        for (i = 0; i < keyLength; ++i)
        {
            key[i] = keyMaterial[i];
        }
        for (; i < AES_BLOCK_SIZE; ++i)
        {
            key[i] = 0;
        }
        return AES_XCBC_MAC_96_init( MOC_SYM( hwAccelCtx) key, pCtx);
    }
    else /* AES_BLOCK_SIZE < keyLenth */
    {
        MSTATUS status;

        for (i = 0; i < AES_BLOCK_SIZE; ++i)
        {
            key[i] = 0;
        }

        if (OK > (status = AES_XCBC_MAC_96_init( MOC_SYM( hwAccelCtx) key, pCtx)))
            return status;

        if ( OK > (status = AES_XCBC_PRF_128_update(MOC_SYM( hwAccelCtx) keyMaterial,
                                                    keyLength, pCtx)))
        {
            return status;
        }

        if ( OK > (status = AES_XCBC_PRF_128_final( MOC_SYM(hwAccelCtx) key, pCtx)))
            return status;

        return AES_XCBC_MAC_96_init( MOC_SYM( hwAccelCtx) key, pCtx);
    }
}


/*---------------------------------------------------------------------------*/

extern MSTATUS
AES_XCBC_MAC_96_update(MOC_SYM(hwAccelDescr hwAccelCtx) const ubyte* data,
                       sbyte4 dataLength, AES_XCBC_MAC_96_Ctx* pCtx)
{
    sbyte4 i;

    if ( !data || !  pCtx)
        return ERR_NULL_POINTER;

    if ( pCtx->pendingLen > 0 )
    {
        while ( pCtx->pendingLen < AES_BLOCK_SIZE && dataLength > 0)
        {
            pCtx->pending[ pCtx->pendingLen++] = *data++;
            dataLength--;
        }
        /* should we proceed ? only if full and more to do */
        if ( AES_BLOCK_SIZE == pCtx->pendingLen && dataLength > 0)
        {
            pCtx->pendingLen = 0;
            for (i = 0; i < AES_BLOCK_SIZE; ++i)
            {
                pCtx->pending[i] ^= pCtx->currBlock[i];
            }
            aesEncrypt(pCtx->aesCtx.rk, pCtx->aesCtx.Nr, pCtx->pending, pCtx->currBlock);
        }
    }

    /* process all the bytes if there are more than AES_BLOCK_SIZE */
    while ( dataLength > AES_BLOCK_SIZE)
    {
        for (i = 0; i < AES_BLOCK_SIZE; ++i, dataLength--)
        {
            pCtx->pending[i] = (pCtx->currBlock[i] ^ (*data++));
        }
        aesEncrypt( pCtx->aesCtx.rk, pCtx->aesCtx.Nr, pCtx->pending, pCtx->currBlock);
    }

    /* at this point, 1 < dataLength <= AES_BLOCK_SIZE */
    /* save the bytes that can't be processed in the pending array */
    while ( dataLength-- > 0)
    {
        pCtx->pending[pCtx->pendingLen++] = *data++;
    }
    return OK;
}



/*---------------------------------------------------------------------------*/

static MSTATUS
AES_XCBC_MAC_finalAux( MOC_SYM(hwAccelDescr hwAccelCtx)
                      ubyte cmac[/*cmacLen*/],
                      sbyte4 cmacLen,
                      AES_XCBC_MAC_96_Ctx* pCtx)
{
    sbyte4  i;
    ubyte   subKey[AES_BLOCK_SIZE];

    if (!cmac || !pCtx)
        return ERR_NULL_POINTER;

    /* which case are we in  */
    if ( AES_BLOCK_SIZE == pCtx->pendingLen)
    {
        /* multiple of block size -> use K2 */
        AES_XCBC_MAC_96_genKey( &pCtx->keyAesCtx, 2, subKey);
    }
    else
    {
        /* pad and use K3 */
        pCtx->pending[ pCtx->pendingLen++] = 0x80;
        while ( pCtx->pendingLen < AES_BLOCK_SIZE)
        {
            pCtx->pending[pCtx->pendingLen++] = 0;
        }
        AES_XCBC_MAC_96_genKey( &pCtx->keyAesCtx, 3, subKey);
    }

    for (i = 0; i < AES_BLOCK_SIZE; ++i)
    {
        pCtx->pending[i] ^= subKey[i];
        pCtx->pending[i] ^= pCtx->currBlock[i];
    }

    if ( AES_BLOCK_SIZE == cmacLen)
    {
        aesEncrypt(pCtx->aesCtx.rk, pCtx->aesCtx.Nr, pCtx->pending, cmac);
    }
    else
    {
        aesEncrypt(pCtx->aesCtx.rk, pCtx->aesCtx.Nr, pCtx->pending, subKey);

        /* truncate */
        memcpy(cmac, subKey, cmacLen);
    }

    return OK;
}


/*---------------------------------------------------------------------------*/

extern MSTATUS
AES_XCBC_MAC_96_final( MOC_SYM(hwAccelDescr hwAccelCtx)
                      ubyte cmac[AES_XCBC_MAC_96_RESULT_SIZE],
                      AES_XCBC_MAC_96_Ctx* pCtx)
{
    return AES_XCBC_MAC_finalAux( MOC_SYM(hwAccelCtx) cmac,
                                    AES_XCBC_MAC_96_RESULT_SIZE,
                                    pCtx);
}


/*---------------------------------------------------------------------------*/

extern MSTATUS
AES_XCBC_PRF_128_final( MOC_SYM(hwAccelDescr hwAccelCtx)
                      ubyte cmac[AES_XCBC_PRF_128_RESULT_SIZE],
                      AES_XCBC_MAC_96_Ctx* pCtx)
{
    return AES_XCBC_MAC_finalAux( MOC_SYM(hwAccelCtx) cmac,
                                    AES_XCBC_PRF_128_RESULT_SIZE,
                                    pCtx);
}


#endif /* (!defined(__DISABLE_AES_CIPHERS__) && !defined(__AES_HARDWARE_CIPHER__)) */
