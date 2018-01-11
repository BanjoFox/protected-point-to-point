/* Version: mss_v5_3_1_13561 */

/*
 * moptions.h
 *
 * Mocana Option Definitions used by the testing framework
 *
 * Copyright Mocana Corp 2003-2009. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */


/*------------------------------------------------------------------*/

#ifndef __MOPTIONS_HEADER__

#define __MOPTIONS_HEADER__

/* @CODEGEN-START */
/* @CODEGEN-END   */

// #define __ENABLE_MOCANA_EXAMPLES__
// #define __ENABLE_MOCANA_SSL_SERVER__ 
// #define __ENABLE_MOCANA_SSH_SERVER__ 
// #define __ENABLE_MOCANA_SSL_CLIENT__ 
// #define __ENABLE_MOCANA_IPSEC_SERVICE__ 
// #define __ENABLE_MOCANA_IKE_SERVER__ 
// #define __ENABLE_MOCANA_RADIUS_CLIENT__ 
// #define __ENABLE_MOCANA_SSH_CLIENT__ 
// #define __ENABLE_MOCANA_EAP_AUTH__ 
// #define __ENABLE_MOCANA_EAP_PEER__ 
// #define __ENABLE_MOCANA_HTTP_CLIENT__ 
// #define __ENABLE_MOCANA_HTTPCC_SERVER__ 
// #define __ENABLE_MOCANA_SCEP_CLIENT__ 
// #define __ENABLE_MOCANA_SCEPCC_SERVER__ 
// #define __MOC_IPV4_STACK__ 
// #define __ENABLE_MOCANA_SSLCC_CLIENT__ 
// #define __ENABLE_MOCANA_SSLCC_SERVER__ 
// #define __ENABLE_MOCANA_HARNESS__ 
// #define __ENABLE_MOCANA_CLI__ 
// #define __ENABLE_MOCANA_UMP__ 
// #define __ENABLE_MOCANA_SEC_BOOT__ 
// #define __ENABLE_MOCANA_SIGN_BOOT__ 
// #define __ENABLE_MOCANA_WPA2__
// #define __ENABLE_MOCANA_NTP_CLIENT__
// #define __ENABLE_MOCANA_SRTP__
// #define __ENABLE_MOCANA_LDAP_CLIENT__
// #define __ENABLE_MOCANA_OCSP_CLIENT__
// #define __ENABLE_MOCANA_SRTP__
// #define __ENABLE_MOCANA_FIPS_MODULE__
// #define __ENABLE_MOCANA_CMP_CLIENT__
// #define __ENABLE_MOCANA_PKI_CLIENT__

