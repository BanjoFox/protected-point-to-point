/**
 * \file p3kcrypto.h
 * <h3>Protected Point to Point key server header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3k_CRYPTO_H
#define _p3k_CRYPTO_H

/*****  INCLUDE FILES *****/

#include "p3linux.h"

/*****  CONSTANTS  *****/

#define p3KTYPE_AES128	1
#define p3KSIZE_AES128	16
#define p3KTYPE_AES256	2
#define p3KSIZE_AES256	32
#define p3MAX_KSIZE		p3KSIZE_AES256

#define p3CRYPTO_ALIGN	16

#define p3DATENC1		1
#define p3DATDEC1		2
#define p3DATENC0		3
#define p3DATDEC0		4
#define p3CTLENC1		5
#define p3CTLDEC1		6
#define p3CTLENC0		7
#define p3CTLDEC0		8

/*****  DATA DEFINITIONS  *****/

typedef struct _p3key_serv p3key_serv;
typedef struct _p3key_mgr p3key_mgr;
typedef struct _p3key p3key;
typedef struct _p3keymgmt p3keymgmt;

/**
 * Structure:
 * p3key_serv
 * 
 * \par Description:
 * The key server structure to maintain information about encyrption keys.
 */

struct _p3key_serv {
	int				head;		/**< Head pointer index for the circular buffer */
	int				tail;		/**< Tail pointer index for the circular buffer */
	int				cbuf_sz;	/**< Number of slots in the circular buffer */
};

/**
 * Structure:
 * p3key_mgr
 *
 * \par Description:
 * The key server management information.
 */

struct _p3key_mgr {
	p3key_serv		*key_serv;	/*<< Key server */
	p3lock			lock;		/**< Circular buffer lock */
};

/**
 * Structure:
 * p3key
 * 
 * \par Description:
 * The structure to maintain information about encyrption keys.
 */

struct _p3key {
	unsigned char	key[32];		/**< Key field for up to 256-bit key */
	unsigned int	size;
};

/**
 * Structure:
 * p3keymgmt
 *
 * \par Description:
 * The structure to maintain information about encyrption keys.
 */

struct _p3keymgmt {
	void			*datenc0;	/*<< Session data encryption context 0 */
	void			*datdec0;	/*<< Session data decryption context 0 */
	void			*datenc1;	/*<< Session data encryption context 1 */
	void			*datdec1;	/*<< Session data decryption context 1 */
	void			*ctlenc0;	/*<< Session control encryption context 0 */
	void			*ctldec0;	/*<< Session control decryption context 0 */
	void			*ctlenc1;	/*<< Session control encryption context 1 */
	void			*ctldec1;	/*<< Session control decryption context 1 */
	p3key			*dnewkey;	/*<< New data key */
	p3key			*cnewkey;	/*<< New control key */
#define p3KMG_KEYS	2
};

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

int p3_get_key_size(int type);
int p3_get_key(p3key *key, p3key_mgr *key_mgr);
p3key **p3_get_key_array(int size, int number, p3key_mgr *key_mgr);
int p3_init_crypto(p3keymgmt *keys);
int p3_rekey(p3keymgmt *keys);
int p3_encrypt(unsigned char *buffer, int size, unsigned int id, int key, p3keymgmt *keys);
int p3_decrypt(unsigned char *buffer, int size, unsigned int id, int key, p3keymgmt *keys);

/*****  EXTERNAL DEFINITIONS  *****/

#endif /* _p3k_CRYPTO_H */
