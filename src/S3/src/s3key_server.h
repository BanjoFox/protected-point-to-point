/**
 * \file s3key_server.h
 * <h3>Secure Storage Solution key server header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _s3_KEY_SERV_H
#define _s3_KEY_SERV_H

/*****  INCLUDE FILES *****/

#include "s3base.h"

/*****  CONSTANTS  *****/

/*****  DATA DEFINITIONS  *****/

typedef struct _s3key_serv s3key_serv;
typedef struct s3key s3key;

/**
 * Structure:
 * s3key_serv
 * 
 * \par Description:
 * The key server structure to maintain information about encyrption keys.
 */

struct _s3key_serv {
	unsigned int	flag;
#define s3KSV_FLG1	0x00000001
};

struct _s3key {
	unsigned int	key[8];		/**< Key structure for 256-bit key */
	int				size;		/**< The size, in bits, of the key */
};

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

int init_key_serv();
s3key *s3_get_key(char *host, char *path, char *file);
s3key *s3_new_key(int size);

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _s3_KEY_SERV_C
extern s3key_serv *key_serv;
#endif

#endif /* _s3_KEY_SERV_H */
