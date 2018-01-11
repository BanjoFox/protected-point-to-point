/* Version: mss_v5_3_1_13561 */
/*
 * debug_console.h
 *
 * Mocana Debug Console Header
 *
 * Copyright Mocana Corp 2003-2007. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */

#ifndef __DEBUG_CONSOLE_HEADER__
#define __DEBUG_CONSOLE_HEADER__

#define DEBUG_MEMORY        (1 <<  0)
#define DEBUG_COMMON        (1 <<  1)
#define DEBUG_CRYPTO        (1 <<  2)
#define DEBUG_PLATFORM      (1 <<  3)
#define DEBUG_SSH_MESSAGES  (1 <<  4)
#define DEBUG_SSL_MESSAGES  (1 <<  5)
#define DEBUG_FIREWALL      (1 <<  6)
#define DEBUG_IKE_MESSAGES  (1 <<  7)
#define DEBUG_IKE_EXAMPLE   (1 <<  8)
#define DEBUG_EAP_MESSAGE   (1 <<  9)
#define DEBUG_HTTP_MESSAGE  (1 << 10)
#define DEBUG_HARNESS       (1 << 11)
#define DEBUG_RADIUS        (1 << 12)
#define DEBUG_SSHC          (1 << 13)
#define DEBUG_ID_MESSAGE    (1 << 14)
#define DEBUG_HASH_MESSAGE  (1 << 15)
#define DEBUG_TIMER_MESSAGE (1 << 16)
#define DEBUG_SCEP_EXAMPLE  (1 << 17)
#define DEBUG_IPC_MESSAGE   (1 << 18)
#define DEBUG_IPSEC         (1 << 19)
#define DEBUG_MOC_IPV4      (1 << 20)
#define DEBUG_CLI_MESSAGE   (1 << 21)
#define DEBUG_ERROR_FLAG    (1 << 22)
#define DEBUG_TEST          (1 << 23)
#define DEBUG_PFKEY_MESSAGE (1 << 24)
#define DEBUG_PFKEY_EXAMPLE (1 << 25)
#define DEBUG_SRTP_MESSAGES (1 << 26)
#define DEBUG_LDAP_MESSAGES (1 << 27)
#define DEBUG_CUSTOM        (1 << 31)

/* collapse some since we ran out */
#define DEBUG_SSH_TRANSPORT DEBUG_SSH_MESSAGES
#define DEBUG_SSH_AUTH      DEBUG_SSH_MESSAGES
#define DEBUG_SSH_SFTP      DEBUG_SSH_MESSAGES
#define DEBUG_SSH_SERVICE   DEBUG_SSH_MESSAGES
#define DEBUG_SSH_EXAMPLE   DEBUG_SSH_MESSAGES

#define DEBUG_SSL_TRANSPORT DEBUG_SSL_MESSAGES
#define DEBUG_SSL_EXAMPLE   DEBUG_SSL_MESSAGES

#define DEBUG_DTLS_EXAMPLE  DEBUG_SSL_MESSAGES

#define DEBUG_EAP_EXAMPLE   DEBUG_EAP_MESSAGE

#define DEBUG_CFGMON        DEBUG_CLI_MESSAGE

#ifdef  __ENABLE_MOCANA_DEBUG_CONSOLE__

MOC_EXTERN sbyte4     m_errorClass;

