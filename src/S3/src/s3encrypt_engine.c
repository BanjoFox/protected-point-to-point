/**
 * \file s3encrypt_engine.c
 * <h3>Secure Storage Solution encryption engine file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The encryption engine handles encrypting and decrypting buffers.
 */

/**
 * \page S3EE_MSGS Secure Storage Solution Encryption Engine Messages
 * <p><hr><hr>
 * \section S3_ENC_ENG S3 Encryption Engine Messages
 */

#define _s3ENCRYPT_ENGINE_C
#include "s3encrypt_engine.h"

/** The main encryption engine data structure */
s3enc_eng *enc_eng;

/**
 * \par Function:
 * init_enc_eng
 *
 * \par Description:
 * Initialize the encryption engine.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_enc_eng()
{
	int ks_status = 0;

	// Create encryption engine data structure
	if ((enc_eng = (s3enc_eng *) calloc(1, sizeof(s3enc_eng))) == NULL) {
		sprintf(s3buf, "init_enc_eng: Failed to allocate s3 encryption engine\
 data structure: %s\n", strerror(errno));
		s3errmsg (s3MSG_CRIT, s3buf);
		ks_status = -1;
		goto out;
	}

out:
	return (ks_status);
} /* end init_enc_eng */

/**
 * \page S3EE_MSGS Secure Storage Solution Encryption Engine Messages
 * <hr><b>init_enc_eng: Failed to allocate encryption engine data structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The S3 encryption engine attempts to allocate the encryption engine
 * structure during initialization.  If this fails, there is a system
 * wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * s3_encrypt
 *
 * \par Description:
 * Encrypt a buffer.
 * 
 * \par Inputs:
 * - buffer:  The buffer to be encrypted
 * - size: The size of the buffer, in bytes
 * - key: The key structure to use for the encryption
 *
 * \par Outputs:
 * - unsigned char*: The encrypted buffer.  If there is an error,
 *   NULL is returned.
 */

unsigned char *s3_encrypt(unsigned char *buffer, int size, s3key *key)
{
	unsigned char *en_buf = NULL;

	return (en_buf);
} /* end s3_encrypt */

/**
 * \par Function:
 * s3_decrypt
 *
 * \par Description:
 * Decrypt a buffer.
 * 
 * \par Inputs:
 * - buffer:  The buffer to be decrypted
 * - size: The size of the buffer, in bytes
 * - key: The key structure to use for the decryption
 *
 * \par Outputs:
 * - unsigned char*: The decrypted buffer.  If there is an error,
 *   NULL is returned.
 */

unsigned char *s3_decrypt(unsigned char *buffer, int size, s3key *key)
{
	unsigned char *de_buf = NULL;

	return (de_buf);
} /* end s3_decrypt */

