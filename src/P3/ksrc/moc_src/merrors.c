/* Version: mss_v5_3_1_13561 */
/*
 * merrors.c
 *
 * Mocana Error Lookup
 *
 * Copyright Mocana Corp 2003-2007. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */

/*! Mocana error look up.
This file contains functions enabling Mocana error code look up.

! External Functions
This file contains the following public ($extern$) functions:
- $MERROR_lookUpErrorCode$

*/

#define __INTERNAL_MERRORS__

#include "moptions.h"
#include "mdefs.h"
#include "mtypes.h"

/* setup for enum list */
#ifdef   __ERROR_LOOKUP_TABLE__
#undef   __ERROR_LOOKUP_TABLE__
#endif
#include "merrors.h"

/* setup for lookup table */
#define  __ERROR_LOOKUP_TABLE__
#undef   __MERRORS_HEADER__
#undef   ERROR_DEF
#undef   ERROR_DEF_LAST

#include "merrors.h"


/*------------------------------------------------------------------*/

#define NUM_ERROR_CODES     (sizeof(m_ErrorLookupTable) / sizeof(errDescr))

extern const ubyte *
MERROR_lookUpErrorCode(MSTATUS errorCode)
{
    ubyte* pErrorMesg = NULL;
    ubyte4 index;

    for (index = 0; index < NUM_ERROR_CODES; index++)
    {
        if (m_ErrorLookupTable[index].errorCode == errorCode)
        {
            pErrorMesg = m_ErrorLookupTable[index].pErrorMesg;
            break;
        }
    }

    return pErrorMesg;
}
