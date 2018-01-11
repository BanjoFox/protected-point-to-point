/* Version: mss_v5_3_1_13561 */
/*
 * mtypes.h
 *
 * Mocana Types
 *
 * Copyright Mocana Corp 2003-2007. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */


/*------------------------------------------------------------------*/

#ifndef __MTYPES_HEADER__
#define __MTYPES_HEADER__


#ifndef __ENABLE_MOCANA_BASIC_TYPES_OVERRIDE__
typedef unsigned char       ubyte;
typedef unsigned short      ubyte2;
typedef unsigned int        ubyte4;

typedef signed char         sbyte;
typedef signed short        sbyte2;
typedef signed int          sbyte4;
#endif

/* gcc 2.95 does not define __LONG_LONG_MAX__ and __LONG_MAX__ as built-in
This code forces the use of long long for all GCC -- using limits.h does
not seem to work (__LONG_LONG_MAX__ is still undefined) */
#if defined (__GNUC__)

#if !defined(__LONG_LONG_MAX__)
#define __LONG_LONG_MAX__ 9223372036854775807LL
#endif

#if !defined(__LONG_MAX__)
#define __LONG_MAX__ 2147483647L
#endif

#endif /* __GNUC__ */

#if defined( __ENABLE_MOCANA_64_BIT__) || defined(__LP64__) || (defined(__LONG_LONG_MAX__) && __LONG_LONG_MAX__ > __LONG_MAX__ && !defined(__MOCANA_MAX_INT_32__) ) || (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 64)

#define __MOCANA_MAX_INT__ (64)

#if defined( __RTOS_WIN32__)
typedef unsigned __int64 ubyte8;
#else
typedef unsigned long long ubyte8;
#endif

typedef struct ubyte16
{
    ubyte8 upper64;
    ubyte8 lower64;
} ubyte16;


#else

#define __MOCANA_MAX_INT__ (32)

typedef struct
{
    ubyte4  upper32;
    ubyte4  lower32;

} ubyte8;

typedef struct ubyte16
{
    ubyte4 w1;
    ubyte4 w2;
    ubyte4 w3;
    ubyte4 w4;
} ubyte16;

#endif /* __ENABLE_MOCANA_64_BIT__ */

typedef sbyte               byteBoolean;
typedef sbyte4              intBoolean;
typedef void*               BulkCtx;


#ifdef __ENABLE_MOCANA_IPV6__
typedef struct moc_ipaddr
{
    ubyte2 family;          /* AF_INET or AF_INET6 */
    union
    {
        ubyte4 addr6[5];    /* IPv6 (128 bits); in network byte order
                               (with trailing scope id in host byte order) */
        ubyte4 addr;        /* IPv4 (32 bits); in host byte order */
    } uin;

} *MOC_IP_ADDRESS, MOC_IP_ADDRESS_S;
#else
#ifndef __ENABLE_MOCANA_NETWORK_TYPES_OVERRIDE__
typedef unsigned long       MOC_IP_ADDRESS;
#endif
#define MOC_IP_ADDRESS_S    MOC_IP_ADDRESS
#endif

#endif /* __MTYPES_HEADER__ */
