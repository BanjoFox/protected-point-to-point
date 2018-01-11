/* Version: mss_v5_3_1_13561 */
/*
 * mrtos.h
 *
 * Mocana RTOS Abstraction Layer
 *
 * Copyright Mocana Corp 2003-2008. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */


/*------------------------------------------------------------------*/

#ifndef __MRTOS_HEADER__
#define __MRTOS_HEADER__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __SYMBIAN32__
#include <stdlib.h>
#endif

#ifdef __MQX_RTOS__

#include <mqx.h>
#include <bsp.h>
#define MSEEK_END     IO_SEEK_END
#define MSEEK_SET     IO_SEEK_SET

#else

#define MSEEK_END    SEEK_END
#define MSEEK_SET    SEEK_SET

#endif

/* thread types */
enum threadTypes
{
    ENTROPY_THREAD,
    DEBUG_CONSOLE,
    SSL_UPCALL_DAEMON,
    SSH_UPCALL_DAEMON,
    SSL_MAIN,
    SSL_SERVER_SESSION,
    DTLS_MAIN,
    SSH_MAIN,
    SSH_SESSION,
    HTTP_THREAD,
    IKE_MAIN,
    DEBUG_THREAD,
    EAP_MAIN,
    IPC_MAIN,
    RADIUS_MAIN,
    HARNESS_MAIN,
    HARNESS_MAIN1,
    HARNESS_TEST,
    CLI_THREAD,
    MOC_IPV4,
    FIREWALL_MAIN,
    FIREWALL_SERVER,
    NTP_MAIN,
    OCSP_MAIN,
    CMP_MAIN,
    LDAP_MAIN,
    PKI_CLIENT_MAIN,
    PKI_IPC_MAIN,
    SRTP_MAIN
};

/* mutex types */
enum mutexTypes
{
    /* if updating this list, please be sure to make sure psos_rtos.c */
    /* and other oses are updated! ask James for details */
    SSL_CACHE_MUTEX,
    HW_ACCEL_CHANNEL_MUTEX,
    IPSEC_REASSEMBLY_MUTEX,
    IKE_MT_MUTEX,
    SSH_SERVER_MUTEX,
    TACACS_CLIENT_MUTEX,
    HARNESS_DRV_MUTEX,
    GUARDIAN_MUTEX,
    MEM_PART_MUTEX,
    EAP_INSTANCE_MUTEX,
    EAP_SESSION_MUTEX,
    FIREWALL_MUTEX,
    SRTP_CACHE_MUTEX,
    PKI_CLIENT_CACHE_MUTEX,
    END_MUTEX
};

/* this structure is such that dates can be easily compared with memcmp */
typedef struct TimeDate
{
    ubyte   m_year;     /* year 0 = 1970, 1 =1971 , etc...*/
    ubyte   m_month;    /* 1 = january, ... 12 = december */
    ubyte   m_day;      /* 1 - 31 */
    ubyte   m_hour;     /* 0 - 23 */
    ubyte   m_minute;   /* 0 - 59 */
    ubyte   m_second;   /* 0 - 59 */
} TimeDate;

/* Do not assume anything about the meaning of the fields of
  moctime_t: use the API functions only! */
typedef struct moctime_t
{
    union {
        ubyte4    time[2];
#if (defined(__LINUX_RTOS__) && defined(__KERNEL__))
        long long jiffies;              /* Linux kernel mode */
#endif

    } u;
} moctime_t;


typedef void*                       RTOS_MUTEX;
typedef void*                       RTOS_COND;
typedef void*                       RTOS_THREAD;

#if defined __SOLARIS_RTOS__
#define RTOS_rtosInit               SOLARIS_rtosInit
#define RTOS_rtosShutdown           SOLARIS_rtosShutdown
#define RTOS_mutexCreate            SOLARIS_mutexCreate
#define RTOS_mutexWait              SOLARIS_mutexWait
#define RTOS_mutexRelease           SOLARIS_mutexRelease
#define RTOS_mutexFree              SOLARIS_mutexFree
#define RTOS_getUpTimeInMS          SOLARIS_getUpTimeInMS
#define RTOS_deltaMS                SOLARIS_deltaMS
#define RTOS_sleepMS                SOLARIS_sleepMS
#define RTOS_deltaConstMS           SOLARIS_deltaConstMS
#define RTOS_timerAddMS             SOLARIS_timerAddMS
#define RTOS_createThread           SOLARIS_createThread
#define RTOS_destroyThread          SOLARIS_destroyThread
#define RTOS_currentThreadId        SOLARIS_currentThreadId
#define RTOS_timeGMT                SOLARIS_timeGMT

#elif defined (__LINUX_RTOS__ )
#define RTOS_rtosInit               LINUX_rtosInit
#define RTOS_rtosShutdown           LINUX_rtosShutdown
#define RTOS_mutexCreate            LINUX_mutexCreate
#define RTOS_mutexWait              LINUX_mutexWait
#define RTOS_mutexRelease           LINUX_mutexRelease
#define RTOS_mutexFree              LINUX_mutexFree
#define RTOS_condCreate             LINUX_condCreate
#define RTOS_condWait               LINUX_condWait
#define RTOS_condSignal             LINUX_condSignal
#define RTOS_condFree               LINUX_condFree
#define RTOS_getUpTimeInMS          LINUX_getUpTimeInMS
#define RTOS_deltaMS                LINUX_deltaMS
#define RTOS_deltaConstMS           LINUX_deltaConstMS
#define RTOS_timerAddMS             LINUX_timerAddMS
#define RTOS_sleepMS                LINUX_sleepMS
#define RTOS_createThread           LINUX_createThread
#define RTOS_destroyThread          LINUX_destroyThread
#define RTOS_currentThreadId        LINUX_currentThreadId
#define RTOS_timeGMT                LINUX_timeGMT

