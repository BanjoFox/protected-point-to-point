/**
 * \file p3crypto.h
 * <h3>Protected Point to Point key server header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3_CRYPTO_H
#define _p3_CRYPTO_H

/*****  INCLUDE FILES *****/

#ifndef _p3_SECONDARY

#define __ENABLE_MOCANA_SSL_CLIENT__
#define __ENABLE_MOCANA_FIPS_MODULE__
#define __RTOS_LINUX__
#include "common/moptions.h"

#include "common/mtypes.h"
#include "common/mdefs.h"
#include "common/merrors.h"
#include "common/mrtos.h"
#include "common/mstdlib.h"
#include "common/random.h"
#include "common/mocana.h"
#include "crypto/hw_accel.h"
#include "crypto/des.h"
#include "crypto/three_des.h"
#include "common/vlong.h"
#include "crypto/rsa.h"
#include "crypto/sha1.h"
#include "crypto/md5.h"
#include "crypto/crypto.h"
#include "crypto/sha256.h"
#include "crypto/sha512.h"
#include "crypto/md5.h"
#include "crypto/hmac.h"
#include "crypto/aes.h"
#include "crypto/aes_ctr.h"
#include "crypto/aes_cmac.h"
#include "crypto/aes_ccm.h"
#include "crypto/aes_ecb.h"
#include "crypto/dsa.h"
#include "crypto/dh.h"
#include "crypto/pkcs1.h"
#include "crypto/primefld.h"
#include "crypto/primeec.h"

#include <sys/time.h>
#include <time.h>

#endif /* _p3_SECONDARY */

/*****  CONSTANTS  *****/

// !!! TEMPORARY !!!
// !!! TEMPORARY !!!
#define p3KEY_FILE	"p3keys.dat"
// !!! TEMPORARY !!!
// !!! TEMPORARY !!!

#define p3MIN_KSIZE		8
#define p3KTYPE_AES128	1
#define p3KSIZE_AES128	16
#define p3KTYPE_AES256	2
#define p3KSIZE_AES256	32
#define p3MAX_KSIZE		p3KSIZE_AES256

#ifndef _p3_SECONDARY
/*****  DATA DEFINITIONS  *****/

typedef struct _p3key p3key;
typedef struct _p3rng p3rng;

/**
 * Structure:
 * p3key
 * 
 * \par Description:
 * The structure to maintain information about encyrption keys.
 */

struct _p3key {
	unsigned char	key[p3MAX_KSIZE];	/**< Must hold largest key */
	unsigned int	size;
};

/**
 * Structure:
 * p3rng
 * 
 * \par Description:
 * The structure to pass requirements to the random number generator.
 *
 * When the structure is allocated, it is immediately followed by an
 * unsigned array which contains the following fields:
 *   Name  Len
 *   ----  ---
 *   rbuf   64
 *   seed   64
 *   xkey   64
 *   ebits   1
 */

struct _p3rng {
	hwAccelDescr	hwAccelCtx;
	unsigned char	*rbuf;		/*<< Random number */
	int				rlen;		/*<< Random number length */
#define p3RNG_RBUF_MIN	p3MAX_KSIZE
	unsigned char	*seed;		/*<< Seed for RNG */
	int				slen;		/*<< Seed length */
#define p3RNG_SEED		p3RNG_RBUF_MIN
#define p3RNG_SEED_MIN	32		/* Mininum seed field */
#define p3RNG_SEED_SZ	16
	unsigned char	*xkey;		 /*<< Entropy added to the seed */
	int				xklen;		 /*<< xKey length (20 - 64) */
#define p3RNG_XKEY		(p3RNG_SEED + p3RNG_SEED_MIN)
#define p3RNG_XKEY_MIN	32
#define p3RNG_XKEY_SZ	30
	unsigned char	*ebits;		/*<< Entropy bits (1 byte) */
#define p3RNG_EBITS		(p3RNG_XKEY + p3RNG_XKEY_MIN)
	int 			index;		/*<< Entropy generator index */
};

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

int p3_get_key(p3key *key);
p3key **p3_get_key_array(int size, int number);
int p3_genrand();
int init_rng(void);
void get_entropy(void);


/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _p3_SYSTEM_C
extern p3rng *rng;
#endif
#endif /* _p3_SECONDARY */

#endif /* _p3_CRYPTO_H */