#if (defined( __RTOS_VXWORKS__ ) && defined( __MOC_IPV4_STACK__ ))
#define __VXWORKS_RTOS__
#define __VXWORKS_TCP__
#define __VXWORKS_UDP__
#define __MOCANA_TCP__
#define __MOCANA_UDP__
#define __MOCANA_IPSTACK__
#elif defined( __RTOS_VXWORKS__ )
#define __VXWORKS_RTOS__
#define __VXWORKS_TCP__
#define __VXWORKS_UDP__
#elif defined( __RTOS_SOLARIS__ )
#define __SOLARIS_RTOS__
#define __SOLARIS_TCP__
#define __SOLARIS_UDP__
#elif (defined( __RTOS_LINUX__ ) && defined( __MOC_IPV4_STACK__ ))
#define __LINUX_RTOS__
#define __LINUX_TCP__
#define __LINUX_UDP__
#define __MOCANA_TCP__
#define __MOCANA_UDP__
#define __MOCANA_IPSTACK__
#elif defined( __RTOS_LINUX__ )
#define __LINUX_RTOS__
#define __LINUX_TCP__
#define __LINUX_UDP__
#elif defined( __RTOS_WIN32__ )
#define __WIN32_RTOS__
#define __WIN32_TCP__
#define __WIN32_UDP__
#elif defined( __RTOS_NNOS__ )
#define __NNOS_RTOS__
#define __NNOS_TCP__
#define __NNOS_UDP__
#elif defined( __RTOS_PSOS__ )
#define __PSOS_RTOS__
#define __PSOS_TCP__
#define __PSOS_UDP__
#elif defined( __RTOS_NUCLEUS__ )
#define __NUCLEUS_RTOS__
#define __NUCLEUS_TCP__
#define __NUCLEUS_UDP__
#elif defined( __RTOS_MQX__ )
#define __MQX_RTOS__
#define __RTCS_TCP__
#define __RTCS_UDP__
#elif defined( __RTOS_ARC__ )
#define __MQX_RTOS__
#define __RTCS_TCP__
#define __RTCS_UDP__
#elif defined( __RTOS_CYGWIN__ ) && defined( __MOC_IPV4_STACK__ )
#define __CYGWIN_RTOS__
#define __CYGWIN_TCP__
#define __CYGWIN_UDP__
#define __MOCANA_TCP__
#define __MOCANA_UDP__
#define __MOCANA_IPSTACK__
#elif defined( __RTOS_CYGWIN__ )
#define __CYGWIN_RTOS__
#define __CYGWIN_TCP__
#define __CYGWIN_UDP__
#elif defined( __RTOS_OSX__ )
#define __OSX_RTOS__
#define __OSX_TCP__
#define __OSX_UDP__
#elif defined( __RTOS_THREADX__ )
#define __THREADX_RTOS__
#define __THREADX_TCP__
#define __THREADX_UDP__
#elif defined( __RTOS_OSE__ )
#define __OSE_RTOS__
#define __OSE_TCP__
#define __OSE_UDP__
#elif defined( __RTOS_NETBURNER__ )
#define __NETBURNER_RTOS__
#define __NETBURNER_TCP__
#define __NETBURNER_UDP__
#elif defined( __RTOS_OPENBSD__ )
#define __OPENBSD_RTOS__
#define __OPENBSD_TCP__
#define __OPENBSD_UDP__
#elif defined( __RTOS_NUTOS__ )
#define __NUTOS_RTOS__
#define __NUTOS_TCP__
#define __NUTOS_UDP__
#elif defined( __RTOS_INTEGRITY__ )
#define __INTEGRITY_RTOS__
#define __INTEGRITY_TCP__
#define __INTEGRITY_UDP__
#elif (defined( __RTOS_ANDROID__ ) && defined( __MOC_IPV4_STACK__ ))
#define __ANDROID_RTOS__
#define __ANDROID_TCP__
#define __ANDROID_UDP__
#define __MOCANA_TCP__
#define __MOCANA_UDP__
#define __MOCANA_IPSTACK__
#elif defined( __RTOS_ANDROID__ )
#define __ANDROID_RTOS__
#define __ANDROID_TCP__
#define __ANDROID_UDP__
#elif defined( __RTOS_FREEBSD__ )
#define __FREEBSD_RTOS__
#define __FREEBSD_TCP__
#define __FREEBSD_UDP__
#elif defined( __RTOS_IRIX__ )
#define __IRIX_RTOS__
#define __IRIX_TCP__
#define __IRIX_UDP__
#elif defined( __RTOS_QNX__ )
#define __QNX_RTOS__
#define __QNX_TCP__
#define __QNX_UDP__
#elif defined( __RTOS_UITRON__ )
#define __UITRON_RTOS__
#define __UITRON_TCP__
#define __UITRON_UDP__
#elif defined( __RTOS_WINCE__ )
#define __WINCE_RTOS__
#define __WINCE_TCP__
#define __WINCE_UDP__
#elif defined ( __RTOS_SYMBIAN32__ )
#define __SYMBIAN_RTOS__
#define __SYMBIAN_TCP__
#define __SYMBIAN_UDP__
#elif defined( __RTOS_WTOS__ )
#define __WTOS_RTOS__
#define __WTOS_TCP__
#define __WTOS_UDP__
#elif defined( __RTOS_ECOS__ )
#define __ECOS_RTOS__
#define __ECOS_TCP__
#define __ECOS_UDP__
#elif defined( __RTOS_FREERTOS__ )
#define __FREERTOS_RTOS__
#elif defined( __RTOS_AIX__ )
#define __AIX_RTOS__
#define __AIX_TCP__
#define __AIX_UDP__
#elif defined( __RTOS_HPUX__ )
#define __HPUX_RTOS__
#define __HPUX_TCP__
#define __HPUX_UDP__
#elif defined( __RTOS_QUADROS__ )
#define __QUADROS_RTOS__
#define __TRECK_TCP__
#define __TRECK_UDP__
#elif defined( __RTOS_UCOS__ )
#define __UCOS_RTOS__
#define __UCOS_TCP__
#elif defined ( __ENABLE_MOCANA_SEC_BOOT__ )
/* NanoBoot does not need any OS */
#else
#error RTOS NOT DEFINED.  [ __RTOS_VXWORKS__ , __RTOS_LINUX__ , __RTOS_SOLARIS__ , __RTOS_WIN32__ , __RTOS_NNOS__ , __RTOS_PSOS__ , __RTOS_NUCLEUS__ , __RTOS_ARC__ , __RTOS_CYGWIN__ , __RTOS_OSX__ , __RTOS_THREADX__ , __RTOS_NETBURNER__ , __RTOS_OPENBSD__ , __RTOS_NUTOS__ , __RTOS_INTEGRITY__ , __RTOS_ANDROID__ , __RTOS_FREEBSD__, __RTOS_IRIX__, __RTOS_QNX__ , __RTOS_UITRON__ , __RTOS_WINCE__ , __RTOS_SYMBIAN32__ , __RTOS_WTOS__, __RTOS_ECOS__, __RTOS_AIX__, __RTOS_HPUX__ , __RTOS_QUADROS__ ]
#endif

