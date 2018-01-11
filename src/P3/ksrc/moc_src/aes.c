/* Version: mss_v5_3_1_13561 */
/*
 * aes.c
 *
 * AES Implementation
 *
 * Copyright Mocana Corp 2003-2007. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */


/*------------------------------------------------------------------*/

#include "../p3kbase.h"
#include "moptions.h"
#include "mtypes.h"
#include "mocana.h"
#include "hw_accel.h"

#if (!defined(__DISABLE_AES_CIPHERS__) && !defined(__AES_HARDWARE_CIPHER__))

#include "mdefs.h"
#include "merrors.h"
#include "mrtos.h"
#include "mstdlib.h"
#include "debug_console.h"
#include "aesalgo.h"
#include "aes.h"
#ifdef __ENABLE_MOCANA_FIPS_MODULE__
#include "fips.h"
#endif

#ifdef __ZEROIZE_TEST__
#include <stdio.h>   /* for printf */
#endif

/*------------------------------------------------------------------*/

extern BulkCtx
CreateAESCtx(MOC_SYM(hwAccelDescr hwAccelCtx) ubyte* keyMaterial, sbyte4 keyLength, sbyte4 encrypt)
{
    aesCipherContext* ctx = NULL;

    ctx = p3malloc(sizeof(aesCipherContext));

    if (NULL != ctx)
    {
        memset((ubyte *)ctx, 0x00, sizeof(aesCipherContext));

        if (OK > AESALGO_makeAesKey(ctx, 8 * keyLength, keyMaterial, encrypt, MODE_CBC))
        {
            p3free(ctx);  ctx = NULL;
        }
    }

    return ctx;
}


/*------------------------------------------------------------------*/

extern MSTATUS
DeleteAESCtx(MOC_SYM(hwAccelDescr hwAccelCtx) BulkCtx ctx)
{
    if (ctx)
    {
        /* Zeroize the sensitive information before deleting the memory */
        memset(ctx, 0x00, sizeof(aesCipherContext));
        p3free(ctx);
        ctx = NULL;
    }

    return OK;
}


/*------------------------------------------------------------------*/

extern MSTATUS
DoAES(MOC_SYM(hwAccelDescr hwAccelCtx) BulkCtx ctx, ubyte* data, sbyte4 dataLength, sbyte4 encrypt, ubyte* iv)
{
    sbyte4              retLength;
    aesCipherContext*   pAesContext = (aesCipherContext *)ctx;
    MSTATUS             status;

    if (0 != (dataLength % AES_BLOCK_SIZE))
    {
        status = ERR_AES_BAD_LENGTH;
		char emsg[64];
		sprintf(emsg, "Bad AES length: %d\n", dataLength);
		p3errmsg(p3MSG_ERR, emsg);
        goto exit;
    }

    if (encrypt)
        status = AESALGO_blockEncrypt(pAesContext, iv, data, 8 * dataLength, data, &retLength);
    else
        status = AESALGO_blockDecrypt(pAesContext, iv, data, 8 * dataLength, data, &retLength);

#ifdef __ENABLE_ALL_DEBUGGING__
    if (OK > status)
        DEBUG_ERROR(DEBUG_SSL_TRANSPORT, "DoAES: cipher failed, error = ", status);
#endif

exit:
    return status;
}

#endif /* (!defined(__DISABLE_AES_CIPHERS__) && !defined(__AES_HARDWARE_CIPHER__)) */

