/**
 * \file p3admin.h
 * <h3>Protected Point to Point adminstrator interface header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3ADMIN_H
#define _p3ADMIN_H

/*****  INCLUDE FILES *****/

#include "p3base.h"
#include "p3system.h"
#include <sys/ipc.h>

/*****  CONSTANTS  *****/

#define p3FIFO_READ		(O_RDONLY | O_NONBLOCK)
#define p3FIFO_WRITE	(O_WRONLY | O_NONBLOCK)
#define p3FIFO_MODE		(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define p3SHARE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

#define p3MAX_GROUPS		4
#define p3MAX_TOKENS		4

/*****  DATA DEFINITIONS  *****/

typedef struct _p3admin p3admin;
typedef struct _p3comm p3comm;
typedef struct _p3msgdata p3msgdata;
typedef struct _p3fifomsg p3fifomsg;
typedef struct _p3shm_data p3shm_data;

/**
 * Structure:
 * p3admin
 * 
 * \par Description:
 * Maintain the information to manage administration functions.
 */

struct _p3admin {
	char			*pipe1_in;	/**< Name of Level 1 inbound pipe */
	char			*pipe1_out;	/**< Name of Level 1 outbound pipe */
	int				fifo1_in;	/**< Read Level 1 FIFO file descriptor */
	int				fifo1_out;	/**< Write Level 1 FIFO file descriptor */
	int				adm_gid1;	/**< Administration Level 1 group ID */
	char			*pipe2_in;	/**< Name of Level 2 inbound pipe */
	char			*pipe2_out;	/**< Name of Level 2 outbound pipe */
	int				fifo2_in;	/**< Read Level 2 FIFO file descriptor */
	int				fifo2_out;	/**< Write Level 2 FIFO file descriptor */
	int				adm_gid2;	/**< Administration Level 2 group ID */
	char			*pipe3_in;	/**< Name of Level 3 inbound pipe */
	char			*pipe3_out;	/**< Name of Level 3 outbound pipe */
	int				fifo3_in;	/**< Read Level 3 FIFO file descriptor */
	int				fifo3_out;	/**< Write Level 3 FIFO file descriptor */
	int				adm_gid3;	/**< Administration Level 3 group ID */
	int				fid;		/**< Next FIFO message ID */
	unsigned int	flag;
};
#define p3FIFO1IN	"u2sfifo1"
#define p3FIFO1OUT	"s2ufifo1"
#define p3FIFO2IN	"u2sfifo2"
#define p3FIFO2OUT	"s2ufifo2"
#define p3FIFO3IN	"u2sfifo3"
#define p3FIFO3OUT	"s2ufifo3"
#define p3FIFONAMESZ	(sizeof(p3FIFO1IN) + sizeof(p3FIFO1OUT) +\
 sizeof(p3FIFO2IN) + sizeof(p3FIFO2OUT) +\
 sizeof(p3FIFO3IN) + sizeof(p3FIFO3OUT))

/**
 * Structure:
 * p3comm
 *
 * \par Description:
 * The administration communication structure maintains information
 * exchanged between the system application and the user interface.
 *
 * <i>Note that the datlen value is the length of the data buffer,
 * including the NULL termination character.</i>
 */

struct _p3comm {
	char	tokens[p3MAX_TOKENS];	/**< List of tokens in request/response */
	int		ID;						/**< Command ID for tracking */
	int		id1;					/**< ID depends on token */
	int		id2;					/**< ID depends on token */
	int		datlen;					/**< Length of data string, including NULL */
	char	*data;					/**< Data string */
};

/**
 * Structure:
 * p3msgdata
 *
 * \par Description:
 * The information about a specific FIFO and shared memory block.
 */

struct _p3msgdata {
	char			*pipe_in;	/**< Name of inbound pipe */
	char			*pipe_out;	/**< Name of outbound pipe */
	int				fifo_in;	/**< Read FIFO file descriptor */
	int				fifo_out;	/**< Write FIFO file descriptor */
	int				adm_gid;	/**< Administration group ID */
	unsigned int	flag;
// p3FMSG_PERM		0x00000020	/* Shared block is permanent */
};

/**
 * Structure:
 * p3fifomsg
 *
 * \par Description:
 * The data passed between the P3 system application and the
 * user interface through the FIFO.
 */

struct _p3fifomsg {
	int				datlen;		/**< Length of data immediately following p3fifomsg */
	unsigned int	flag;
#define p3FMSG_CMD	0x0000000f	/* Message command field */
#define p3FCMD_REQ	0x00000002	/* Message data is request */
#define p3FCMD_RESP	0x00000004	/* Message data is response */
#define p3FCMD_ACK	0x0000000f	/* Message received successfully */
#define p3FMSG_RSVP	0x00000010	/* Message command requires response */
#define p3FMSG_PERM	0x00000020	/* Shared block is permanent */
};

/**
 * Structure:
 * p3shm_data
 *
 * \par Description:
 * The information required to access a shared memory block for
 * communication between the P3 system application and the
 * user interface.
 */

struct _p3shm_data {
	int				shmid;		/**< Shared block ID */
	int				size;		/**< Size of shared block */
};

#define p3SHARE_MSG_SIZE	(sizeof(p3fifomsg) + sizeof(p3shm_data))

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

int init_admin();
int openFIFO();
int admin_handler();
p3comm *get_request();
int send_response(p3comm *response, p3fifomsg *fmsg, p3msgdata *msgdata);
int get_comm_data(p3comm *comm, int size);

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _p3ADMIN_C
extern p3admin *admin;
#endif

#endif /* _p3ADMIN_H */