#if !defined( __ENABLE_MOCANA_SSH_SERVER__ ) && defined( __ENABLE_MOCANA_SSH_ASYNC_SERVER_API__ )
#define __ENABLE_MOCANA_SSH_SERVER__
#endif

#if defined( __ENABLE_MOCANA_DTLS_SERVER__ )
#if !defined(__ENABLE_MOCANA_SSL_SERVER__)
#define __ENABLE_MOCANA_SSL_SERVER__
#endif
#if !defined(__ENABLE_MOCANA_SSL_ASYNC_SERVER_API__)
#define __ENABLE_MOCANA_SSL_ASYNC_SERVER_API__
#endif
#if !defined(__ENABLE_MOCANA_SSL_ASYNC_API_EXTENSIONS__)
#define __ENABLE_MOCANA_SSL_ASYNC_API_EXTENSIONS__
#endif
#if defined(__ENABLE_MOCANA_DTLS_SRTP__) && !defined(__ENABLE_RFC3546__)
#define __ENABLE_RFC3546__
#endif
#endif

#if defined( __ENABLE_MOCANA_DTLS_CLIENT__ )
#if !defined(__ENABLE_MOCANA_SSL_CLIENT__)
#define __ENABLE_MOCANA_SSL_CLIENT__
#endif
#if !defined(__ENABLE_MOCANA_SSL_ASYNC_CLIENT_API__)
#define __ENABLE_MOCANA_SSL_ASYNC_CLIENT_API__
#endif
#if !defined(__ENABLE_MOCANA_SSL_ASYNC_API_EXTENSIONS__)
#define __ENABLE_MOCANA_SSL_ASYNC_API_EXTENSIONS__
#endif
#if defined(__ENABLE_MOCANA_DTLS_SRTP__) && !defined(__ENABLE_RFC3546__)
#define __ENABLE_RFC3546__
#endif
#endif

#if !defined( __ENABLE_MOCANA_SSL_SERVER__ ) && defined( __ENABLE_MOCANA_SSL_ASYNC_SERVER_API__ )
#define __ENABLE_MOCANA_SSL_SERVER__
#endif

#if !defined( __ENABLE_MOCANA_SSL_CLIENT__ ) && defined( __ENABLE_MOCANA_SSL_ASYNC_CLIENT_API__ )
#define __ENABLE_MOCANA_SSL_CLIENT__
#endif

#if ((defined( __ENABLE_MOCANA_SSL_CLIENT__ ) || defined( __ENABLE_MOCANA_SSL_SERVER__ )) && \
     !defined( __ENABLE_RFC3546__ ))
#define __ENABLE_RFC3546__ /* extensions needed for TLS 1.2 hash/signature algorithm pairs */
#endif

