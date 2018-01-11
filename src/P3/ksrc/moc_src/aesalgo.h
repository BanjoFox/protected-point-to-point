/* Version: mss_v5_3_1_13561 */
/*
 * aesalgo.h
 *
 * AES Implementation
 *
 * Copyright Mocana Corp 2003-2007. All Rights Reserved.
 * Proprietary and Confidential Material.
 *
 */


/*------------------------------------------------------------------*/

#ifndef __AESALGO_HEADER__
#define __AESALGO_HEADER__

#ifdef __cplusplus
extern "C" {
#endif

/*  Generic Defines  */
#define MODE_ECB                1         /* Are we ciphering in ECB mode?   */
#define MODE_CBC                2         /* Are we ciphering in CBC mode?   */
#define MODE_CFB1               3         /* Are we ciphering in 1-bit CFB mode? */

MOC_EXTERN sbyte4 aesKeySetupEnc(ubyte4 rk[/*4*(Nr + 1)*/], const ubyte cipherKey[], sbyte4 keyBits);
MOC_EXTERN sbyte4 aesKeySetupDec(ubyte4 rk[/*4*(Nr + 1)*/], const ubyte cipherKey[], sbyte4 keyBits);
MOC_EXTERN void aesEncrypt(ubyte4 rk[/*4*(Nr + 1)*/], sbyte4 Nr, ubyte pt[16], ubyte ct[16]);
MOC_EXTERN void aesDecrypt(ubyte4 rk[/*4*(Nr + 1)*/], sbyte4 Nr, ubyte ct[16], ubyte pt[16]);

#ifdef __cplusplus
}
#endif

#endif /* __AESALGO_HEADER__ */
