/**
 * \file s3template.h
 * <h3>Secure Storage Solution template header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _s3_TEMPLATE_H
#define _s3_TEMPLATE_H

/*****  INCLUDE FILES *****/

#include "s3base.h"

/*****  CONSTANTS  *****/

/** Description is optional */
#define s3CONSTANT		0

/*****  DATA DEFINITIONS  *****/

typedef struct _s3struct s3struct;

/**
 * Structure:
 * s3struct
 * 
 * \par Description:
 * Description.
 */

struct _s3struct {
	s3struct		*next;	/**< List pointer */
	char 			*c1;	/**< Field usage */
	int				i1;		/**< Field usage */
	unsigned int	flag;
#define s3STR_FLG1	0x00000001
#define s3STR_FLG2	0x00000002
#define s3STR_FLG3	0x00000004
};

/*****  MACROS  *****/

/**
 * Macro:
 * S3MACRO
 *
 * \par Description:
 * Description.
 *
 * \par Parameters:
 * - s3parm1: Description
 */

#define S3MACRO(s3parm1) \
	printf("%x", s3parm1);

/*****  PROTOTYPES  *****/

int s3function1(int input1);
char *s3function2(int input2);

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _s3_TEMPLATE_C
extern int s3template_global;
#endif

#endif /* _s3_TEMPLATE_H */
