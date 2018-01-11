/**
 * \file s3utils.c
 * <h3>Secure Storage Solution utils file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * Utilities used by all S3 applications.
 */

/**
 * \page S3MSGS Secure Storage Solution Messages
 * <p><hr><hr>
 * \section S3_UTILS S3 System Messages
 */

#define _s3_UTILS_C
#include "s3utils.h"

/** Global variable usage */
s3util *s3utils;

/**
 * \par Function:
 * init_utils
 *
 * \par Description:
 * Initialize utilities for the S3 system.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_utils()
{
	int iu_stat = 0;

	if ((s3utils = (s3util *) calloc(1, sizeof(s3util))) == NULL) {
		fprintf(stderr, "init_utils: Failed to allocate s3 utilities data\
 structure: %s\n", strerror(errno));
		iu_stat = -1;
		goto out;
	}

out:
	return(iu_stat);
} /* end s3errmsg */

/**
 * \page S3MSGS Secure Storage Solution Messages
 * <hr><b>init_utils: Failed to allocate main s3 utilities data structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The S3 system attempts to allocate the utilities structure during
 * initialization.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * s3errmsg
 *
 * \par Description:
 * Handle error messages for the S3 system.  The types of messages are
 * defined in the s3base.h include file.  They are:
 * <ul>
 *  <li>Critical:  Caused by system errors</li>
 *  <li>Error:  Caused by application errors</li>
 *  <li>Warning:  Does not cause application shutdown, but indicate the
 *      need for adminstrative correction</li>
 *  <li>Notice:  Information that is always reported, such as startup</li>
 *  <li>Information:  Information such as usage statistics.  This may be
 *      ignored using the -I command line argument</li>
 * </ul>
 * 
 * \par Inputs:
 * - type: The type of the message
 * - message: The text of the message
 *
 * \par Outputs:
 * - None
 */

void s3errmsg(int type, char *message)
{
	char msg_time[64], msg_type[32];
	time_t now = time(NULL);
	const struct tm mtm, *msgtm = &mtm;

	if (type == s3MSG_CRIT)
		strcpy(msg_type, "CRITICAL");
	else if (type == s3MSG_ERR)
		strcpy(msg_type, "ERROR");
	else if (type == s3MSG_WARN)
		strcpy(msg_type, "WARNING");
	else if (type == s3MSG_NOTICE)
		strcpy(msg_type, "NOTICE");
	else if (type == s3MSG_INFO) {
		if (s3utils->flag & s3UTL_NINFO)
			goto out;
		strcpy(msg_type, "INFORMATION");
	}
	
	MSG_TIME(msg_time, &now, msgtm)
	fprintf(stderr, "%s (%s)  s3function1: Message text: %s",
		msg_time, msg_type, message);

out:
	return;
} /* end s3errmsg */