#if defined(__ENABLE_MOCANA_WPA2__)
#define __ENABLE_MOCANA_SSL_CLIENT__
#define __ENABLE_MOCANA_SSL_ASYNC_CLIENT_API__
#define __ENABLE_MOCANA_SSL_ASYNC_API_EXTENSIONS__
#define __ENABLE_MOCANA_SSL_MUTUAL_AUTH_SUPPORT__
#define __ENABLE_MOCANA_SSL_ALERTS__
#define __ENABLE_MOCANA_SSL_KEY_EXPANSION__
#define __ENABLE_MOCANA_EAP_PEER__
#define __ENABLE_MOCANA_EAP1X_PEER__
#define __ENABLE_MOCANA_EAP_TLS__
#define __ENABLE_MOCANA_EAP_TTLS__
#define __ENABLE_MOCANA_EAP_FAST__
#define __ENABLE_MOCANA_EAP_PSK__
#define __ENABLE_MOCANA_SSL_DH_ANON_SUPPORT__
#define __ENABLE_MOCANA_SSL_CIPHER_SUITES_SELECT__
#define __ENABLE_RFC3546__
#define __ENABLE_MOCANA_EAP_LEAP__
#define __ENABLE_MOCANA_EAP_PEAP__
#define __ENABLE_MOCANA_EAP_MSCHAPv2__
#define __ENABLE_MOCANA_EAP_GTC__
#define __ENABLE_MOCANA_MD4__
#define __ENABLE_MOCANA_EAP_MD5__
#define __DISABLE_EAP_FAST_EXTENSION_LENGTH__
#define __MOCANA_PARENT_CERT_CHECK_OPTIONS__ 0
#define __MOCANA_SELFSIGNED_CERT_CHECK_OPTIONS__ 0
#define __ENABLE_MOCANA_DER_CONVERSION__
#ifdef __ENABLE_MOCANA_DOT11_SME_EXAMPLE__
#define __ENABLE_MOCANA_EXAMPLES__
#endif
#ifdef __RTOS_WIN32__
#define __DP_USE_WPCAP__ 1
#endif
#endif

#if defined(__ENABLE_MOCANA_LDAP_CLIENT__)
#if !defined(__ENABLE_MOCANA_HTTP_CLIENT__)
#define __ENABLE_MOCANA_URI__
#endif
#if defined(__ENABLE_MOCANA_LDAP_TLS_EXTENSION__)
#define __ENABLE_MOCANA_SSL_CLIENT__
#define __ENABLE_MOCANA_SSL_ASYNC_CLIENT_API__
#define __ENABLE_MOCANA_SSL_MUTUAL_AUTH_SUPPORT__
#define __ENABLE_MOCANA_SSL_ALERTS__
#define __ENABLE_MOCANA_DER_CONVERSION__
#endif /* end of __ENABLE_MOCANA_LDAP_TLS_EXTENSION__ */
#endif

#if defined(__ENABLE_MOCANA_SRTP__)
#define __ENABLE_NIL_CIPHER__
#define __ENABLE_MOCANA_GCM_256B__
#if defined(__ENABLE_MOCANA_SRTP_EXAMPLE__)
#define __DISABLE_3DES_CIPHERS__
#define __DISABLE_ARC4_CIPHERS__
#define __DISABLE_MOCANA_SHA224__
#define __DISABLE_MOCANA_SHA256__
#define __DISABLE_MOCANA_SHA384__
#define __DISABLE_MOCANA_SHA512__
#endif
#endif

#if defined(__ENABLE_MOCANA_CMP_CLIENT__)
#if !defined(__ENABLE_MOCANA_HTTP_CLIENT__)
#define __ENABLE_MOCANA_HTTP_CLIENT__
#endif
#if !defined(__ENABLE_MOCANA_URI__)
#define __ENABLE_MOCANA_URI__
#endif
#if !defined(__ENABLE_MOCANA_VERIFY_RSA_SIGNATURE__)
#define __ENABLE_MOCANA_VERIFY_RSA_SIGNATURE__
#endif
#if !defined(__ENABLE_MOCANA_PKCS5__)
#define __ENABLE_MOCANA_PKCS5__
#endif
#if !defined(__CMP_CA_INSTA__)
#define __CMP_CA_INSTA__
#endif
#if !defined(__ENABLE_MOCANA_CERTIFICATE_SEARCH_SUPPORT__)
#define __ENABLE_MOCANA_CERTIFICATE_SEARCH_SUPPORT__
#endif
#endif

