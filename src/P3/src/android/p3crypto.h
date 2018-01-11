/**
 * \file p3crypto.h
 * <h3>Protected Point to Point key server header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3_CRYPTO_H
#define _p3_CRYPTO_H

/*****  INCLUDE FILES *****/

/*****  CONSTANTS  *****/

// !!! TEMPORARY !!!
// !!! TEMPORARY !!!
#define p3KEY_FILE	"p3keys.dat"
// !!! TEMPORARY !!!
// !!! TEMPORARY !!!

#define p3KTYPE_AES128	1
#define p3KSIZE_AES128	16
#define p3KTYPE_AES256	2
#define p3KSIZE_AES256	32
#define p3MAX_KSIZE		p3KSIZE_AES256

/*****  DATA DEFINITIONS  *****/

typedef struct _p3key p3key;

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

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

int p3_get_key(p3key *key);
p3key **p3_get_key_array(int size, int number);

/*****  EXTERNAL DEFINITIONS  *****/

#endif /* _p3_CRYPTO_H */
