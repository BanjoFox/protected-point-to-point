/**
 * \file p3kcrypto.c
 * <h3>Protected Point to Point cryptography file</h3>
 *
 * Copyright (C) Velocite 2010
 *
 * The cryptography module provides true random keys for encryption and
 * handles the encryption and decryption of buffers.
 */
/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <p><hr><hr>
 * \section P3KM_CRYPTO P3 Cryptography Messages
 */

#include "p3kbase.h"
#include "p3kcrypto.h"

#include "moc_src/moptions.h"
#include "moc_src/mtypes.h"
#include "moc_src/mdefs.h"
#include "moc_src/merrors.h"
#include "moc_src/mrtos.h"
#include "moc_src/mstdlib.h"
#include "moc_src/random.h"
#include "moc_src/mocana.h"

#include "moc_src/hw_accel.h"
#include "moc_src/aes.h"
#include "moc_src/aes_ctr.h"
#include "moc_src/aes_cmac.h"
#include "moc_src/aes_ccm.h"
#include "moc_src/aes_ecb.h"

char unknown_err[] = {"Unknown error"};
p3lock crypto_lock;
int initlock = 1, lockheld = 0;;

/**
 * \par Function:
 * p3_get_key_size
 *
 * \par Description:
 * Get the size of an encryption key.
 *
 * \par Inputs:
 * - type: The type of the encryption algorithm.
 *
 * \par Outputs:
 * - int:
 *   - 0: Key size
 *   - <0: Error
 */
int p3_get_key_size(int type)
{
	int size = -1;

	switch(type) {
	case p3KTYPE_AES128:
		size = p3KSIZE_AES128;
		break;

	case p3KTYPE_AES256:
		size = p3KSIZE_AES256;
		break;
	}

	return (size);
} /* end p3_get_key_size */

/**
 * \par Function:
 * p3_get_key
 *
 * \par Description:
 * Get an encryption key from the Ramdisk buffer.
 *
 * \par Inputs:
 * - key: The P3 key structure.  The flag in the structure contains
 *   information about the key, such as its type (size).  The new key
 *   is returned in this structure.
 * - key_mgr: The P3 key manager structure that maintains information
 *   about the circular buffer of keys.
 *
 * \par Outputs:
 * - int: Status
 *   - 0: OK
 *   - <0: Error
 *   - >0: Key unavailable, try later
 */
int p3_get_key(p3key *key, p3key_mgr *key_mgr)
{
	int i, stat = 0;
	unsigned long l;
	p3key_serv *key_serv;
	unsigned char *cbuf;

	if (key_mgr == NULL || (key_serv = key_mgr->key_serv) == NULL) {
		p3errmsg(p3MSG_ERR, "p3_get_key: Key server is NULL\n");
		stat = -1;
		goto out;
	}
	l = (unsigned long) key_serv + sizeof(p3key_serv);
	cbuf = (unsigned char *) l;

sprintf(p3buf, " == Get key: H %d, T %d\n", key_serv->head, key_serv->tail);
p3errmsg(p3MSG_DEBUG, p3buf);
	p3lock(key_mgr->lock);
	// No keys available
	if (key_serv->head == key_serv->tail ||
			key_serv->head == (key_serv->tail - 1)) {
		stat = 1;
// TODO: Zero used keys??
	} else {
		// Tail has wrapped around circular buffer
		if (key_serv->tail < key_serv->head) {
			if ((key_serv->head + key->size) <= key_serv->cbuf_sz) {
sprintf(p3buf, " == Copy key: %p, %p, %d\n",
	&key->key, &cbuf[key_serv->head], key->size);
p3errmsg(p3MSG_DEBUG, p3buf);
				memcpy(&key->key, &cbuf[key_serv->head], key->size);
			// Head wraps around circular buffer
			} else {
				key_serv->head = 0;
				if ((key_serv->tail - key_serv->head) < key->size) {
					stat = 1;
				} else {
sprintf(p3buf, " == Copy key: %p, %p, %d\n",
	&key->key, &cbuf[key_serv->head], key->size);
p3errmsg(p3MSG_DEBUG, p3buf);
					memcpy(&key->key, &cbuf[key_serv->head], key->size);
				}
			}
			// Advance head
			if ((key_serv->head += key->size) > key_serv->cbuf_sz)
				key_serv->head = 0;
		// Not enough key data
		} else if ((key_serv->tail - key_serv->head) < key->size) {
			stat = 1;
		} else {
sprintf(p3buf, " == Copy key: %p, %p, %d\n",
	&key->key, &cbuf[key_serv->head], key->size);
p3errmsg(p3MSG_DEBUG, p3buf);
			memcpy(&key->key, &cbuf[key_serv->head], key->size);
			// Advance head
			if ((key_serv->head += key->size) > key_serv->cbuf_sz)
				key_serv->head = 0;
		}

	}
	p3unlock(key_mgr->lock);

out:
	return (stat);
} /* end p3_get_key */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <hr><b>p3_get_key: Key server is NULL</b>
 * \par Description (ERR):
 * The key server was not properly initialized.
 * \par Response:
 * Report the problem to Velocite Systems support.
 *
 */


