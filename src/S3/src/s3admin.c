/**
 * \file s3admin.c
 * <h3>Secure Storage Solution adminstrator interface file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The adminstrator interface is used to manage the key server.
 */

/**
 * \page S3KS_MSGS Secure Storage Solution Administration Messages
 * <p><hr><hr>
 * \section S3_ADMIN S3 Administration Messages
 */

#define _s3ADMIN_C
#include "s3admin.h"
#include "s3ui.h"

/** Global variable usage */
s3admin *admin;
/** Working buffer */
char tbuf[4092], *admbuf = tbuf;

/**
 * \par Function:
 * main
 *
 * \par Description:
 * Initialize the S3 administration application.
 * 
 * \par Inputs:
 * - argc: Number of arguments
 * - argv: Argument vector
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */
int main(int argc, char *argv[])
{
	int s3status = 0;
	char msg_time[64];
	time_t now = time(NULL);
	const struct tm mtm, *msgtm = &mtm;

	// Create main s3 admin data structure
	if ((admin = (s3admin *) calloc(1, sizeof(s3admin))) == NULL) {
		MSG_TIME(msg_time, &now, msgtm)
		fprintf(stderr, "%s (CRITICAL)  s3admin: Failed to allocate main s3\
 admin data structure: %s\n", msg_time, strerror(errno));
		s3status = -1;
		goto out;
	}
	if (init_admin() < 0) {
		s3status = -1;
		goto out;
	}
	if (admin_handler() < 0) {
		s3status = -1;
		goto out;
	}

out:
	if (s3status > 0)
		s3status = 0;
	return (s3status);
} /* end S3 admin main */

/**
 * \page S3KS_MSGS Secure Storage Solution Administration Messages
 * <hr><b>s3admin: Failed to allocate main s3 admin data structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The S3 administration interface attempts to allocate its main structure
 * before any other activity.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */


/**
 * \par Function:
 * init_admin
 *
 * \par Description:
 * Initialize the adminstrator interface.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_admin()
{
	int ia_status = 0;

	// Open the FIFO to the application
	// NOTE: Attempt to open the key server FIFO first,
	//       if not successful attempt the secondary
	
	// Depending on which FIFO was opened, read the user interface configuration
	
	// Display the admin interface screen

out:
	return (ia_status);
} /* end init_admin */

/**
 * \par Function:
 * admin_handler
 *
 * \par Description:
 * Handle input from the adminstrator interface.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int admin_handler()
{
	int ah_status = 0;

	// Read input from the admin interface screen
	
	// Validate the input
	
	// Send interface with the S3 system application
	
	// Update display

	// Display message if there is an error
	
out:
	return (ah_status);
} /* end admin_handler */
