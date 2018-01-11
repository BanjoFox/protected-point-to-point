/**
 * \file s3agent.c
 * <h3>Secure Storage Solution encryption engine file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The S3 agent interfaces with the host file system to intercept data
 * being read from or written to storage media.  It determines if the data
 * needs to be encrypted, and if so, requests a key from the key server
 * and then sends the buffer and key to the encryption engine for processing.
 */

/**
 * \page S3AG_MSGS Secure Storage Solution Agent Messages
 * <p><hr><hr>
 * \section S3_AGENT S3 Agent Messages
 */

#define _s3AGENT_C
#include "s3agent.h"

/** The main agent data structure */
s3agent *agent;

/**
 * \par Function:
 * init_agent
 *
 * \par Description:
 * Initialize the agent.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_agent()
{
	int ks_status = 0;

	// Create agent data structure
	if ((agent = (s3agent *) calloc(1, sizeof(s3agent))) == NULL) {
		sprintf(s3buf, "init_agent: Failed to allocate s3 encryption engine\
 data structure: %s\n", strerror(errno));
		s3errmsg (s3MSG_CRIT, s3buf);
		ks_status = -1;
		goto out;
	}

out:
	return (ks_status);
} /* end init_agent */

/**
 * \page S3AG_MSGS Secure Storage Solution Agent Messages
 * <hr><b>init_agent: Failed to allocate agent data structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The S3 agent attempts to allocate the agent structure during
 * initialization.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * s3_req_key
 *
 * \par Description:
 * Get an encryption key.
 * 
 * \par Inputs:
 * - size: The size, in bits, of the key to be retrieved
 *
 * \par Outputs:
 * - s3key*: The structure containing the key.  If there is an error,
 *   NULL is returned.
 */

s3key *s3_req_key(int size)
{
	int gk_size = size >> 3;
	s3key *gk_key = NULL;

	return (gk_key);
} /* end s3_req_key */

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