#define RTOS_recursiveMutexCreate   LINUX_recursiveMutexCreate
#define RTOS_recursiveMutexWait     LINUX_recursiveMutexWait
#define RTOS_recursiveMutexRelease  LINUX_recursiveMutexRelease
#define RTOS_recursiveMutexFree     LINUX_recursiveMutexFree

#elif defined (__SYMBIAN32__)
#define RTOS_rtosInit               SYMBIAN_rtosInit
#define RTOS_rtosShutdown           SYMBIAN_rtosShutdown
#define RTOS_mutexCreate            SYMBIAN_mutexCreate
#define RTOS_mutexWait              SYMBIAN_mutexWait
#define RTOS_mutexRelease           SYMBIAN_mutexRelease
#define RTOS_mutexFree              SYMBIAN_mutexFree
#define RTOS_condCreate             SYMBIAN_condCreate
#define RTOS_condWait               SYMBIAN_condWait
#define RTOS_condSignal             SYMBIAN_condSignal
#define RTOS_condFree               SYMBIAN_condFree
#define RTOS_getUpTimeInMS          SYMBIAN_getUpTimeInMS
#define RTOS_deltaMS                SYMBIAN_deltaMS
#define RTOS_deltaConstMS           SYMBIAN_deltaConstMS
#define RTOS_timerAddMS             SYMBIAN_timerAddMS
#define RTOS_sleepMS                SYMBIAN_sleepMS
#define RTOS_createThread           SYMBIAN_createThread
#define RTOS_destroyThread          SYMBIAN_destroyThread
#define RTOS_currentThreadId        SYMBIAN_currentThreadId
#define RTOS_timeGMT                SYMBIAN_timeGMT

#elif defined __WIN32_RTOS__
#define RTOS_rtosInit               WIN32_rtosInit
#define RTOS_rtosShutdown           WIN32_rtosShutdown
#define RTOS_mutexCreate            WIN32_mutexCreate
#define RTOS_mutexWait              WIN32_mutexWait
#define RTOS_mutexRelease           WIN32_mutexRelease
#define RTOS_mutexFree              WIN32_mutexFree
#define RTOS_getUpTimeInMS          WIN32_getUpTimeInMS
#define RTOS_deltaMS                WIN32_deltaMS
#define RTOS_deltaConstMS           WIN32_deltaConstMS
#define RTOS_timerAddMS             WIN32_timerAddMS
#define RTOS_sleepMS                WIN32_sleepMS
#define RTOS_createThread           WIN32_createThread
#define RTOS_destroyThread          WIN32_destroyThread
#define RTOS_currentThreadId        WIN32_currentThreadId
#define RTOS_timeGMT                WIN32_timeGMT

#elif defined __VXWORKS_RTOS__
#define RTOS_rtosInit               VXWORKS_rtosInit
#define RTOS_rtosShutdown           VXWORKS_rtosShutdown
#define RTOS_mutexCreate            VXWORKS_mutexCreate
#define RTOS_mutexWait              VXWORKS_mutexWait
#define RTOS_mutexRelease           VXWORKS_mutexRelease
#define RTOS_mutexFree              VXWORKS_mutexFree
#define RTOS_getUpTimeInMS          VXWORKS_getUpTimeInMS
#define RTOS_deltaMS                VXWORKS_deltaMS
#define RTOS_sleepMS                VXWORKS_sleepMS
#define RTOS_createThread           VXWORKS_createThread
#define RTOS_destroyThread          VXWORKS_destroyThread
#define RTOS_currentThreadId        VXWORKS_currentThreadId
#define RTOS_timeGMT                VXWORKS_timeGMT

#define RTOS_recursiveMutexCreate   VXWORKS_recursiveMutexCreate
#define RTOS_recursiveMutexWait     VXWORKS_recursiveMutexWait
#define RTOS_recursiveMutexRelease  VXWORKS_recursiveMutexRelease
#define RTOS_recursiveMutexFree     VXWORKS_recursiveMutexFree


#elif defined __NNOS_RTOS__
#define RTOS_rtosInit               NNOS_rtosInit
#define RTOS_rtosShutdown           NNOS_rtosShutdown
#define RTOS_mutexCreate            NNOS_mutexCreate
#define RTOS_mutexWait              NNOS_mutexWait
#define RTOS_mutexRelease           NNOS_mutexRelease
#define RTOS_mutexFree              NNOS_mutexFree
#define RTOS_getUpTimeInMS          NNOS_getUpTimeInMS
#define RTOS_deltaMS                NNOS_deltaMS
#define RTOS_sleepMS                NNOS_sleepMS
#define RTOS_createThread           NNOS_createThread
#define RTOS_destroyThread          NNOS_destroyThread
#define RTOS_currentThreadId        NNOS_currentThreadId
#define RTOS_timeGMT                NNOS_timeGMT