/**
 * \par Function:
 * p3_get_key_array
 *
 * \par Description:
 * Get an array of encryption keys.  All keys in the array are true
 * random numbers.
 *
 * \par Inputs:
 * - size: The size, in bytes, of the key to be retrieved
 * - number: The number of keys in the array
 * - key_mgr: The P3 key server structure that maintains information
 *   about the circular buffer of keys.
 *
 * \par Outputs:
 * - p3key**: The array containing the keys.  If there is an error,
 *   NULL is returned.
 */
p3key **p3_get_key_array(int size, int number, p3key_mgr *key_mgr)
{
	p3key **gk_key_array = NULL;
	// TODO: Get an encryption key array
	return (gk_key_array);
} /* end p3_get_key_array */

/**
 * \par Function:
 * p3_init_crypto
 *
 * \par Description:
 * Initialize the keys for a session by creating the crypto context
 * for each key.
 *
 * <i>The value of the data and control new key fields will be used
 *    for initializing the contexts.</i>
 *
 * \par Inputs:
 * - keys: The session key managment structure.
 *
 * \par Outputs:
 * - int: Status
 *   - 0: OK
 *   - <0: Error
 */

int p3_init_crypto(p3keymgmt *keys)
{
	int stat = 0;
	BulkCtx aesCtx;
int ix;

	if (initlock) {
		initlock = 0;
		p3lock_init(crypto_lock);
	}

sprintf(p3buf, "Call init crypto: %p, %d\n", keys->dnewkey->key, (keys->dnewkey->size));
p3errmsg(p3MSG_DEBUG, p3buf);
strcpy(p3buf, "Initial Key: ");
for (ix=0; ix < keys->dnewkey->size; ix++) {
sprintf(p3buf, "%s%2.2x", p3buf, keys->dnewkey->key[ix]);
}
sprintf(p3buf, "%s\n", p3buf);
p3errmsg(p3MSG_DEBUG, p3buf);
	// Get data encryption context (one each for encryption and decryption)
	if (!lockheld)
		p3lock(crypto_lock);
	if ((aesCtx = CreateAESCtx(MOC_SYM(hwAccelCtx) keys->dnewkey->key,
			keys->dnewkey->size, TRUE)) == NULL) {
		p3errmsg(p3MSG_ERR, "p3_init_crypto: Failed to create data crypto context\n");
		stat = -1;
		goto out;
	}
	keys->datenc1 = aesCtx;
	if ((aesCtx = CreateAESCtx(MOC_SYM(hwAccelCtx) keys->dnewkey->key,
			keys->dnewkey->size, FALSE)) == NULL) {
		p3errmsg(p3MSG_ERR, "p3_init_crypto: Failed to create data crypto context\n");
		stat = -1;
		goto out;
	}
	keys->datdec1 = aesCtx;
	// Get control encryption context (one each for encryption and decryption)
	if ((aesCtx = CreateAESCtx(MOC_SYM(hwAccelCtx) keys->cnewkey->key,
			keys->cnewkey->size, TRUE)) == NULL) {
		p3errmsg(p3MSG_ERR, "p3_init_crypto: Failed to create control crypto context\n");
		stat = -1;
		goto out;
	}
	keys->ctlenc1 = aesCtx;
	if ((aesCtx = CreateAESCtx(MOC_SYM(hwAccelCtx) keys->cnewkey->key,
			keys->cnewkey->size, FALSE)) == NULL) {
		p3errmsg(p3MSG_ERR, "p3_init_crypto: Failed to create control crypto context\n");
		stat = -1;
		goto out;
	}
	keys->ctldec1 = aesCtx;
	if (!lockheld)
		p3unlock(crypto_lock);

out:
p3errmsg(p3MSG_DEBUG, "Exit init crypto\n");
	return(stat);
} /* end p3_init_crypto */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <hr><b>p3_init_crypto: Failed to create <i>P3 type</i> crypto context</b>
 * \par Description (CRIT):
 * The cryptography context structure could not be created.  This
 * is most likely a system resource problem.
 * \par Response:
 * Troubleshoot the operating system problem.
 *
 */