#ifndef __ENABLE_CUSTOM_DEBUG_CONSOLE_DEFS__
#define DEBUG_PRINT(X,Y)         DEBUG_CONSOLE_printString(X,Y)
#define DEBUG_INT(X,Y)           DEBUG_CONSOLE_printInteger(X,Y)
#define DEBUG_INT2(X,Y,Z)        DEBUG_CONSOLE_printInteger2(X,Y,Z)
#define DEBUG_INT3(P,Q,R,S)      DEBUG_CONSOLE_printInteger3(P,Q,R,S)
#define DEBUG_INT4(V,W,X,Y,Z)    DEBUG_CONSOLE_printInteger4(V,W,X,Y,Z)
#define DEBUG_UINT(X,Y)          DEBUG_CONSOLE_printUnsignedInteger(X,Y)
#define DEBUG_UINT2(X,Y,Z)       DEBUG_CONSOLE_printUnsignedInteger2(X,Y,Z)
#define DEBUG_UINT3(W,X,Y,Z)     DEBUG_CONSOLE_printUnsignedInteger3(W,X,Y,Z)
#define DEBUG_UINT4(V,W,X,Y,Z)   DEBUG_CONSOLE_printUnsignedInteger4(V,W,X,Y,Z)
#define DEBUG_PRINTNL(X,Y)       DEBUG_CONSOLE_printNewLine(X,Y)
#define DEBUG_UPTIME(X)          DEBUG_CONSOLE_printUpTime(X)
#define DEBUG_ERROR(X,Y,Z)       DEBUG_CONSOLE_printError(X,Y,Z)
#define DEBUG_PRINT3(W,X,Y,Z)    DEBUG_CONSOLE_print3(W,X,Y,Z)
#define DEBUG_HEXBYTE(X,Y)       DEBUG_CONSOLE_hexByte(X,Y)
#define DEBUG_HEXINT(X,Y)        DEBUG_CONSOLE_hexInt(X,Y)
#define DEBUG_HEXINT2(X,Y,Z)     DEBUG_CONSOLE_hexInt2(X,Y,Z)
#define DEBUG_HEXINT3(W,X,Y,Z)   DEBUG_CONSOLE_hexInt3(W,X,Y,Z)
#define DEBUG_HEXINT4(V,W,X,Y,Z) DEBUG_CONSOLE_hexInt4(V,W,X,Y,Z)
#define DEBUG_PRINTBYTE(X,Y)     DEBUG_CONSOLE_printByte(X,Y)
#define DEBUG_HEXDUMP(X,Y,Z)     DEBUG_CONSOLE_hexDump(X,Y,Z)
#define DEBUG_ASCIPADDR (X,Y)    DEBUG_CONSOLE_printAsciiIPAddr(X,Y)
#define DEBUG_PRINTSTR2INT2(V,W,X,Y,Z) DEBUG_CONSOLE_printString2Int2(V,W,X,Y,Z)
#define DEBUG_PRINTSTR2HEXINT2(V,W,X,Y,Z) DEBUG_CONSOLE_printString2HexInt2(V,W,X,Y,Z)
#define DEBUG_PRINTSTR1INT1(X,Y,Z) DEBUG_CONSOLE_printString1Int1(X,Y,Z)
#define DEBUG_PRINTSTR1HEXINT1(X,Y,Z) DEBUG_CONSOLE_printString1HexInt1(X,Y,Z)
#define DEBUG_PRINTSTR1ASCIPADDR1(X,Y,Z) DEBUG_CONSOLE_printString1AsciiIPAddr(X,Y,Z)
#define DEBUG_PRINT2(X,Y,Z) DEBUG_CONSOLE_printString2(X,Y,Z)
#endif
#else
#define DEBUG_PRINT(X,Y)
#define DEBUG_INT(X,Y)
#define DEBUG_UINT(X,Y)
#define DEBUG_PRINTNL(X,Y)
#define DEBUG_UPTIME(X)
#define DEBUG_ERROR(X,Y,Z)
#define DEBUG_PRINT3(W,X,Y,Z)
#define DEBUG_HEXBYTE(X,Y)
#define DEBUG_HEXINT(X,Y)
#define DEBUG_PRINTBYTE(X,Y)
#define DEBUG_HEXDUMP(X,Y,Z)
#define DEBUG_ASCIPADDR(X,Y)
#define DEBUG_PRINTSTR2INT2(V,W,X,Y,Z)
#define DEBUG_PRINTSTR2HEXINT2(V,W,X,Y,Z)
#define DEBUG_PRINTSTR1INT1(X,Y,Z)
#define DEBUG_PRINTSTR1HEXINT1(X,Y,Z)
#define DEBUG_PRINTSTR1ASCIPADDR1(X,Y,Z)
#define DEBUG_PRINT2(X,Y,Z)
#endif /* __ENABLE_MOCANA_DEBUG_CONSOLE__ */

#ifndef __ENABLE_MOCANA_DEBUG_CONSOLE__
  #define DB_PRINT
  #define DBUG_PRINT(ec, expr)
  #define ERROR_PRINT(expr)

  #define DUMP_BYTES(a,b,c,d)
  #define DUMP_SHORTS(a,b,c,d)
  #define DUMP_LONGS(a,b,c,d)
#else
  #define DB_PRINT(...)  DEBUG_CONSOLE_printf(__VA_ARGS__)
  #define DBUG_PRINT(ec, expr)  \
        if (ec == (m_errorClass & ec))  {\
            DB_PRINT("%s[%d]: ", __FUNCTION__, __LINE__);\
            DB_PRINT expr ;\
            DB_PRINT("\n") ;\
        } \
        else
  #define ERROR_PRINT(expr)  DBUG_PRINT(DEBUG_ERROR_FLAG, expr)

  #define DUMP_BYTES(a,b,c,d)   DEBUG_CONSOLE_dump_data(a,b,c,1,d)
  #define DUMP_SHORTS(a,b,c,d)  DEBUG_CONSOLE_dump_data(a,b,c,2,d)
  #define DUMP_LONGS(a,b,c,d)   DEBUG_CONSOLE_dump_data(a,b,c,4,d)

#endif

/*------------------------------------------------------------------*/

#ifdef __ENABLE_MOCANA_DEBUG_CONSOLE__
MOC_EXTERN void DEBUG_CONSOLE_init(void);
MOC_EXTERN sbyte4  DEBUG_CONSOLE_start(ubyte2 listenPort);
MOC_EXTERN void DEBUG_CONSOLE_stop(void);