#elif defined __PSOS_RTOS__
#define RTOS_rtosInit               PSOS_rtosInit
#define RTOS_rtosShutdown           PSOS_rtosShutdown
#define RTOS_mutexCreate            PSOS_mutexCreate
#define RTOS_mutexWait              PSOS_mutexWait
#define RTOS_mutexRelease           PSOS_mutexRelease
#define RTOS_mutexFree              PSOS_mutexFree
#define RTOS_getUpTimeInMS          PSOS_getUpTimeInMS
#define RTOS_deltaMS                PSOS_deltaMS
#define RTOS_sleepMS                PSOS_sleepMS
#define RTOS_createThread           PSOS_createThread
#define RTOS_destroyThread          PSOS_destroyThread
#define RTOS_currentThreadId        PSOS_currentThreadId
#define RTOS_timeGMT                PSOS_timeGMT

#elif defined __NUCLEUS_RTOS__
#define RTOS_rtosInit               NUCLEUS_rtosInit
#define RTOS_rtosShutdown           NUCLEUS_rtosShutdown
#define RTOS_mutexCreate            NUCLEUS_mutexCreate
#define RTOS_mutexWait              NUCLEUS_mutexWait
#define RTOS_mutexRelease           NUCLEUS_mutexRelease
#define RTOS_mutexFree              NUCLEUS_mutexFree
#define RTOS_getUpTimeInMS          NUCLEUS_getUpTimeInMS
#define RTOS_deltaMS                NUCLEUS_deltaMS
#define RTOS_sleepMS                NUCLEUS_sleepMS
#define RTOS_createThread           NUCLEUS_createThread
#define RTOS_destroyThread          NUCLEUS_destroyThread
#define RTOS_currentThreadId        NUCLEUS_currentThreadId
#define RTOS_timeGMT                NUCLEUS_timeGMT

#elif defined __CYGWIN_RTOS__
#define RTOS_rtosInit               CYGWIN_rtosInit
#define RTOS_rtosShutdown           CYGWIN_rtosShutdown
#define RTOS_mutexCreate            CYGWIN_mutexCreate
#define RTOS_mutexWait              CYGWIN_mutexWait
#define RTOS_mutexRelease           CYGWIN_mutexRelease
#define RTOS_mutexFree              CYGWIN_mutexFree
#define RTOS_getUpTimeInMS          CYGWIN_getUpTimeInMS
#define RTOS_deltaMS                CYGWIN_deltaMS
#define RTOS_deltaConstMS           CYGWIN_deltaConstMS
#define RTOS_timerAddMS             CYGWIN_timerAddMS
#define RTOS_sleepMS                CYGWIN_sleepMS
#define RTOS_createThread           CYGWIN_createThread
#define RTOS_destroyThread          CYGWIN_destroyThread
#define RTOS_currentThreadId        CYGWIN_currentThreadId
#define RTOS_timeGMT                CYGWIN_timeGMT
#define RTOS_condCreate             CYGWIN_condCreate
#define RTOS_condWait               CYGWIN_condWait
#define RTOS_condSignal             CYGWIN_condSignal
#define RTOS_condFree               CYGWIN_condFree

#define RTOS_recursiveMutexCreate   CYGWIN_mutexCreate
#define RTOS_recursiveMutexWait     CYGWIN_mutexWait
#define RTOS_recursiveMutexRelease  CYGWIN_mutexRelease
#define RTOS_recursiveMutexFree     CYGWIN_mutexFree

#elif defined __OSX_RTOS__
#define RTOS_rtosInit               OSX_rtosInit
#define RTOS_rtosShutdown           OSX_rtosShutdown
#define RTOS_mutexCreate            OSX_mutexCreate
#define RTOS_mutexWait              OSX_mutexWait
#define RTOS_mutexRelease           OSX_mutexRelease
#define RTOS_mutexFree              OSX_mutexFree
#define RTOS_getUpTimeInMS          OSX_getUpTimeInMS
#define RTOS_deltaMS                OSX_deltaMS
#define RTOS_sleepMS                OSX_sleepMS
#define RTOS_createThread           OSX_createThread
#define RTOS_destroyThread          OSX_destroyThread
#define RTOS_currentThreadId        OSX_currentThreadId
#define RTOS_timeGMT                OSX_timeGMT

#elif defined __THREADX_RTOS__
#define RTOS_rtosInit               THREADX_rtosInit
#define RTOS_rtosShutdown           THREADX_rtosShutdown
#define RTOS_mutexCreate            THREADX_mutexCreate
#define RTOS_mutexWait              THREADX_mutexWait
#define RTOS_mutexRelease           THREADX_mutexRelease
#define RTOS_mutexFree              THREADX_mutexFree
#define RTOS_getUpTimeInMS          THREADX_getUpTimeInMS
#define RTOS_deltaMS                THREADX_deltaMS
#define RTOS_sleepMS                THREADX_sleepMS
#define RTOS_createThread           THREADX_createThread
#define RTOS_destroyThread          THREADX_destroyThread
#define RTOS_currentThreadId        THREADX_currentThreadId
#define RTOS_timeGMT                THREADX_timeGMT
#define RTOS_malloc                 THREADX_malloc
#define RTOS_free                   THREADX_free
#define RTOS_mallocAlign16          THREADX_malloc_Align16
#define RTOS_freeAlign16            THREADX_freeAlign16

