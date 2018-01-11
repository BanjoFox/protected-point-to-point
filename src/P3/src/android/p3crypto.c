/**
 * \file p3crypto.c
 * <h3>Protected Point to Point cryptography file</h3>
 *
 * Copyright (C) Velocite 2010
 *
 * The cryptography module provides true random keys for encryption and
 * handles the encryption and decryption of buffers.
 */
/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <p><hr><hr>
 * \section P3_CRYPTO P3 Cryptography Messages
 */

#include "p3utils.h"
#include "p3system.h"
#include "p3crypto.h"

// !!! Temporary !!!
// !!! Temporary !!!
FILE *InFile = NULL;
// !!! Temporary !!!
// !!! Temporary !!!

/**
 * \par Function:
 * p3_get_key
 *
 * \par Description:
 * Get an encryption key.
 *
 * \par Inputs:
 * - key: The P3 key structure.  The flag in the structure contains
 *   information about the key, such as its type (size).  The new key
 *   is returned in this structure.
 *
 * \par Outputs:
 * - int: Status
 *   - 0: OK
 *   - <0: Error
 *   - >0: Key not obtained, retry later
 */
int p3_get_key(p3key *key)
{
	int i, stat = 0;

	// Get an encryption key
	// !!! Temporary !!!
	// !!! Temporary !!!
	if (InFile == NULL && (InFile = fopen(p3KEY_FILE,"r")) == NULL) {
		p3errmsg(p3MSG_CRIT, "Error opening encryption key file\n");
		stat = -1;
		goto out;
	}
	if ((i = fread(key->key, 1, key->size, InFile)) < key->size) {
sprintf(p3buf, "Read error: Req %d, Actual %d\n", key->size, i);
p3errmsg(p3MSG_DEBUG, p3buf);
		stat = 1;
		goto out;
	}
	// !!! Temporary !!!
	// !!! Temporary !!!

out:
	return (stat);
} /* end p3_get_key */