MOC_EXTERN void DEBUG_CONSOLE_printString(sbyte4 errorClass, sbyte *pPrintString);
MOC_EXTERN void DEBUG_CONSOLE_printInteger(sbyte4 errorClass, sbyte4 value);
MOC_EXTERN void DEBUG_CONSOLE_printInteger2(sbyte4 errorClass, sbyte4 value1, sbyte4 value2);
MOC_EXTERN void DEBUG_CONSOLE_printInteger3(sbyte4 errorClass, sbyte4 value1, sbyte4 value2, sbyte4 value3);
MOC_EXTERN void DEBUG_CONSOLE_printInteger4(sbyte4 errorClass, sbyte4 value1, sbyte4 value2, sbyte4 value3, sbyte4 value4);
MOC_EXTERN void DEBUG_CONSOLE_printUnsignedInteger(sbyte4 errorClass, sbyte4 value);
MOC_EXTERN void DEBUG_CONSOLE_printUnsignedInteger2(sbyte4 errorClass, sbyte4 value1, sbyte4 value2);
MOC_EXTERN void DEBUG_CONSOLE_printUnsignedInteger3(sbyte4 errorClass, sbyte4 value1, sbyte4 value2, sbyte4 value3);
MOC_EXTERN void DEBUG_CONSOLE_printUnsignedInteger4(sbyte4 errorClass, sbyte4 value1, sbyte4 value2, sbyte4 value3, sbyte4 value4);
MOC_EXTERN void DEBUG_CONSOLE_printNewLine(sbyte4 errorClass, sbyte *pPrintString);
MOC_EXTERN void DEBUG_CONSOLE_printUpTime(sbyte4 errorClass);
MOC_EXTERN void DEBUG_CONSOLE_printError(sbyte4 errorClass, sbyte *pPrintString, sbyte4 value);
MOC_EXTERN void DEBUG_CONSOLE_print3(sbyte4 errorClass, sbyte *pPrintString1, sbyte *pPrintString2, sbyte *pPrintString3);
MOC_EXTERN void DEBUG_CONSOLE_hexByte(sbyte4 errorClass, sbyte value);
MOC_EXTERN void DEBUG_CONSOLE_hexInt(sbyte4 errorClass, sbyte4 value);
MOC_EXTERN void DEBUG_CONSOLE_hexInt2(sbyte4 errorClass, sbyte4 value1, sbyte4 value2);
MOC_EXTERN void DEBUG_CONSOLE_hexInt3(sbyte4 errorClass, sbyte4 value1, sbyte4 value2, sbyte4 value3);
MOC_EXTERN void DEBUG_CONSOLE_hexInt4(sbyte4 errorClass, sbyte4 value1, sbyte4 value2, sbyte4 value3, sbyte4 value4);
MOC_EXTERN void DEBUG_CONSOLE_printByte(sbyte4 errorClass, sbyte value);
MOC_EXTERN void DEBUG_CONSOLE_hexDump(sbyte4 errorClass, ubyte *pMesg, ubyte4 mesgLen);
MOC_EXTERN void DEBUG_CONSOLE_dump_data(ubyte *address, int size, int limit, int wsize, char *title);
MOC_EXTERN void    DEBUG_CONSOLE_printf(char *format, ...);
MOC_EXTERN void DEBUG_CONSOLE_asciiIPAddr(sbyte4 errorClass, sbyte4 value);
MOC_EXTERN void
DEBUG_CONSOLE_printString1Int1(sbyte4 errorClass, sbyte *pPrintString1, sbyte4 value1);
MOC_EXTERN void
DEBUG_CONSOLE_printString1HexInt1(sbyte4 errorClass, sbyte *pPrintString1, sbyte4 value1);
MOC_EXTERN void
DEBUG_CONSOLE_printString2Int2(sbyte4 errorClass, sbyte *pPrintString1, sbyte4 value1, sbyte *pPrintString2, sbyte4 value2);
MOC_EXTERN void
DEBUG_CONSOLE_printString2HexInt2(sbyte4 errorClass, sbyte *pPrintString1, sbyte4 value1, sbyte *pPrintString2, sbyte4 value2);
MOC_EXTERN void
DEBUG_CONSOLE_printString1AsciiIPAddr(sbyte4 errorClass,sbyte *pPrintString1, sbyte4 ipValue);
MOC_EXTERN void
DEBUG_CONSOLE_printString2(sbyte4 errorClass, sbyte *pPrintString1, sbyte *pPrintString2);
#ifndef __KERNEL__
MOC_EXTERN sbyte4  DEBUG_CONSOLE_setOutput(char *filename);
#endif

#endif /* __ENABLE_MOCANA_DEBUG_CONSOLE__ */

#endif /* __DEBUG_CONSOLE_HEADER__ */