#elif defined __POSNET_RTOS__
#define RTOS_rtosInit               POSNET_rtosInit
#define RTOS_rtosShutdown           POSNET_rtosShutdown
#define RTOS_mutexCreate            POSNET_mutexCreate
#define RTOS_mutexWait              POSNET_mutexWait
#define RTOS_mutexRelease           POSNET_mutexRelease
#define RTOS_mutexFree              POSNET_mutexFree
#define RTOS_getUpTimeInMS          POSNET_getUpTimeInMS
#define RTOS_deltaMS                POSNET_deltaMS
#define RTOS_sleepMS                POSNET_sleepMS
#define RTOS_createThread           POSNET_createThread
#define RTOS_destroyThread          POSNET_destroyThread
#define RTOS_currentThreadId        POSNET_currentThreadId
#define RTOS_timeGMT                POSNET_timeGMT

#elif defined __OSE_RTOS__
#define RTOS_rtosInit               OSE_rtosInit
#define RTOS_rtosShutdown           OSE_rtosShutdown
#define RTOS_mutexCreate            OSE_mutexCreate
#define RTOS_mutexWait              OSE_mutexWait
#define RTOS_mutexRelease           OSE_mutexRelease
#define RTOS_mutexFree              OSE_mutexFree
#define RTOS_getUpTimeInMS          OSE_getUpTimeInMS
#define RTOS_deltaMS                OSE_deltaMS
#define RTOS_sleepMS                OSE_sleepMS
#define RTOS_createThread           OSE_createThread
#define RTOS_destroyThread          OSE_destroyThread
#define RTOS_currentThreadId        OSE_currentThreadId
#define RTOS_timeGMT                OSE_timeGMT

#elif defined __MQX_RTOS__
#define RTOS_rtosInit               MQX_rtosInit
#define RTOS_rtosShutdown           MQX_rtosShutdown
#define RTOS_mutexCreate            MQX_mutexCreate
#define RTOS_mutexWait              MQX_mutexWait
#define RTOS_mutexRelease           MQX_mutexRelease
#define RTOS_mutexFree              MQX_mutexFree
#define RTOS_getUpTimeInMS          MQX_getUpTimeInMS
#define RTOS_deltaMS                MQX_deltaMS
#define RTOS_sleepMS                MQX_sleepMS
#define RTOS_createThread           MQX_createThread
#define RTOS_destroyThread          MQX_destroyThread
#define RTOS_currentThreadId        MQX_currentThreadId
#define RTOS_timeGMT                MQX_timeGMT
#define RTOS_malloc                 MQX_malloc
#define RTOS_free                   MQX_free

#elif defined __NETBURNER_RTOS__
#define RTOS_rtosInit               NETBURNER_rtosInit
#define RTOS_rtosShutdown           NETBURNER_rtosShutdown
#define RTOS_mutexCreate            NETBURNER_mutexCreate
#define RTOS_mutexWait              NETBURNER_mutexWait
#define RTOS_mutexRelease           NETBURNER_mutexRelease
#define RTOS_mutexFree              NETBURNER_mutexFree
#define RTOS_getUpTimeInMS          NETBURNER_getUpTimeInMS
#define RTOS_deltaMS                NETBURNER_deltaMS
#define RTOS_sleepMS                NETBURNER_sleepMS
#define RTOS_createThread           NETBURNER_createThread
#define RTOS_destroyThread          NETBURNER_destroyThread
#define RTOS_currentThreadId        NETBURNER_currentThreadId
#define RTOS_timeGMT                NETBURNER_timeGMT

#elif defined __OPENBSD_RTOS__
#define RTOS_rtosInit               OPENBSD_rtosInit
#define RTOS_rtosShutdown           OPENBSD_rtosShutdown
#define RTOS_mutexCreate            OPENBSD_mutexCreate
#define RTOS_mutexWait              OPENBSD_mutexWait
#define RTOS_mutexRelease           OPENBSD_mutexRelease
#define RTOS_mutexFree              OPENBSD_mutexFree
#define RTOS_getUpTimeInMS          OPENBSD_getUpTimeInMS
#define RTOS_deltaMS                OPENBSD_deltaMS
#define RTOS_deltaConstMS           OPENBSD_deltaConstMS
#define RTOS_timerAddMS             OPENBSD_timerAddMS
#define RTOS_sleepMS                OPENBSD_sleepMS
#define RTOS_createThread           OPENBSD_createThread
#define RTOS_destroyThread          OPENBSD_destroyThread
#define RTOS_currentThreadId        OPENBSD_currentThreadId
#define RTOS_timeGMT                OPENBSD_timeGMT

#elif defined __NUTOS_RTOS__
#define RTOS_rtosInit               NUTOS_rtosInit
#define RTOS_rtosShutdown           NUTOS_rtosShutdown
#define RTOS_mutexCreate            NUTOS_mutexCreate
#define RTOS_mutexWait              NUTOS_mutexWait
#define RTOS_mutexRelease           NUTOS_mutexRelease
#define RTOS_mutexFree              NUTOS_mutexFree
#define RTOS_getUpTimeInMS          NUTOS_getUpTimeInMS
#define RTOS_deltaMS                NUTOS_deltaMS
#define RTOS_sleepMS                NUTOS_sleepMS
#define RTOS_createThread           NUTOS_createThread
#define RTOS_destroyThread          NUTOS_destroyThread
#define RTOS_currentThreadId        NUTOS_currentThreadId
#define RTOS_timeGMT                NUTOS_timeGMT

