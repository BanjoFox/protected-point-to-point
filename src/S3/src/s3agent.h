/**
 * \file s3agent.h
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

typedef struct _s3agent s3agent;
typedef struct s3key s3key;

/**
 * Structure:
 * s3agent
 * 
 * \par Description:
 * The key server structure to maintain information about encyrption keys.
 */

struct _s3agent {
	unsigned int	flag;
#define s3KSV_FLG1	0x00000001
};

struct _s3key {
	unsigned int	key[8];		/**< Key structure for 256-bit key */
	int				size;		/**< The size, in bits, of the key */
};

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

int init_agent();
s3key *s3_req_key(int size);
unsigned char *s3_encrypt(unsigned char *buffer, int size, s3key *key);
unsigned char *s3_decrypt(unsigned char *buffer, int size, s3key *key);

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _s3_AGENT_C
extern s3agent *agent;
#endif

#endif /* _s3_ENCRYPT_ENGINE_H */