#if defined(__ENABLE_MOCANA_VPN_EXAMPLE__)
#define __ENABLE_MOCANA_EXAMPLES__
#define __ENABLE_MOCANA_IKE_SERVER__
#define __ENABLE_IKE_MODE_CFG__
#define __ENABLE_IKE_XAUTH__
#define __ENABLE_IKE_MODE_CFG__
#define __ENABLE_MOCANA_CERTIFICATE_SEARCH_SUPPORT__
#define __ENABLE_IKE_AGGRESSIVE_MODE__
#define __ENABLE_MOBIKE__
#define __IKE_MULTI_HOMING__
#define __ENABLE_MOCANA_VPN_CLIENT_GUI__
#define __ENABLE_MOCANA_DEBUG_CONSOLE__
#define __MOCANA_DUMP_CONSOLE_TO_STDOUT__
#define __ENABLE_ALL_DEBUGGING__
#endif


#if defined(__ENABLE_MOCANA_PKI_CLIENT__)
#if (defined(__ENABLE_MOCANA_CMP_CLIENT__) && defined(__ENABLE_MOCANA_SCEP_CLIENT__))
#error ENABLE CERTIFICATE MANGEMENT PROTOCOL FLAG EITHER __ENABLE_MOCANA_CMP_CLIENT OR __ENABLE_MOCANA_SCEP_CLIENT__
#endif
#if defined(__ENABLE_MOCANA_SCEP_CLIENT__)
#if !defined(__ENABLE_MOCANA_HTTP_CLIENT__)
#define __ENABLE_MOCANA_HTTP_CLIENT__
#endif
#if !defined(__ENABLE_MOCANA_PKCS7__)
#define __ENABLE_MOCANA_PKCS7__
#endif
#if !defined(__ENABLE_DES_CIPHER__)
#define __ENABLE_DES_CIPHER__
#endif
#if !defined(__ENABLE_MOCANA_VERIFY_RSA_SIGNATURE__)
#define __ENABLE_MOCANA_VERIFY_RSA_SIGNATURE__
#endif
#endif /*end of __ENABLE_MOCANA_SCEP_CLIENT__ */
#if !defined(__ENABLE_MOCANA_PRINTF__)
#define __ENABLE_MOCANA_PRINTF__
#endif
#if !defined(__ENABLE_MOCANA_URI__)
#define __ENABLE_MOCANA_URI__
#endif
#if !defined(__ENABLE_MOCANA_PKCS10__)
#define __ENABLE_MOCANA_PKCS10__
#endif
#if !defined(__ENABLE_MOCANA_DER_CONVERSION__)
#define __ENABLE_MOCANA_DER_CONVERSION__
#endif
#if !defined(__ENABLE_MOCANA_CERTIFICATE_SEARCH_SUPPORT__)
#define __ENABLE_MOCANA_CERTIFICATE_SEARCH_SUPPORT__
#endif
#endif

#ifdef __RTOS_WIN32__
#define SCOPEID 4
#endif