#elif defined __INTEGRITY_RTOS__
#define RTOS_rtosInit               INTEGRITY_rtosInit
#define RTOS_rtosShutdown           INTEGRITY_rtosShutdown
#define RTOS_mutexCreate            INTEGRITY_mutexCreate
#define RTOS_mutexWait              INTEGRITY_mutexWait
#define RTOS_mutexRelease           INTEGRITY_mutexRelease
#define RTOS_mutexFree              INTEGRITY_mutexFree
#define RTOS_getUpTimeInMS          INTEGRITY_getUpTimeInMS
#define RTOS_deltaMS                INTEGRITY_deltaMS
#define RTOS_sleepMS                INTEGRITY_sleepMS
#define RTOS_createThread           INTEGRITY_createThread
#define RTOS_destroyThread          INTEGRITY_destroyThread
#define RTOS_currentThreadId        INTEGRITY_currentThreadId
#define RTOS_timeGMT                INTEGRITY_timeGMT

#elif defined __UITRON_RTOS__
#define RTOS_rtosInit               UITRON_rtosInit
#define RTOS_rtosShutdown           UITRON_rtosShutdown
#define RTOS_mutexCreate            UITRON_mutexCreate
#define RTOS_mutexWait              UITRON_mutexWait
#define RTOS_mutexRelease           UITRON_mutexRelease
#define RTOS_mutexFree              UITRON_mutexFree
#define RTOS_getUpTimeInMS          UITRON_getUpTimeInMS
#define RTOS_deltaMS                UITRON_deltaMS
#define RTOS_sleepMS                UITRON_sleepMS
#define RTOS_createThread           UITRON_createThread
#define RTOS_destroyThread          UITRON_destroyThread
#define RTOS_currentThreadId        UITRON_currentThreadId
#define RTOS_timeGMT                UITRON_timeGMT

#elif defined __ANDROID_RTOS__
#define RTOS_rtosInit               ANDROID_rtosInit
#define RTOS_rtosShutdown           ANDROID_rtosShutdown
#define RTOS_mutexCreate            ANDROID_mutexCreate
#define RTOS_mutexWait              ANDROID_mutexWait
#define RTOS_mutexRelease           ANDROID_mutexRelease
#define RTOS_mutexFree              ANDROID_mutexFree
#define RTOS_condCreate             ANDROID_condCreate
#define RTOS_condWait               ANDROID_condWait
#define RTOS_condSignal             ANDROID_condSignal
#define RTOS_condFree               ANDROID_condFree
#define RTOS_getUpTimeInMS          ANDROID_getUpTimeInMS
#define RTOS_deltaMS                ANDROID_deltaMS
#define RTOS_deltaConstMS           ANDROID_deltaConstMS
#define RTOS_timerAddMS             ANDROID_timerAddMS
#define RTOS_sleepMS                ANDROID_sleepMS
#define RTOS_createThread           ANDROID_createThread
#define RTOS_destroyThread          ANDROID_destroyThread
#define RTOS_currentThreadId        ANDROID_currentThreadId
#define RTOS_timeGMT                ANDROID_timeGMT

#define RTOS_recursiveMutexCreate   ANDROID_recursiveMutexCreate
#define RTOS_recursiveMutexWait     ANDROID_recursiveMutexWait
#define RTOS_recursiveMutexRelease  ANDROID_recursiveMutexRelease
#define RTOS_recursiveMutexFree     ANDROID_recursiveMutexFree

#elif defined __FREEBSD_RTOS__
#define RTOS_rtosInit               FREEBSD_rtosInit
#define RTOS_rtosShutdown           FREEBSD_rtosShutdown
#define RTOS_mutexCreate            FREEBSD_mutexCreate
#define RTOS_mutexWait              FREEBSD_mutexWait
#define RTOS_mutexRelease           FREEBSD_mutexRelease
#define RTOS_mutexFree              FREEBSD_mutexFree
#define RTOS_getUpTimeInMS          FREEBSD_getUpTimeInMS
#define RTOS_deltaMS                FREEBSD_deltaMS
#define RTOS_deltaConstMS           FREEBSD_deltaConstMS
#define RTOS_timerAddMS             FREEBSD_timerAddMS
#define RTOS_sleepMS                FREEBSD_sleepMS
#define RTOS_createThread           FREEBSD_createThread
#define RTOS_destroyThread          FREEBSD_destroyThread
#define RTOS_currentThreadId        FREEBSD_currentThreadId
#define RTOS_timeGMT                FREEBSD_timeGMT

#elif defined __IRIX_RTOS__
#define RTOS_rtosInit               IRIX_rtosInit
#define RTOS_rtosShutdown           IRIX_rtosShutdown
#define RTOS_mutexCreate            IRIX_mutexCreate
#define RTOS_mutexWait              IRIX_mutexWait
#define RTOS_mutexRelease           IRIX_mutexRelease
#define RTOS_mutexFree              IRIX_mutexFree
#define RTOS_getUpTimeInMS          IRIX_getUpTimeInMS
#define RTOS_deltaMS                IRIX_deltaMS
#define RTOS_deltaConstMS           IRIX_deltaConstMS
#define RTOS_timerAddMS             IRIX_timerAddMS
#define RTOS_sleepMS                IRIX_sleepMS
#define RTOS_createThread           IRIX_createThread
#define RTOS_destroyThread          IRIX_destroyThread
#define RTOS_currentThreadId        IRIX_currentThreadId
#define RTOS_timeGMT                IRIX_timeGMT