/**
 * \par Function:
 * p3_rekey
 *
 * \par Description:
 * Update the key management structure after a P3 Rekey control message
 * has been completed.  This consists of releasing the oldest keys,
 * copying the current keys to the previous key fields, and
 * initializing the new keys.
 *
 * <i>Note that the data and control new key fields must contain the new keys
 *    to be used.</i>
 *
 * \par Inputs:
 * - keys: The session key managment structure.
 *
 * \par Outputs:
 * - int: Status
 *   - 0: OK
 *   - <0: Error
 */

int p3_rekey(p3keymgmt *keys)
{
	int stat = 0;

int i;
sprintf(p3buf, "Rekey: %p\n", keys);
p3errmsg(p3MSG_DEBUG, p3buf);
	// Release the oldest keys
sprintf(p3buf, " == DE1 %p, DE0 %p, DD1 %p, DD0 %p\n",
keys->datenc1, keys->datenc0, keys->datdec0, keys->datdec1);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, " == CE1 %p, CE0 %p, CD1 %p, CD0 %p\n",
keys->ctlenc1, keys->ctlenc0, keys->ctldec0, keys->ctldec1);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  New DKey: ");
for (i=0; i < 16; i++) {
	if (!(i & 3))
		sprintf(p3buf, "%s ", p3buf);
	sprintf(p3buf, "%s%2.2x", p3buf, keys->dnewkey->key[i]);
}
sprintf(p3buf, "%s\n", p3buf);
p3errmsg(p3MSG_DEBUG, p3buf);
sprintf(p3buf, "  New CKey: ");
for (i=0; i < 16; i++) {
	if (!(i & 3))
		sprintf(p3buf, "%s ", p3buf);
	sprintf(p3buf, "%s%2.2x", p3buf, keys->cnewkey->key[i]);
}
sprintf(p3buf, "%s\n", p3buf);
p3errmsg(p3MSG_DEBUG, p3buf);

	p3lock(crypto_lock);
    if (keys->datenc0 != NULL &&
			DeleteAESCtx(MOC_SYM(hwAccelCtx) (BulkCtx)keys->datenc0) < 0) {
		p3errmsg(p3MSG_ERR, "p3_rekey: Failed to release data crypto context\n");
    }
    if (keys->datdec0 != NULL &&
			DeleteAESCtx(MOC_SYM(hwAccelCtx) (BulkCtx)keys->datdec0) < 0) {
		p3errmsg(p3MSG_ERR, "p3_rekey: Failed to release data crypto context\n");
    }
    if (keys->ctlenc0 != NULL &&
			DeleteAESCtx(MOC_SYM(hwAccelCtx) (BulkCtx)keys->ctlenc0) < 0) {
		p3errmsg(p3MSG_ERR, "p3_rekey: Failed to release control crypto context\n");
    }
    if (keys->ctldec0 != NULL &&
			DeleteAESCtx(MOC_SYM(hwAccelCtx) (BulkCtx)keys->ctldec0) < 0) {
		p3errmsg(p3MSG_ERR, "p3_rekey: Failed to release control crypto context\n");
    }
// if (keys->datenc0 != NULL) {
// 	stat = -1;
// 	goto out;
// }

	// Set key 0 to key 1 values
	keys->datenc0 = keys->datenc1;
	keys->datdec0 = keys->datdec1;
	keys->ctlenc0 = keys->ctlenc1;
	keys->ctldec0 = keys->ctldec1;

	// Initialize the new keys
	lockheld = 1;
	stat = p3_init_crypto(keys);
	lockheld = 0;
	p3unlock(crypto_lock);

out:
	return (stat);
} /* end p3_rekey */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <hr><b>p3_rekey: Failed to release <i>P3 type</i> crypto context</b>
 * \par Description (ERR):
 * The cryptography context structure could not be released.  This
 * will probably lead to a system resource problem, however processing
 * continues.
 * \par Response:
 * Troubleshoot the operating system problem based on the reason code.
 *
 */

/**
 * \par Function:
 * p3_encrypt
 *
 * \par Description:
 * Encrypt a buffer.
 *
 * \par Inputs:
 * - buffer:  The buffer to be encrypted.  The encrypted data is
 *   returned in this buffer.
 * - size: The size of the buffer, in bytes, which must be a
 *   multiple of 16.
 * - id: The ID of the P3 packet.
 * - crypto: The session crypto type (ie. p3DATENC1, p3CTLENC0).
 *
 * \par Outputs:
 * - int: Status
 *   - 0: OK
 *   - <0: Error
 */

