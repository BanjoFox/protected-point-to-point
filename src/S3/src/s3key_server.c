/**
 * \file s3key_server.c
 * <h3>Secure Storage Solution key server key server file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The key server provides true random keys for encryption.  It runs in
 * the background so it can constantly be producing keys to be instantly
 * available on request.
 */

/** \mainpage Velocite Systems S3 Solution Documentation
 * \section S3_OVERVIEW Overview:
 * The Velocite Systems Secure Storage Solution Solution secures data
 * at rest by encyrpting files with a unique key.  The documentation
 * for this application includes:
 *
 * \section S3_FUNC Functions:
 * The S3 Solution source code files are described in the Files section.
 * There are two tabs in this section:
 * - File List: Each function is described under the source code file where
 *   it is found.  The constants, macros, and data definitions used by 
 *   functions are described in the corresponding header file.
 * - Globals: All global variables and contant definitions used in the
 *   S3 Solution application are described in the Globals section.
 *
 * \section S3_STRU Structures:
 * - All data structures used in the S3 Solution application are described
 *   in the Data Structures section.
 *
 * \section S3_MSG Messages:
 * - The S3 Solution messages are explained in the Related Pages section.
 */

/**
 * \page S3KS_MSGS Secure Storage Solution Key Server Messages
 * <p><hr><hr>
 * \section S3_KEY S3 Key Server Messages
 */

#define _s3KEY_SERVER_C
#include "s3key_server.h"
#include "s3key server.h"

/** The main key server data structure */
s3key_serv *key_serv;

/**
 * \par Function:
 * init_key_serv
 *
 * \par Description:
 * Initialize the key server.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_key_serv()
{
	int ks_status = 0;

	// Create key server data structure
	if ((key_serv = (s3key_serv *) calloc(1, sizeof(s3key_serv))) == NULL) {
		sprintf(s3buf, "init_key_serv: Failed to allocate s3 key server data\
 structure: %s\n", strerror(errno));
		s3errmsg (s3MSG_CRIT, s3buf);
		ks_status = -1;
		goto out;
	}

out:
	return (ks_status);
} /* end init_key_serv */

/**
 * \page S3KS_MSGS Secure Storage Solution Key Server Messages
 * <hr><b>init_key_serv: Failed to allocate key server data structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The S3 key server attempts to allocate the key server structure during
 * initialization.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * s3_get_key
 *
 * \par Description:
 * Get an encryption key from the key repository
 * 
 * \par Inputs:
 * - host: The host for which the key is requested
 * - path: The fully qualified path of the file
 * - file: The name of the file for which the key is requested
 *
 * \par Outputs:
 * - s3key*: The structure containing the key.  If there is an error,
 *   NULL is returned.
 */

s3key *s3_get_key(char *host, char *path, char *file)
{
	s3key *gk_key = NULL;

	return (gk_key);
} /* end s3_get_key */

/**
 * \par Function:
 * s3_new_key
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

s3key *s3_new_key(int size)
{
	int nk_size = size >> 3;
	s3key *nk_key = NULL;

	return (nk_key);
} /* end s3_new_key */