#elif defined __QNX_RTOS__
#define RTOS_rtosInit               QNX_rtosInit
#define RTOS_rtosShutdown           QNX_rtosShutdown
#define RTOS_mutexCreate            QNX_mutexCreate
#define RTOS_mutexWait              QNX_mutexWait
#define RTOS_mutexRelease           QNX_mutexRelease
#define RTOS_mutexFree              QNX_mutexFree
#define RTOS_getUpTimeInMS          QNX_getUpTimeInMS
#define RTOS_deltaMS                QNX_deltaMS
#define RTOS_deltaConstMS           QNX_deltaConstMS
#define RTOS_timerAddMS             QNX_timerAddMS
#define RTOS_sleepMS                QNX_sleepMS
#define RTOS_createThread           QNX_createThread
#define RTOS_destroyThread          QNX_destroyThread
#define RTOS_currentThreadId        QNX_currentThreadId
#define RTOS_timeGMT                QNX_timeGMT
#define RTOS_malloc                 QNX_malloc
#define RTOS_free                   QNX_free

#elif defined __UITRON_RTOS__
#define RTOS_rtosInit               UITRON_rtosInit
#define RTOS_rtosShutdown           UITRON_rtosShutdown
#define RTOS_mutexCreate            UITRON_mutexCreate
#define RTOS_mutexWait              UITRON_mutexWait
#define RTOS_mutexRelease           UITRON_mutexRelease
#define RTOS_mutexFree              UITRON_mutexFree
#define RTOS_getUpTimeInMS          UITRON_getUpTimeInMS
#define RTOS_deltaMS                UITRON_deltaMS
#define RTOS_deltaConstMS           UITRON_deltaConstMS
#define RTOS_timerAddMS             UITRON_timerAddMS
#define RTOS_sleepMS                UITRON_sleepMS
#define RTOS_createThread           UITRON_createThread
#define RTOS_destroyThread          UITRON_destroyThread
#define RTOS_currentThreadId        UITRON_currentThreadId
#define RTOS_timeGMT                UITRON_timeGMT

#elif defined __WINCE_RTOS__
#define RTOS_rtosInit               WINCE_rtosInit
#define RTOS_rtosShutdown           WINCE_rtosShutdown
#define RTOS_mutexCreate            WINCE_mutexCreate
#define RTOS_mutexWait              WINCE_mutexWait
#define RTOS_mutexRelease           WINCE_mutexRelease
#define RTOS_mutexFree              WINCE_mutexFree
#define RTOS_getUpTimeInMS          WINCE_getUpTimeInMS
#define RTOS_deltaMS                WINCE_deltaMS
#define RTOS_deltaConstMS           WINCE_deltaConstMS
#define RTOS_timerAddMS             WINCE_timerAddMS
#define RTOS_sleepMS                WINCE_sleepMS
#define RTOS_createThread           WINCE_createThread
#define RTOS_destroyThread          WINCE_destroyThread
#define RTOS_currentThreadId        WINCE_currentThreadId
#define RTOS_timeGMT                WINCE_timeGMT

#elif defined __WTOS_RTOS__
#define RTOS_rtosInit               WTOS_rtosInit
#define RTOS_rtosShutdown           WTOS_rtosShutdown
#define RTOS_mutexCreate            WTOS_mutexCreate
#define RTOS_mutexWait              WTOS_mutexWait
#define RTOS_mutexRelease           WTOS_mutexRelease
#define RTOS_mutexFree              WTOS_mutexFree
#define RTOS_getUpTimeInMS          WTOS_getUpTimeInMS
#define RTOS_deltaMS                WTOS_deltaMS
#define RTOS_sleepMS                WTOS_sleepMS
#define RTOS_createThread           WTOS_createThread
#define RTOS_destroyThread          WTOS_destroyThread
#define RTOS_currentThreadId        WTOS_currentThreadId
#define RTOS_timeGMT

#elif defined __ECOS_RTOS__
#define RTOS_rtosInit               ECOS_rtosInit
#define RTOS_rtosShutdown           ECOS_rtosShutdown
#define RTOS_mutexCreate            ECOS_mutexCreate
#define RTOS_mutexWait              ECOS_mutexWait
#define RTOS_mutexRelease           ECOS_mutexRelease
#define RTOS_mutexFree              ECOS_mutexFree
#define RTOS_getUpTimeInMS          ECOS_getUpTimeInMS
#define RTOS_deltaMS                ECOS_deltaMS
#define RTOS_deltaConstMS           ECOS_deltaConstMS
#define RTOS_timerAddMS             ECOS_timerAddMS
#define RTOS_sleepMS                ECOS_sleepMS
#define RTOS_createThread           ECOS_createThread
#define RTOS_destroyThread          ECOS_destroyThread
#define RTOS_currentThreadId        ECOS_currentThreadId
#define RTOS_timeGMT                ECOS_timeGMT

#elif defined __FREERTOS_RTOS__
#define RTOS_rtosInit               FREERTOS_rtosInit
#define RTOS_rtosShutdown           FREERTOS_rtosShutdown
#define RTOS_mutexCreate            FREERTOS_mutexCreate
#define RTOS_mutexWait              FREERTOS_mutexWait
#define RTOS_mutexRelease           FREERTOS_mutexRelease
#define RTOS_mutexFree              FREERTOS_mutexFree
#define RTOS_getUpTimeInMS          FREERTOS_getUpTimeInMS
#define RTOS_deltaMS                FREERTOS_deltaMS
#define RTOS_deltaConstMS           FREERTOS_deltaConstMS
#define RTOS_timerAddMS             FREERTOS_timerAddMS
#define RTOS_sleepMS                FREERTOS_sleepMS
#define RTOS_createThread           FREERTOS_createThread
#define RTOS_destroyThread          FREERTOS_destroyThread
#define RTOS_currentThreadId        FREERTOS_currentThreadId
#define RTOS_timeGMT                FREERTOS_timeGMT

