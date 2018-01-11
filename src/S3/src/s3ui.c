/**
 * \file s3ui.c
 * <h3>Secure Storage Solution key server user interface file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The key server user interface is used to manage the key server.
 */

/**
 * \page S3KS_MSGS Secure Storage Solution Messages
 * <p><hr><hr>
 * \section S3_UI S3 User Interface Messages
 */

#define _s3UI_C
#include "s3ui.h"
#include "s3key server.h"

/** Global variable usage */
s3ui *ui;

/**
 * \par Function:
 * init_ui
 *
 * \par Description:
 * Initialize the key server user interface.  The S3 key server application does not
 * interact directly with an administrator, the s3admin application does. Then
 * that application communicates changes to the S3 key server through a FIFO.  The
 * S3 key server interface then updates the configuration, and if appropriate,
 * dynamically updates the application.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_ui()
{
	int pu_stat = 0;

	// Create key server user interface data structure
	if ((ui = (s3ui *) calloc(1, sizeof(s3ui))) == NULL) {
		sprintf(s3buf, "init_ui: Failed to allocate key server user interface\
 data structure: %s\n", strerror(errno));
		s3errmsg (s3MSG_CRIT, s3buf);
		pu_stat = -1;
		goto out;
	}

	// Open FIFO
	// open ui->fifo_in
	// open ui->fifo_out

	// Give control to UI handler
	if (ui_handler() < 0) {
		pu_stat = -1;
		goto out;
	}

out:
	return (pu_stat);
} /* end init_ui */

/**
 * \page S3KS_MSGS Secure Storage Solution Messages
 * <hr><b>init_ui: Failed to allocate key server user interface data structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The S3 key server attempts to allocate the user interface structure during
 * initialization.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * ui_handler
 *
 * \par Description:
 * Wait for input from the administrator interface through a FIFO.  The
 * S3 key server interface then updates the configuration, and if appropriate,
 * dynamically updates the application.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int ui_handler()
{
	int uh_stat = 0;

	// Listen on read FIFO for input from adminstration interface

out:
	return (uh_stat);
} /* end ui_handler */