#if   defined(__ENABLE_MOCANA_SSL_SERVER__)
#elif defined(__ENABLE_MOCANA_SSH_SERVER__)
#elif defined(__ENABLE_MOCANA_SSL_CLIENT__)
#elif defined(__ENABLE_MOCANA_IPSEC_SERVICE__)
#elif defined(__ENABLE_MOCANA_IKE_SERVER__)
#elif defined(__ENABLE_MOCANA_RADIUS_CLIENT__)
#elif defined(__ENABLE_MOCANA_SSH_CLIENT__)
#elif defined(__ENABLE_MOCANA_EAP_AUTH__)
#elif defined(__ENABLE_MOCANA_EAP_PEER__)
#elif defined(__ENABLE_MOCANA_HTTP_CLIENT__)
#elif defined(__ENABLE_MOCANA_HTTPCC_SERVER__)
#elif defined(__ENABLE_MOCANA_SCEP_CLIENT__)
#elif defined(__ENABLE_MOCANA_SCEPCC_SERVER__)
#elif defined(__MOC_IPV4_STACK__)
#elif defined(__ENABLE_MOCANA_SSLCC_CLIENT__)
#elif defined(__ENABLE_MOCANA_SSLCC_SERVER__)
#elif defined(__ENABLE_MOCANA_HARNESS__)
#elif defined(__ENABLE_MOCANA_CLI__)
#elif defined(__ENABLE_MOCANA_UMP__)
#elif defined(__ENABLE_MOCANA_SEC_BOOT__)
#elif defined(__ENABLE_MOCANA_SIGN_BOOT__)
#elif defined(__ENABLE_MOCANA_WPA2__)
#elif defined(__ENABLE_MOCANA_NTP_CLIENT__)
#elif defined(__ENABLE_MOCANA_SRTP__)
#elif defined(__ENABLE_MOCANA_OCSP_CLIENT__)
#elif defined(__ENABLE_MOCANA_LDAP_CLIENT__)
#elif defined(__ENABLE_MOCANA_FIREWALL__)
#elif defined(__ENABLE_MOCANA_FIPS_MODULE__)
#elif defined(__ENABLE_MOCANA_CMP_CLIENT__)
#elif defined(__ENABLE_MOCANA_SRTP__)
#elif defined(__ENABLE_MOCANA_PKI_CLIENT__)
#else
#error MOCANA PRODUCT NOT DEFINED. [\
 __ENABLE_MOCANA_SSL_SERVER__ ,\
 __ENABLE_MOCANA_SSH_SERVER__ ,\
 __ENABLE_MOCANA_SSL_CLIENT__ ,\
 __ENABLE_MOCANA_IPSEC_SERVICE__ ,\
 __ENABLE_MOCANA_IKE_SERVER__ ,\
 __ENABLE_MOCANA_RADIUS_CLIENT__ ,\
 __ENABLE_MOCANA_SSH_CLIENT__ ,\
 __ENABLE_MOCANA_EAP_AUTH__ ,\
 __ENABLE_MOCANA_EAP_PEER__ ,\
 __ENABLE_MOCANA_HTTP_CLIENT__ ,\
 __ENABLE_MOCANA_HTTPCC_SERVER__ ,\
 __ENABLE_MOCANA_SCEP_CLIENT__ ,\
 __ENABLE_MOCANA_SCEPCC_SERVER__ ,\
 __MOC_IPV4_STACK__ ,\
 __ENABLE_MOCANA_SSLCC_CLIENT__ ,\
 __ENABLE_MOCANA_SSLCC_SERVER__ ,\
 __ENABLE_MOCANA_HARNESS__ ,\
 __ENABLE_MOCANA_CLI__ ,\
 __ENABLE_MOCANA_UMP__ ,\
 __ENABLE_MOCANA_SEC_BOOT__ ,\
 __ENABLE_MOCANA_SIGN_BOOT__ ,\
 __ENABLE_MOCANA_WPA2__,\
 __ENABLE_MOCANA_NTP_CLIENT__, \
 __ENABLE_MOCANA_SRTP__, \
 __ENABLE_MOCANA_LDAP_CLIENT__, \
 __ENABLE_MOCANA_OCSP_CLIENT__, \
 __ENABLE_MOCANA_SRTP__, \
 __ENABLE_MOCANA_FIPS_MODULE__, \
 __ENABLE_MOCANA_CMP_CLIENT__, \
 __ENABLE_MOCANA_PKI_CLIENT__ ]
#endif


/*------------------------------------------------------------------*/

#ifndef MOC_EXTERN
#define MOC_EXTERN extern
#endif

#ifndef MOC_EXTERN_DATA_DEF
#define MOC_EXTERN_DATA_DEF
#endif

#define MOCANA_DEBUG_CONSOLE_PORT           4097

#ifdef __WIN32_RTOS__
#define MOC_UNUSED(X) X=X
#endif


#define MOCANA_YIELD_PROCESSOR()

#ifdef _MSC_VER

#pragma warning( error: 4013 4020 4024 4028 4029 4047 4133 4716 )
#pragma warning( disable: 4200 4206)
#if _MSC_VER >= 8
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifndef __ENABLE_MOCANA_DEBUG_MEMORY__
#if !( defined(__ENABLE_MOCANA_IPSEC_SERVICE__) && defined(__WIN32_RTOS__) )
#include <stdlib.h>
#define MALLOC malloc
#define FREE free
#endif
#endif /* __ENABLE_MOCANA_DEBUG_MEMORY__ */

#endif /* _MSC_VER */

#if (defined(__ENABLE_MOCANA_SSL_ECDH_SUPPORT__) || \
    defined(__ENABLE_MOCANA_SSL_ECDHE_SUPPORT__) || \
    defined(__ENABLE_MOCANA_SSL_ECDH_ANON_SUPPORT__))