#elif defined (__AIX_RTOS__ )
#define RTOS_rtosInit               AIX_rtosInit
#define RTOS_rtosShutdown           AIX_rtosShutdown
#define RTOS_mutexCreate            AIX_mutexCreate
#define RTOS_mutexWait              AIX_mutexWait
#define RTOS_mutexRelease           AIX_mutexRelease
#define RTOS_mutexFree              AIX_mutexFree
#define RTOS_getUpTimeInMS          AIX_getUpTimeInMS
#define RTOS_deltaMS                AIX_deltaMS
#define RTOS_deltaConstMS           AIX_deltaConstMS
#define RTOS_timerAddMS             AIX_timerAddMS
#define RTOS_sleepMS                AIX_sleepMS
#define RTOS_createThread           AIX_createThread
#define RTOS_destroyThread          AIX_destroyThread
#define RTOS_currentThreadId        AIX_currentThreadId
#define RTOS_timeGMT                AIX_timeGMT

#elif defined (__HPUX_RTOS__ )
#define RTOS_rtosInit               HPUX_rtosInit
#define RTOS_rtosShutdown           HPUX_rtosShutdown
#define RTOS_mutexCreate            HPUX_mutexCreate
#define RTOS_mutexWait              HPUX_mutexWait
#define RTOS_mutexRelease           HPUX_mutexRelease
#define RTOS_mutexFree              HPUX_mutexFree
#define RTOS_getUpTimeInMS          HPUX_getUpTimeInMS
#define RTOS_deltaMS                HPUX_deltaMS
#define RTOS_deltaConstMS           HPUX_deltaConstMS
#define RTOS_timerAddMS             HPUX_timerAddMS
#define RTOS_sleepMS                HPUX_sleepMS
#define RTOS_createThread           HPUX_createThread
#define RTOS_destroyThread          HPUX_destroyThread
#define RTOS_currentThreadId        HPUX_currentThreadId
#define RTOS_timeGMT                HPUX_timeGMT

#elif defined (__QUADROS_RTOS__)
#define RTOS_rtosInit               QUADROS_rtosInit
#define RTOS_rtosShutdown           QUADROS_rtosShutdown
#define RTOS_mutexCreate            QUADROS_mutexCreate
#define RTOS_mutexWait              QUADROS_mutexWait
#define RTOS_mutexRelease           QUADROS_mutexRelease
#define RTOS_mutexFree              QUADROS_mutexFree
#define RTOS_getUpTimeInMS          QUADROS_getUpTimeInMS
#define RTOS_deltaMS                QUADROS_deltaMS
#define RTOS_deltaConstMS           QUADROS_deltaConstMS
#define RTOS_timerAddMS             QUADROS_timerAddMS
#define RTOS_sleepMS                QUADROS_sleepMS
#define RTOS_createThread           QUADROS_createThread
#define RTOS_destroyThread          QUADROS_destroyThread
#define RTOS_currentThreadId        QUADROS_currentThreadId
#define RTOS_timeGMT                QUADROS_timeGMT

#elif defined (__UCOS_RTOS__)
#define RTOS_rtosInit               UCOS_rtosInit
#define RTOS_rtosShutdown           UCOS_rtosShutdown
#define RTOS_mutexCreate            UCOS_mutexCreate
#define RTOS_mutexWait              UCOS_mutexWait
#define RTOS_mutexRelease           UCOS_mutexRelease
#define RTOS_mutexFree              UCOS_mutexFree
#define RTOS_getUpTimeInMS          UCOS_getUpTimeInMS
#define RTOS_deltaMS                UCOS_deltaMS
#define RTOS_deltaConstMS           UCOS_deltaConstMS
#define RTOS_timerAddMS             UCOS_timerAddMS
#define RTOS_sleepMS                UCOS_sleepMS
#define RTOS_createThread           UCOS_createThread
#define RTOS_destroyThread          UCOS_destroyThread
#define RTOS_currentThreadId        UCOS_currentThreadId
#define RTOS_timeGMT                UCOS_timeGMT

#elif defined __DUMMY_RTOS__
#define RTOS_rtosInit               DUMMY_rtosInit
#define RTOS_rtosShutdown           DUMMY_rtosShutdown
#define RTOS_mutexCreate            DUMMY_mutexCreate
#define RTOS_mutexWait              DUMMY_mutexWait
#define RTOS_mutexRelease           DUMMY_mutexRelease
#define RTOS_mutexFree              DUMMY_mutexFree
#define RTOS_getUpTimeInMS          DUMMY_getUpTimeInMS
#define RTOS_deltaMS                DUMMY_deltaMS
#define RTOS_sleepMS                DUMMY_sleepMS
#define RTOS_createThread           DUMMY_createThread
#define RTOS_destroyThread          DUMMY_destroyThread
#define RTOS_currentThreadId        DUMMY_currentThreadId
#define RTOS_timeGMT                DUMMY_timeGMT
#define RTOS_deltaConstMS           DUMMY_deltaConstMS
#define RTOS_timerAddMS             DUMMY_timerAddMS

