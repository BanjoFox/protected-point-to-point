/**
 * \file p3crypto.c
 * <h3>Protected Point to Point cryptography file</h3>
 *
 * Copyright (C) Velocite 2010
 *
 * The cryptography module provides true random keys for encryption and
 * handles the encryption and decryption of buffers.  It is only used by
 * the primary P3 host.
 */

#ifndef _p3_SECONDARY

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
#ifdef p3FIXED_KEYS
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
#else
	rng->rlen = key->size;
	if ((stat = p3_genrand(rng)) < 0)
		goto out;
	memcpy(rng->rbuf, &key->key, key->size);
#endif

out:
	return (stat);
} /* end p3_get_key */


/**
 * \par Function:
 * p3init_rng
 *
 * \par Description:
 * Initialize the random number generator context.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0: OK
 *   - <0: Error
 */

int init_rng(void)
{
	int i, stat = 0;
	unsigned long l;

	i = sizeof(p3rng) + p3RNG_EBITS + 1;
	if ((rng = (p3rng *) p3calloc(i)) == NULL) {
		sprintf(p3buf, "init_rng: Failed to allocate p3 random number\
 generator structure: %s\n", strerror(errno));
		p3errmsg(p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}

	// Initialize Random Number Generator structure
	if (HARDWARE_ACCEL_OPEN_CHANNEL(MOCANA_SSL, &rng->hwAccelCtx) < OK) {
		p3errmsg(p3MSG_CRIT,
			"init_rng: Failed to initialize p3 random number generator\n");
		free(rng);
		rng = NULL;
		stat = -1;
		goto out;
	}
	l = (unsigned long) rng + sizeof(p3rng);
	rng->rbuf = (unsigned char *) l;
	l += p3RNG_RBUF_MIN;
	rng->seed = (unsigned char *) l;
	l += p3RNG_SEED_MIN;
	rng->xkey = (unsigned char *) l;
	l += p3RNG_XKEY_MIN;
	rng->ebits = (unsigned char *) l;
	rng->rlen = p3MAX_KSIZE;
	rng->slen = p3RNG_SEED_SZ;
	rng->xklen = p3RNG_XKEY_SZ;
	rng->index = p3RNG_RBUF_MIN;

	// Initialize entropy
	for (i=p3RNG_RBUF_MIN; i < (p3RNG_EBITS + 1); i++) {
		get_entropy();
	}
//	HARDWARE_ACCEL_CLOSE_CHANNEL(MOCANA_SSL, &rng->hwAccelCtx);

out:
	return (stat);

} /* end init_rng */

/**
 * \par Function:
 * p3_genrand
 *
 * \par Description:
 * Generate a random number.  The P3 Random Number Generator structure
 * contains information about the random number, including the random
 * number itself, see p3crypto.h for the description of the fields.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0: OK
 *   - <0: Error
 */

int p3_genrand()
{
	int stat = OK;
	randomContext* pRandomContext = NULL;

	if ((stat = RANDOM_newFIPS186Context(&pRandomContext, rng->xklen,
			rng->xkey, rng->slen, rng->seed)) < OK) {
		p3errmsg(p3MSG_ERR, "p3_genrand: RANDOM_newFIPS186Context failed\n");
		stat = -1;
		goto out;
	}

#ifndef __DISABLE_MOCANA_ADD_ENTROPY__
	// Add 32 bits more rentropy
	if ((stat = RANDOM_addEntropyBit(pRandomContext, rng->ebits[0])) < OK) {
		p3errmsg(p3MSG_ERR, "p3_genrand: RANDOM_addEntropyBit failed\n");
		stat = -1;
		goto out;
	}
#endif

	// Generates random number
	if ((stat = RANDOM_numberGenerator(pRandomContext, rng->rbuf, rng->rlen))
			< OK) {
		p3errmsg(p3MSG_ERR, "p3_genrand: RANDOM_numberGenerator failed\n");
		stat = -1;
		goto out;
	}

	// Destroy RNG context to prevent memory leak
	if ((stat = RANDOM_releaseContext(&pRandomContext)) < OK) {
		p3errmsg(p3MSG_ERR, "p3_genrand: RANDOM_releaseContext failed\n");
		stat = -1;
		goto out;
	}

out:
	return stat;

} /* p3_genrand */

/**
 * \par Function:
 * get_entropy
 *
 * \par Description:
 * Get one byte of entropy from microsecond clock and add
 * to the Random Number Generator structure array.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - None
 */

// TODO: Run as a thread
void get_entropy()
{
	int stat = 0;
	unsigned int t1, t2, t3;
	unsigned char byte;
	struct timeval ts;
	struct timezone tz;

	gettimeofday(&ts, &tz);
	t1 = ts.tv_usec;
	usleep(231);
	gettimeofday(&ts, &tz);
	t2 = ts.tv_usec;
	usleep(187);
	gettimeofday(&ts, &tz);
	t3 = ts.tv_usec;
	usleep(23);

	switch (t3 & 7) {
	case 0:
		byte = ((t1 >> 1) ^ t2) & 0xff;
		break;

	case 1:
		byte = (t1 ^ (t2 >> 1)) & 0xff;
		break;

	case 2:
		byte = ((t1 >>1) ^ (t2 >> 1)) & 0xff;
		break;

	case 3:
		byte = ((t1 >> 2) ^ t2) & 0xff;
		break;

	case 4:
		byte = ((t1 >> 2) ^ (t2 >> 1)) & 0xff;
		break;

	case 5:
		byte = (t1 ^ (t2 >> 2)) & 0xff;
		break;

	case 6:
		byte = ((t1 >> 1) ^ (t2 >> 2)) & 0xff;
		break;

	case 7:
		byte = (t1 ^ t2) & 0xff;
		break;

	}

	rng->rbuf[rng->index++] = byte;
	if (rng->index >= (p3RNG_EBITS + 1))
		rng->index = p3RNG_RBUF_MIN;

	return;
} /* end get_entropy */

#endif /* _p3_SECONDARY */

