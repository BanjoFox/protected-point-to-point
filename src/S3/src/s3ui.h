/**
 * \file s3ui.h
 * <h3>Secure Storage Solution key server user interface header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _s3UI_H
#define _s3UI_H

/*****  INCLUDE FILES *****/

#include "s3base.h"

/*****  CONSTANTS  *****/

#define	FIFO_IN		"pui_in"	/**< read FIFO name */
#define	FIFO_OUT	"pui_out"	/**< write FIFO name */

/*****  DATA DEFINITIONS  *****/

typedef struct _s3ui s3ui;

/**
 * Structure:
 * s3ui
 * 
 * \par Description:
 * Description.
 */

struct _s3ui {
	int				fifo_in;	/**< Read FIFO file descriptor */
	int				fifo_out;	/**< Write FIFO file descriptor */
	unsigned int	flag;
};

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

int init_ui();
int ui_handler();

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _s3UI_C
extern s3ui *ui;
#endif

#endif /* _s3UI_H */