int p3_encrypt(unsigned char *buffer, int size, unsigned int id, int key, p3keymgmt *keys)
{
	int stat = 0;
	char *mocerr;
	unsigned char iv[16];
	void *ctx;

	// Initialize the IV to the packet ID
	iv[0] = iv[4] = iv[8] = iv[12] = (id >> 24) & 0xff;
	iv[1] = iv[5] = iv[9] = iv[13] = (id >> 16) & 0xff;
	iv[2] = iv[6] = iv[10] = iv[14] = (id >> 8) & 0xff;
	iv[3] = iv[7] = iv[11] = iv[15] = id & 0xff;

	// Set key
	p3lock(crypto_lock);
	if (key == p3DATENC1)
		ctx = keys->datenc1;
	else if (key == p3CTLENC1)
		ctx = keys->ctlenc1;
	else if (key == p3DATENC0)
		ctx = keys->datenc0;
	else if (key == p3CTLENC0)
		ctx = keys->ctlenc0;
	else {
		p3unlock(crypto_lock);
p3errmsg(p3MSG_DEBUG, "Bad Encrypt key type\n");
		stat = -1;
		goto out;
	}
	p3unlock(crypto_lock);

    if ((stat = DoAES(MOC_SYM(hwAccelCtx) (BulkCtx)ctx, buffer, size, TRUE, iv)) < 0) {
		if ((mocerr = MERROR_lookUpErrorCode(stat)) == NULL)
			mocerr = unknown_err;
    	sprintf(p3buf, "p3_encrypt: Failed to encrypt buffer: %s\n", mocerr);
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
    }

out:
	return (stat);
} /* end p3_encrypt */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <hr><b>p3_encrypt: Failed to encrypt buffer: <i>reason code</i></b>
 * \par Description (ERR):
 * There was an error encrypting a buffer.
 * \par Response:
 * Troubleshoot the operating system problem based on the reason code.
 *
 */

/**
 * \par Function:
 * p3_decrypt
 *
 * \par Description:
 * Decrypt a buffer.
 *
 * \par Inputs:
 * - buffer:  The buffer to be decrypted.  The decrypted data is
 *   returned in this buffer.
 * - size: The size of the buffer, in bytes, which must be a
 *   multiple of 16.
 * - id: The ID of the P3 packet.
 * - crypto: The session crypto type (ie. p3DATENC1, p3CTLENC0).
 *
 * \par Outputs:
 * - int: Status
 *   - 0: OK
 *   - <0: Error
 */

int p3_decrypt(unsigned char *buffer, int size, unsigned int id, int key, p3keymgmt *keys)
{
	int stat = 0;
	char *mocerr;
	unsigned char iv[16];
	void *ctx;

	// Initialize the IV to the packet ID
	iv[0] = iv[4] = iv[8] = iv[12] = (id >> 24) & 0xff;
	iv[1] = iv[5] = iv[9] = iv[13] = (id >> 16) & 0xff;
	iv[2] = iv[6] = iv[10] = iv[14] = (id >> 8) & 0xff;
	iv[3] = iv[7] = iv[11] = iv[15] = id & 0xff;

	// Set key
	p3lock(crypto_lock);
	if (key == p3DATDEC1)
		ctx = keys->datdec1;
	else if (key == p3CTLDEC1)
		ctx = keys->ctldec1;
	else if (key == p3DATDEC0)
		ctx = keys->datdec0;
	else if (key == p3CTLDEC0)
		ctx = keys->ctldec0;
	else {
		p3unlock(crypto_lock);
p3errmsg(p3MSG_DEBUG, "Bad Decrypt key type\n");
		stat = -1;
		goto out;
	}
	p3unlock(crypto_lock);

    if ((stat = DoAES(MOC_SYM(hwAccelCtx) (BulkCtx)ctx, buffer, size, FALSE, iv)) < 0) {
		if ((mocerr = MERROR_lookUpErrorCode(stat)) == NULL)
			mocerr = unknown_err;
    	sprintf(p3buf, "p3_decrypt: Failed to decrypt buffer: %s\n", mocerr);
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
    }

out:
	return (stat);
} /* end p3_decrypt */

/**
 * \page P3KM_MSGS Protected Point to Point Kernel Module Messages
 * <hr><b>p3_decrypt: Failed to decrypt buffer: <i>reason code</i></b>
 * \par Description (ERR):
 * There was an error encrypting a buffer.
 * \par Response:
 * Troubleshoot the operating system problem based on the reason code.
 *
 */

