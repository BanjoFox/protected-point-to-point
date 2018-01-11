/**
 * \file p3pri_key_server.c
 * <h3>Protected Point to Point primary key server file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The key server manages true random keys for encryption.  It constantly
 * produces keys which are placed on the circular buffer shared with the
 * kernel module.  Whenever the circular buffer has slots for keys the
 * key server fills all slots with new keys.
 */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <p><hr><hr>
 * \section P3_PRI_KEY Primary P3 Key Server Messages
 */

#define _p3PRI_KEY_SERVER_C
#include "p3pri_key_server.h"
#include "p3system.h"
#include "p3utils.h"
#include "p3crypto.h"

/** The main key server data structure */
p3key_serv *key_serv = NULL;
/** The key server uses a single key structure */
p3key keysrv_key, *ks_key = &keysrv_key;

/**
 * \par Function:
 * init_key_serv
 *
 * \par Description:
 * Initialize the key server data structure and the circular buffer
 * shared with the kernel module.  This memory includes the key server
 * data structure at the beginning, followed by the circular buffer.
 *
 * The circular buffer is an array of pointers to keys, because the
 * keys used by different sessions can be different sizes.  The kernel
 * module can use a key if the head and tail indexes are not equal.
 * 
 * \par Inputs:
 * - kserv: The location of the key server structure in the mmap'ed buffer.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_key_serv(p3key_serv *kserv)
{
	int stat = 0;
	unsigned long l;
	unsigned char *cbuf;

	if (kserv == NULL) {
		p3errmsg(p3MSG_ERR, "init_key_serv: Key server structure location NULL\n");
		stat = -1;
		goto out;
	}

	// Initialize keys for the kernel module
	key_serv = kserv;
	ks_key->size = p3MIN_KSIZE;
	stat = buffer_handler();
	if ((stat = p3_get_key(ks_key)) < 0) {
		stat = -1;
		goto out;
	} else if (stat > 0) {
		goto out;
	}
	l = (unsigned long) key_serv + sizeof(p3key_serv);
	cbuf = (unsigned char *) l;
	memcpy(&cbuf[key_serv->tail], ks_key->key, ks_key->size);
	key_serv->tail += ks_key->size;

out:
	return (stat);
} /* end init_key_serv */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>init_key_serv: Key server structure location NULL</b>
 * \par Description (ERR):
 * The P3 primary initializes the mmap'ed buffer shared between the
 * user space appliation and the kernel module and populates it with
 * keys.  If there is a problem in the mmap function, the initialization
 * cannot be performed.
 * \par Response:
 * Report the problem to Velocite Systems support.
 *
 */

/**
 * \par Function:
 * buffer_handler
 *
 * \par Description:
 * Handle the circular buffer shared with the kernel module.
 * This is done when the tail index is not equal to the head index.
 * In this case, a new key is created in the current head slot
 * and the head pointer is incremented to indicate that the key
 * is available.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int buffer_handler()
{
	int stat = 0;
	unsigned long l;
	unsigned char *cbuf;

	if (key_serv == NULL)
		goto out;
	l = (unsigned long) key_serv + sizeof(p3key_serv);
	cbuf = (unsigned char *) l;

	// Add new keys and update tail pointer
	while ((key_serv->head < key_serv->tail &&
			key_serv->tail <= (key_serv->cbuf_sz - ks_key->size)) ||
			(0 < key_serv->head && key_serv->tail <
			(key_serv->head - ks_key->size))) {
		if ((stat = p3_get_key(ks_key)) < 0) {
			stat = -1;
			goto out;
		} else if (stat > 0) {
			goto out;
		}
		memcpy(&cbuf[key_serv->tail], ks_key->key, ks_key->size);
		key_serv->tail += ks_key->size;
		if (key_serv->tail > (key_serv->cbuf_sz - ks_key->size)) {
p3errmsg(p3MSG_DEBUG, "Cbuf tail wrapped\n");
			key_serv->tail = 0;
		}
	}

out:
	return (stat);
} /* end buffer_handler */

