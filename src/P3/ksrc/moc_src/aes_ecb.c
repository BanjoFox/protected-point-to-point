/* Version: mss_v5_3_1_13561 */
/*
 * aes_ecb.c
 *
 * AES Implementation
 *
 * Copyright Mocana Corp 2009. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */


/*------------------------------------------------------------------*/

#include "../common/moptions.h"
#include "../common/mtypes.h"
#include "../common/mocana.h"
#include "../crypto/hw_accel.h"

#if (!defined(__DISABLE_AES_CIPHERS__) && !defined(__AES_HARDWARE_CIPHER__))

#include "../common/mdefs.h"
#include "../common/merrors.h"
#include "../common/mrtos.h"
#include "../common/mstdlib.h"
#include "../common/debug_console.h"
#include "../crypto/aesalgo.h"
#include "../crypto/aes.h"
#include "../crypto/aes_ecb.h"
#ifdef __ENABLE_MOCANA_FIPS_MODULE__
#include "../crypto/fips.h"
#endif

#ifdef __ZEROIZE_TEST__
#include <stdio.h>   /* for printf */
#endif

/*------------------------------------------------------------------*/

extern BulkCtx
CreateAESECBCtx(MOC_SYM(hwAccelDescr hwAccelCtx) ubyte* keyMaterial, sbyte4 keyLength, sbyte4 encrypt)
{
    aesCipherContext* ctx = NULL;

#ifdef __ENABLE_MOCANA_FIPS_MODULE__
    if (OK != gFIPS_powerupStatus)
        return NULL;
#endif /* __ENABLE_MOCANA_FIPS_MODULE__ */

    ctx = MALLOC(sizeof(aesCipherContext));

    if (NULL != ctx)
    {
        memset((ubyte *)ctx, 0x00, sizeof(aesCipherContext));

        if (OK > AESALGO_makeAesKey(ctx, 8 * keyLength, keyMaterial, encrypt, MODE_ECB))
        {
            FREE(ctx);  ctx = NULL;
        }
    }

    return ctx;
}


/*------------------------------------------------------------------*/

extern MSTATUS
DeleteAESECBCtx(MOC_SYM(hwAccelDescr hwAccelCtx) BulkCtx* ctx)
{
#ifdef __ENABLE_MOCANA_FIPS_MODULE__
   if (OK != gFIPS_powerupStatus)
        return gFIPS_powerupStatus;
#endif /* __ENABLE_MOCANA_FIPS_MODULE__ */
    if (*ctx)
    {
#ifdef __ZEROIZE_TEST__
        int counter = 0;
        printf("\nAES - Before Zeroization\n");
        for( counter = 0; counter < sizeof(aesCipherContext); counter++)
        {
            printf("%02x",*((ubyte*)*ctx+counter));
        }
#endif
        /* Zeroize the sensitive information before deleting the memory */
        memset((ubyte*)*ctx,0x00,sizeof(aesCipherContext));
#ifdef __ZEROIZE_TEST__
        printf("\nAES - After Zeroization\n");
        for( counter = 0; counter < sizeof(aesCipherContext); counter++)
        {
            printf("%02x",*((ubyte*)*ctx+counter));
        }
#endif
        FREE(*ctx);
        *ctx = NULL;
    }

    return OK;
}


/*------------------------------------------------------------------*/

extern MSTATUS
DoAESECB(MOC_SYM(hwAccelDescr hwAccelCtx) BulkCtx ctx, ubyte* data, sbyte4 dataLength, sbyte4 encrypt, ubyte* iv)
{
    sbyte4              retLength;
    aesCipherContext*   pAesContext = (aesCipherContext *)ctx;
    MSTATUS             status;

#ifdef __ENABLE_MOCANA_FIPS_MODULE__
   if (OK != gFIPS_powerupStatus)
        return gFIPS_powerupStatus;
#endif /* __ENABLE_MOCANA_FIPS_MODULE__ */

    if (0 != (dataLength % AES_BLOCK_SIZE))
    {
        status = ERR_AES_BAD_LENGTH;
        goto exit;
    }

    if (encrypt)
        status = AESALGO_blockEncrypt(pAesContext, iv, data, 8 * dataLength, data, &retLength);
    else
        status = AESALGO_blockDecrypt(pAesContext, iv, data, 8 * dataLength, data, &retLength);

#ifdef __ENABLE_ALL_DEBUGGING__
    if (OK > status)
        DEBUG_ERROR(DEBUG_SSL_TRANSPORT, "DoAESECB: cipher failed, error = ", status);
#endif

exit:
    return status;
}

#endif /* (!defined(__DISABLE_AES_CIPHERS__) && !defined(__AES_HARDWARE_CIPHER__)) */