#elif defined (__ENABLE_MOCANA_SEC_BOOT__)

/* Nanoboot does not require any OS */

#else

#error UNSUPPORTED PLATFORM

#endif


MOC_EXTERN MSTATUS      RTOS_rtosInit               (void);
MOC_EXTERN MSTATUS      RTOS_rtosShutdown           (void);
MOC_EXTERN MSTATUS      RTOS_mutexCreate            (RTOS_MUTEX* pMutex, enum mutexTypes mutexType, int mutexCount);
MOC_EXTERN MSTATUS      RTOS_mutexWait              (RTOS_MUTEX mutex);
MOC_EXTERN MSTATUS      RTOS_mutexRelease           (RTOS_MUTEX mutex);
MOC_EXTERN MSTATUS      RTOS_mutexFree              (RTOS_MUTEX* pMutex);
MOC_EXTERN MSTATUS      RTOS_condCreate             (RTOS_COND* pcond, enum mutexTypes mutexType, int mutexCount);
MOC_EXTERN MSTATUS      RTOS_condWait               (RTOS_COND  cond,RTOS_MUTEX mutex);
MOC_EXTERN MSTATUS      RTOS_condSignal             (RTOS_COND mutex);
MOC_EXTERN MSTATUS      RTOS_condFree               (RTOS_MUTEX* pCond);
MOC_EXTERN ubyte4       RTOS_getUpTimeInMS          (void);
MOC_EXTERN ubyte4       RTOS_deltaMS                (const moctime_t *pPrevTime, moctime_t *pRetCurrentTime);
MOC_EXTERN ubyte4       RTOS_deltaConstMS           (const moctime_t* origin, const moctime_t* current);
MOC_EXTERN moctime_t*   RTOS_timerAddMS             (moctime_t* pTimer, ubyte4 addNumMS);
MOC_EXTERN void         RTOS_sleepMS                (ubyte4 sleepTimeInMS);
MOC_EXTERN MSTATUS      RTOS_createThread           (void(*threadEntry)(void*), void* context, ubyte4 threadType, RTOS_THREAD *pRetTid);
MOC_EXTERN void         RTOS_destroyThread          (RTOS_THREAD tid);
MOC_EXTERN MSTATUS      RTOS_timeGMT                (TimeDate* td);

MOC_EXTERN MSTATUS      RTOS_recursiveMutexCreate   (RTOS_MUTEX* pMutex, enum mutexTypes mutexType, int mutexCount);
MOC_EXTERN MSTATUS      RTOS_recursiveMutexWait     (RTOS_MUTEX mutex);
MOC_EXTERN MSTATUS      RTOS_recursiveMutexRelease  (RTOS_MUTEX mutex);
MOC_EXTERN MSTATUS      RTOS_recursiveMutexFree     (RTOS_MUTEX* pMutex);

#ifdef RTOS_malloc
MOC_EXTERN void*        RTOS_malloc                 (ubyte4 bufsize);
#endif
#ifdef RTOS_free
MOC_EXTERN void         RTOS_free                   (void *p);
#endif

/* see mrtos.c: easy to use wrappers for RTOS_mutexWait() and RTOS_mutexRelease() */
MOC_EXTERN MSTATUS MRTOS_mutexWait(RTOS_MUTEX mutex, intBoolean *pIsMutexSet);
MOC_EXTERN MSTATUS MRTOS_mutexRelease(RTOS_MUTEX mutex, intBoolean *pIsMutexSet);

#ifdef __ENABLE_MOCANA_DEBUG_MEMORY__

#define MALLOC(X)       dbg_malloc(X, (ubyte *)__FILE__, __LINE__)
#define FREE(X)         dbg_free((void *)(X), (ubyte *)__FILE__, __LINE__)

MOC_EXTERN void  dbg_dump(void);
MOC_EXTERN void* dbg_malloc(ubyte4 numBytes, ubyte *pFile, ubyte4 lineNum);
MOC_EXTERN void  dbg_free(void *pBlockToFree, ubyte *pFile, ubyte4 lineNum);

#else

#ifndef MALLOC
#ifdef RTOS_malloc
#define MALLOC                      RTOS_malloc
#else
#define MALLOC                      malloc
#endif
#endif

#ifndef FREE
#ifdef RTOS_free
#define FREE                        RTOS_free
#else
#define FREE                        free
#endif
#endif

/* To prevent warnings with GNU or errors with MSVC++
   (with this compiler, we enforce that functions must be
   declared before being used), make sure we include
   stdlib.h */
#if defined(__KERNEL__) && defined(__LINUX_RTOS__)
    #include <linux/types.h>
    void *malloc(size_t size);
    void free(void *ptr);
#elif defined(__ENABLE_MOCANA_IPSEC_SERVICE__) && defined(__WIN32_RTOS__)
    MOC_EXTERN void* malloc(unsigned int);
    MOC_EXTERN void free(void *);
#elif defined(__MOC_IPV4_STACK__) || defined(__QNX_RTOS__)

#elif defined(__GNUC__) || defined(_MSC_VER)
    #include <stdlib.h>
#endif

#endif /* __ENABLE_MOCANA_DEBUG_MEMORY__ */


#ifdef __cplusplus
}
#endif

#endif /* __MRTOS_HEADER__ */

