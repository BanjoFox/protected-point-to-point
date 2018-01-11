/**
 * \file s3template.c
 * <h3>Secure Storage Solution template file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * Overview of the purpose of the functions in the source file.
 */

/**
 * \page S3TM_MSGS Secure Storage Solution Template Messages
 * <p><hr><hr>
 * \section S3_TEMPLATE S3 System Messages
 */

#include "s3base.h"

/** Global variable usage */
int s3template_global;

/**
 * \par Function:
 * s3function1
 *
 * \par Description:
 * description
 * 
 * \par Inputs:
 * - input1: description
 *
 * \par Outputs:
 * - int: description
 */

int s3function1(int input1)
{
	int i, j, f1_status = 0;

	// Inline comments
	i = input1;
	for (j=0; j < input1; j++) {
		i = input1 - j;
		if (i == j) {
			f1_status = -1;
			fprintf(stderr, "s3function1: Message text: %s", strerror(errno));
			goto out;
		}
	}

out:
	return (f1_status);
} /* end s3function1 */

/**
 * \page S3TM_MSGS Secure Storage Solution Template Messages
 * <hr><b>s3function1: Message text: <i>error reason</i></b>
 * \par Description (ERR):
 *   Explanation of what caused the message.
 * \par Response:
 *   Description of how to respond.
 */

/**
 * \par Function:
 * s3function2
 *
 * \par Description:
 * description
 * 
 * \par Inputs:
 * - input2: description
 *
 * \par Outputs:
 * - char *: description
 */

char *s3function2(int input2)
{
	char *f2_output = NULL;

	// Inline comments
	if ((f2_output = (char *) malloc(input2)) == NULL) {
		fprintf(stderr, "s3function2: Message text: %s", strerror(errno));
		goto out;
	}

out:
	return (f2_output);
} /* end s3function2 */

/**
 * \page S3TM_MSGS Secure Storage Solution Template Messages
 * <hr><b>s3function2: Message text: <i>error reason</i></b>
 * \par Description (CRIT):
 *   Failed to allocate the output buffer.
 * \par Response:
 *   Troubleshoot the operating system problem based on the error reason
 */