#if !defined(__ENABLE_RFC3546__)
#define __ENABLE_RFC3546__ /* TLS extensions are needed if SSL_ECDH or SSL_ECDHE */
#endif
#if !defined(__ENABLE_MOCANA_ECC__) /* only if not on to prevent warnings */
#define __ENABLE_MOCANA_ECC__ /* turn ECC on of course! */
#endif
#endif

/* If def example, turn feature definition into examples also */
#ifdef __ENABLE_MOCANA_EXAMPLES__
  #if defined(__ENABLE_MOCANA_SSH_CLIENT__)
    #define __ENABLE_MOCANA_SSH_CLIENT_EXAMPLE__
  #elif defined(__ENABLE_MOCANA_SSH_SERVER__)
    #define __ENABLE_MOCANA_SSH_SERVER_EXAMPLE__
  #elif defined(__ENABLE_MOCANA_DTLS_CLIENT__) && !defined(__ENABLE_MOCANA_SRTP_DTLS_EXAMPLE__)
    #define __ENABLE_MOCANA_DTLS_CLIENT_EXAMPLE__
    #define __ENABLE_MOCANA_SSL_CLIENT_EXAMPLE__
  #elif defined(__ENABLE_MOCANA_DTLS_SERVER__)
    #define __ENABLE_MOCANA_DTLS_SERVER_EXAMPLE__
    #define __ENABLE_MOCANA_SSL_SERVER_EXAMPLE__
  #elif defined(__ENABLE_MOCANA_SCEP_CLIENT__) && !defined(__ENABLE_MOCANA_PKI_CLIENT__)
    #define __ENABLE_MOCANA_SCEP_CLIENT_EXAMPLE__
  #elif defined(__ENABLE_MOCANA_PKI_CLIENT__)
    #define __ENABLE_MOCANA_PKI_CLIENT_EXAMPLE__
  /* #elif defined(__ENABLE_MOCANA_LDAP_CLIENT__)
    #define __ENABLE_MOCANA_LDAP_CLIENT_EXAMPLE__ */
  #elif defined(__ENABLE_MOCANA_OCSP_CLIENT__)
    #define __ENABLE_MOCANA_OCSP_CLIENT_EXAMPLE__
  #elif defined(__ENABLE_MOCANA_HTTP_CLIENT__)
    #define __ENABLE_MOCANA_HTTP_CLIENT_EXAMPLE__
  #elif defined(__ENABLE_MOCANA_SSL_CLIENT__)
    #define __ENABLE_MOCANA_SSL_CLIENT_EXAMPLE__
  #elif defined(__ENABLE_MOCANA_SSL_SERVER__)
    #define __ENABLE_MOCANA_SSL_SERVER_EXAMPLE__
  #elif defined(__ENABLE_MOCANA_IKE_SERVER__)
    #define __ENABLE_MOCANA_IKE_SERVER_EXAMPLE__
  #elif defined(__ENABLE_MOCANA_SCEPCC_SERVER__)
    #define __ENABLE_MOCANA_SCEPCC_SERVER_EXAMPLE__
  /* #elif defined(__ENABLE_MOCANA_CMP_CLIENT__)
    #define __ENABLE_MOCANA_CMP_CLIENT_EXAMPLE__ */
  #elif defined(__ENABLE_MOCANA_RADIUS_CLIENT__)
    #define __ENABLE_MOCANA_RADIUS_CLIENT_EXAMPLE__
  #elif defined(__ENABLE_MOCANA_FIREWALL__)
    #define __ENABLE_MOCANA_FIREWALL_EXAMPLE__
  #endif

  #if (defined (__ENABLE_MOCANA_PKI_CLIENT_EXAMPLE__) && \
       defined (__ENABLE_MOCANA_PKI_CLIENT_FOREGROUND__) && \
       defined (__ENABLE_MOCANA_IKE_SERVER__) )
    #define __ENABLE_MOCANA_IKE_SERVER_EXAMPLE__
  #endif
#endif

#ifndef EAP_PACKED
#define EAP_PACKED
#endif

#ifndef EAP_PACKED_POST
#ifdef __GNUC__
#define EAP_PACKED_POST    __attribute__ ((__packed__))
#else
#define EAP_PACKED_POST
#endif
#endif


#endif /* __MOPTIONS_HEADER__ */
