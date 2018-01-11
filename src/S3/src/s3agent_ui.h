/**
 * \file s3agent_ui.h
 * <h3>Secure Storage Solution agent user interface header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _s3AGENT_UI_H
#define _s3AGENT_UI_H

/*****  INCLUDE FILES *****/

#include "s3base.h"

/*****  CONSTANTS  *****/

#define	FIFO_IN		"pui_in"	/**< read FIFO name */
#define	FIFO_OUT	"pui_out"	/**< write FIFO name */

/*****  DATA DEFINITIONS  *****/

typedef struct _s3agent_ui s3agent_ui;

/**
 * Structure:
 * s3agent_ui
 * 
 * \par Description:
 * Description.
 */

struct _s3agent_ui {
	int				fifo_in;	/**< Read FIFO file descriptor */
	int				fifo_out;	/**< Write FIFO file descriptor */
	unsigned int	flag;
};

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

int init_ui();
int ui_handler();

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _s3AGENT_UI_C
extern s3agent_ui *ui;
#endif

#endif /* _s3AGENT_UI_H */
