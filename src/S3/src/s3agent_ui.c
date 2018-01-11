/**
 * \file s3agent_ui.c
 * <h3>Secure Storage Solution agent user interface file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The agent user interface is used to manage the agent.
 */

/**
 * \page S3AG_MSGS Secure Storage Solution Agent Messages
 * <p><hr><hr>
 * \section S3AG__UI S3 Agent User Interface Messages
 */

#define _s3AGENT_UI_C
#include "s3agent_ui.h"
#include "s3agent.h"

/** Global variable usage */
s3agent_ui *ui;

/**
 * \par Function:
 * init_ui
 *
 * \par Description:
 * Initialize the agent user interface.  The S3 agent application does not
 * interact directly with an administrator, the s3admin application does. Then
 * that application communicates changes to the S3 agent through a FIFO.  The
 * S3 agent interface then updates the configuration, and if appropriate,
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

	// Create agent user interface data structure
	if ((ui = (s3agent_ui *) calloc(1, sizeof(s3agent_ui))) == NULL) {
		sprintf(s3buf, "init_ui: Failed to allocate agent user interface\
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
 * \page S3AG_MSGS Secure Storage Solution Agent Messages
 * <hr><b>init_ui: Failed to allocate agent user interface data structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The S3 agent attempts to allocate the user interface structure during
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
 * S3 agent interface then updates the configuration, and if appropriate,
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

