/**
 * \file s3encrypt_engine.h
 * <h3>Secure Storage Solution key server header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _s3_ENCRYPT_ENGINE_H
#define _s3_ENCRYPT_ENGINE_H

/*****  INCLUDE FILES *****/

#include "s3base.h"

/*****  CONSTANTS  *****/

/*****  DATA DEFINITIONS  *****/

typedef struct _s3enc_eng s3enc_eng;
typedef struct s3key s3key;

/**
 * Structure:
 * s3enc_eng
 * 
 * \par Description:
 * The key server structure to maintain information about encryption keys.
 */

struct _s3enc_eng {
	unsigned int	flag;
#define s3KSV_FLG1	0x00000001
};

struct _s3key {
	unsigned int	key[8];		/**< Key structure for 256-bit key */
	int				size;		/**< The size, in bits, of the key */
};

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

int init_enc_eng();
unsigned char *s3_encrypt(unsigned char *buffer, int size, s3key *key);
unsigned char *s3_decrypt(unsigned char *buffer, int size, s3key *key);

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _s3_ENCRYPT_ENGINE_C
extern s3enc_eng *enc_eng;
#endif

#endif /* _s3_ENCRYPT_ENGINE_H */
