/**
 * \file p3fifo.h
 * <h3>Protected Point to Point product fifo header file</h3>
 *
 * Copyright (C) Velocite 2010
 */

#ifndef _p3_FIFO_H
#define _p3_FIFO_H

#include "p3utils.h"
#include <sys/ipc.h>
#include <sys/shm.h>

/*****  CONSTANTS  *****/

#define p3ANON_FNAME	"p3anon.temp"	/* Temporary storage of fifo data */

#define p3SHARE_SIZE	0x10000		/* Size of shared blocks (64K) */
#define p3SHARE_START	0x3000000	/* Starting location of shared blocks */

#define p3FIFO_READ		(O_RDONLY | O_NONBLOCK)
#define p3FIFO_WRITE	(O_WRONLY | O_NONBLOCK)
#define p3FIFO_MODE		(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define p3SHARE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

/*****  MACROS  *****/

/*****  DATA DEFINITIONS  *****/

typedef struct _p3fifo p3fifo;
typedef struct _p3fifomsg p3fifomsg;
typedef struct _p3shm_data p3shm_data;
typedef struct _p3fifoshare p3fifoshare;

/**
 * Structure:
 * p3fifo
 *
 * \par Description:
 * The information about the FIFO and shared memory block used
 * to communicate with the system application.
 */

struct _p3fifo {
	char			*pipe_in;	/**< Name of inbound pipe */
	char			*pipe_out;	/**< Name of outbound pipe */
	int				fifo_in;	/**< Read FIFO file descriptor */
	int				fifo_out;	/**< Write FIFO file descriptor */
	int				adm_gid;	/**< Administration group ID */
	int				pid;		/**< FIFO handler process ID */
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
#define p3FCMD_SHB	0x00000001	/* Message data is shared block info */
#define p3FCMD_RESP	0x00000002	/* Message data is response */
#define p3FCMD_ACK	0x0000000f	/* Message received successfully */
#define p3FMSG_RSVP	0x00000010	/* Wait for response */
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

/**
 * Structure:
 * p3fifoshare
 *
 * \par Description:
 * The information about a block of shared memory used to send a
 * response from the system application to the user interface.
 */

struct _p3fifoshare {
	p3fifoshare		*next;	/**< Shared block list pointer */
	void			*loc;	/**< Location of shared block */
	int				shmid;	/**< Shared memory ID of block */
	int				size;	/**< Size of block */
	unsigned int	accept;	/**< Indicates parent accepted shared block */
};

/*****  PROTOTYPES  *****/

int init_fifo();
int openFIFO();
int send_request(p3comm *request, p3fifomsg *fmsg);
int get_response(p3comm *request);
int fifo_handler();
int save_response();
p3fifoshare *get_fifo_share(p3shm_data *shmdata);
p3fifoshare *attach_share(p3shm_data *shmdata);
void fifo_shutdown(int status);

/*****  EXTERNAL DEFINITIONS  *****/

#endif /* _p3_FIFO_H */
