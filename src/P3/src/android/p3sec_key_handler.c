/**
 * \file p3sec_key_handler.c
 * <h3>Protected Point to Point secondary key handler file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The key handler provides true random keys for encryption.  It runs in
 * the background so it can constantly be producing keys to be instantly
 * available on request.
 */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <p><hr><hr>
 * \section P3_SEC_KEY Secondary P3 Key Handler Messages
 */

#define _p3SEC_KEY_HANDLER_C
#include "p3sec_key_handler.h"
#include "p3system.h"

/**
 * \par Function:
 * init_key_handler
 *
 * \par Description:
 * Create a buffer to communicate with the kernel module.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_key_handler()
{
	int stat = 0;

	// TODO: Create kernel module communication buffer

out:
	return (stat);
} /* end init_key_handler */

/**
 * \par Function:
 * key_handler
 *
 * \par Description:
 * When a session is initialized, the primary P3 system sends the initial
 * data and control keys.  Get these keys from the initialization message
 * and provide them to the kernel module using the communication buffer.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int key_handler()
{
	int stat = 0;

	// TODO: Handle new encryption keys

out:
	return (stat);
} /* end key_handler */

