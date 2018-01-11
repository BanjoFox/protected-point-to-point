/* Version: mss_v5_3_1_13561 */
/*
 * mstdlib.h
 *
 * Mocana Standard Library
 *
 * Copyright Mocana Corp 2003-2007. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */


/*------------------------------------------------------------------*/

#ifndef __MSTDLIB_HEADER__
#define __MSTDLIB_HEADER__

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(MOC_LITTLE_ENDIAN) && defined(MOC_BIG_ENDIAN))
#error Must not define both MOC_LITTLE_ENDIAN and MOC_BIG_ENDIAN
#endif

#if (defined(__ENABLE_MOCANA_EAP_AUTH__) || defined(__ENABLE_MOCANA_EAP_PEER__))
#if defined(MOC_LITTLE_ENDIAN)
#define NTOHS(A) SWAPWORD(A)
#define HTONS(A) SWAPWORD(A)
#define NTOHL(A) SWAPDWORD(A)
#define HTONL(A) SWAPDWORD(A)
#elif defined(MOC_BIG_ENDIAN)
#define NTOHS(A) (A)
#define HTONS(A) (A)
#define NTOHL(A) (A)
#define HTONL(A) (A)
#else
#error Must define either MOC_LITTLE_ENDIAN or MOC_BIG_ENDIAN in moptions.h
#endif
#endif

#define BIGEND32( BUFF, X)         {  ubyte* _tmp = (ubyte*) (BUFF);     \
                                   *_tmp++ = (ubyte)((X) >> 24);   \
                                   *_tmp++ = (ubyte)((X) >> 16);   \
                                   *_tmp++ = (ubyte)((X)>>  8);   \
                                   *_tmp++ = (ubyte)(X);  }

/* compute the new length so that x mod p = 0; p must be a power of 2 */
#define MOC_PAD(x,p)                ((x+p-1) & (~(p-1)))

#if defined(__ASM_386_GCC__)
#define ROTATE_LEFT(X,n)  \
    ({ ubyte4 _R = X; __asm __volatile ("rol %1, %0": "=r"(_R) : "i"(n), "0"(_R)); _R;})
#define ROTATE_RIGHT(X,n) \
({ ubyte4 _R = X; __asm __volatile ("ror %1, %0": "=r"(_R) : "i"(n), "0"(_R)); _R;})
#elif defined(__ASM_PPC__)
 /* newer versions of the gcc compiler do that optimization already! */
#define ROTATE_LEFT(X,n) \
( {ubyte4 _R; __asm __volatile("rotlwi %0, %1, %2": "=r"(_R): "r"(X), "i"(n)); _R; })
#define ROTATE_RIGHT(X,n) \
( {ubyte4 _R; __asm __volatile("rotrwi %0, %1, %2": "=r"(_R): "r"(X), "i"(n)); _R; })
#elif defined(__xxASM_ARM__)
/* don't enable this for ARM since gcc is able to wrap the rotation operation
   as part of some other instruction... like add r1, r2, r7, ror #27 */
#define ROTATE_LEFT(X,n) \
( {ubyte4 _R; __asm __volatile("mov %0, %1, ror %2": "=r"(_R): "r"(X), "i"(32-n)); _R; })
#define ROTATE_RIGHT(X,n) \
( {ubyte4 _R; __asm __volatile("mov %0, %1, ror %2": "=r"(_R): "r"(X), "i"(n)); _R; })
#else
    /* MIPS does not have rotate instructions */
/* rotate X left n bits */
#define ROTATE_LEFT(X,n)        (((ubyte4)(X) << (n)) | (((ubyte4)X) >> (32-(n))))
#define ROTATE_RIGHT(X,n)       (((ubyte4)(X) >> (n)) | (((ubyte4)(X)) << (32-(n))))
#endif

/*------------------------------------------------------------------*/

MOC_EXTERN ubyte2 SWAPWORD(ubyte2 a);
MOC_EXTERN ubyte4 SWAPDWORD(ubyte4 a);

MOC_EXTERN ubyte4  MOC_NTOHL(const ubyte *v);
MOC_EXTERN ubyte2  MOC_NTOHS(const ubyte *v);
MOC_EXTERN void    MOC_HTONL(ubyte n[4], ubyte4 h);
MOC_EXTERN void    MOC_HTONS(ubyte n[2], ubyte2 h);

/* REVIEW -- prototypes should use void* */
MOC_EXTERN MSTATUS MOC_MEMMOVE(ubyte *pDest, const ubyte *pSrc, sbyte4 len);
MOC_EXTERN MSTATUS MOC_MEMCPY(void *pDest, const void *pSrc, sbyte4 len);
MOC_EXTERN MSTATUS MOC_MEMCMP(const ubyte *pSrc1, const ubyte *pSrc2, ubyte4 len, sbyte4 *pResult);
MOC_EXTERN MSTATUS MOC_MEMSET(ubyte *pDest, ubyte value, sbyte4 len);

MOC_EXTERN ubyte   returnHexDigit(ubyte4 digit);
MOC_EXTERN sbyte   MTOLOWER(sbyte c);
MOC_EXTERN byteBoolean   MOC_ISSPACE( sbyte c);
MOC_EXTERN byteBoolean   MOC_ISLWS( sbyte c);
MOC_EXTERN byteBoolean   MOC_ISXDIGIT( sbyte c);
MOC_EXTERN byteBoolean   MOC_ISLOWER( sbyte c);
MOC_EXTERN byteBoolean   MOC_ISDIGIT( sbyte c);
MOC_EXTERN byteBoolean   MOC_ISASCII( sbyte c);

MOC_EXTERN sbyte4  MOC_STRCMP(const sbyte *pString1, const sbyte *pString2);
MOC_EXTERN sbyte4  MOC_STRNICMP(const sbyte *pString1, const sbyte* pString2, ubyte4 n);
MOC_EXTERN ubyte4  MOC_STRLEN(const sbyte *s);
MOC_EXTERN ubyte4  MOC_STRCBCPY( sbyte* dest, ubyte4 destSize, const sbyte* src);
MOC_EXTERN ubyte4  MOC_BITLENGTH( ubyte4 w);

MOC_EXTERN MSTATUS MOC_alloc(ubyte4 size, void **ppRetAllocBuf);
MOC_EXTERN MSTATUS MOC_free(void **ppFreeAllocBuf);

/* convert a string to a integer using decimal base, the stop pointer
    can be null. If it is not, it is set to the character that can't be
    interpreted as a decimal digit */
MOC_EXTERN sbyte4  MOC_ATOL(const sbyte *s, const sbyte **stop);

/* write a integer to a string using decimal base. Returns the
    position in the buffer where the value was written or NULL
    if there was not enough space in the buffer == the buffer is
    NOT NUL terminated */
MOC_EXTERN sbyte* MOC_LTOA(sbyte4 value, sbyte *buff, ubyte4 bufSize);

/* 0 = Sunday, ..... 6 = Saturday */
MOC_EXTERN sbyte4 MOC_DAYOFWEEK( sbyte4 day, sbyte4 month /*1-12*/, sbyte4 year /* 1752- */);

MOC_EXTERN MSTATUS MOC_MALLOC(void **ppPtr, ubyte4 bufSize);
MOC_EXTERN MSTATUS MOC_FREE(void **ppPtr);

MOC_EXTERN MSTATUS MOC_UTOA(ubyte4 value, ubyte *pRetResult, ubyte4 *pRetNumDigitsLong);
MOC_EXTERN ubyte4  MOC_floorPower2(ubyte4 value);

#ifdef __cplusplus
}
#endif

#endif /* __MSTDLIB_HEADER__ */

