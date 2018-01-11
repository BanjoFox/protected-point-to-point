/**
 * \file p3fifo.c
 * <h3>Protected Point to Point fifo connections program file</h3>
 *
 * Copyright (C) Velocite 2010
 *
 * The P3 fifo functions manage fifo connections from
 * P3 secondaries to P3 primaries to automatically supply remote
 * host configuration data.
 */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <p><hr><hr>
 * \section P3_FIFO P3 FIFO Handler Messages
 */

#define _p3_FIFO_C	1
#include "p3fifo.h"
#include "p3admin.h"

/** Local FIFO information */
p3fifo *admfifo = NULL;

/** Share memory list anchor */
p3fifoshare *fifo_share = NULL;
p3fifoshare *share_tail = NULL;
/* End of shared blocks */
void *sharemax;

/**
 * \par Function:
 * init_fifo
 *
 * \par Description:
 * Initialize the fifo connection functions.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_fifo()
{
	int i, stat = 0;
	unsigned long l;

	if (command->config == NULL) {
		p3errmsg (p3MSG_ERR, "init_fifo: No configuration data\n");
		stat = -1;
		goto out;
	}

	i = sizeof(p3fifo) + (2 * (strlen(admin->home)))
		+ strlen(admin->config->local->s2uname)
		+ strlen(admin->config->local->s2uname) + 10;
	if (admfifo == NULL && (admfifo = (p3fifo *) p3calloc(i)) == NULL) {
		sprintf(p3buf, "init_fifo: Failed to allocate fifo\
 structures: %s\n", strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	l = (unsigned long) admfifo + sizeof(p3fifo);
	admfifo->pipe_in = (char *) l;
// TODO: *** Temporary ***
	sprintf(admfifo->pipe_in, "%s%c%s%d", admin->home, PATH_SEPARATOR,
		admin->config->local->s2uname, 1);
//		admin->config->local->s2uname, admin->admuser->level);
	l = (unsigned long) admfifo->pipe_in + strlen(admfifo->pipe_in) + 1;
	admfifo->pipe_out = (char *) l;
	sprintf(admfifo->pipe_out, "%s%c%s%d", admin->home, PATH_SEPARATOR,
		admin->config->local->u2sname, admin->admuser->level);
	if (openFIFO() < 0) {
		stat = -1;
		goto out;
	}

out:
	return(stat);
}

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>init_fifo: Failed to allocate fifo structures:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 fifo connection handler attempts to allocate FIFO management
 * structures.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 *   openFIFO
 *
 * \par Description:
 *   Open the FIFO pipe to/from the user interface and get the group ID
 *   for each level.  The group ID is used to determine the level of access
 *   allowed to the administration user.  It is assumed that the FIFO has been
 *   created during installation and the permissions are set appropriately for
 *   the authorization level, although checking for consistency is performed.
 *
 * \par Inputs:
 *   - None
 *
 * \par Outputs:
 *   - int: Status
 *	 - 0 = Open successful
 *	 - <0 = Error
 *	 - >0 = Warning
 */

int openFIFO()
{
	int stat = 0;
	struct stat fifostat;

printf("DEBUG: Open FIFO in: %s\n", admfifo->pipe_in);
	if ((admfifo->fifo_in = open(admfifo->pipe_in, p3FIFO_READ)) < 0) {
		sprintf(p3buf, "openFIFO: Failed to open Level 1 administration\
 FIFO for reading: %s\n", strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	} else if (fstat(admfifo->fifo_in, &fifostat) < 0) {
		sprintf(p3buf, "openFIFO: Failed to get Level 1 administration\
 FIFO group ID: %s\n", strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	} else
		admfifo->adm_gid = fifostat.st_gid;
	if ((admfifo->fifo_out = open(admfifo->pipe_out, p3FIFO_WRITE)) < 0) {
		sprintf(p3buf, "openFIFO: Failed to open Level 1 administration\
 FIFO for writing: %s\n", strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	} else if (fstat(admfifo->fifo_out, &fifostat) < 0 ) {
		sprintf(p3buf, "openFIFO: Failed to get Level 1 administration\
 FIFO group ID: %s\n", strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	} else if (fifostat.st_gid != admfifo->adm_gid) {
		p3errmsg(p3MSG_ERR,
			"openFIFO: Level 1 administration FIFO group ID mismatch\n");
		stat = -1;
		goto out;
	}

out:
  return(stat);

} /* end openFIFO */

 /** \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>openFIFO: Failed to open <i>access level</i> administration FIFO for
 *   <i>reading/writing</i>: <i>error reason</i></b>
 * \par Description (ERR):
 *   Opening the FIFO for communication between the system application
 *   and user interface failed.
 * \par Response:
 *   Troubleshoot system problem based on the error reason.
 *
 * <hr><b>openFIFO: Failed to get <i>access level</i> administration FIFO
 *   group ID: <i>error reason</i></b>
 * \par Description (ERR):
 *   Getting the FIFO group ID which determines the level of access allowed
 *   to the administration user failed.
 * \par Response:
 *   Troubleshoot system problem based on the error reason.
 *
 * <hr><b>openFIFO: <i>Access level</i> administration FIFO group ID mismatch</b>
 * \par Description (ERR):
 *   The FIFO group ID for the FIFO from the P3 system application to the
 *   user interface is different from the group ID for the opposite direction
 *   FIFO at the same level.
 * \par Response:
 *   Correct the FIFO group definitions.
 *
 */

/**
 * \par Function:
 * send_request
 *
 * \par Description:
 * Send a request to the system application through the FIFO.
 *
 * \par Inputs:
 * - request: The request data to be sent.
 * - fmsg: The FIFO message information determining how the
 *   request is formatted.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int send_request(p3comm *request, p3fifomsg *fmsg)
{
	int stat = 0, numfds;
	fd_set fdset;
	struct timeval tv;

	fmsg->datlen = sizeof(p3comm) + request->datlen;

	if (write(admfifo->fifo_out, (void *) fmsg, sizeof(p3fifomsg)) < 0) {
		sprintf(p3buf, "send_request: Error sending request to system\
  application: %s\n", strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	}
	if (write(admfifo->fifo_out, (void *) request, sizeof(p3comm)) < 0) {
		sprintf(p3buf, "send_request: Error sending request to system\
 application:  %s\n", strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	}
	if (write(admfifo->fifo_out, request->data, request->datlen) < 0) {
		sprintf(p3buf, "send_request: Error sending request to system\
 application:  %s\n", strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	}

	if (fmsg->flag & p3FMSG_RSVP) {
		FD_ZERO(&fdset);
		numfds = admfifo->fifo_in + 1;
		FD_SET(admfifo->fifo_in, &fdset);
		// Wait up to 2 seconds(?)
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		if ((stat = select(numfds, &fdset, 0, 0, &tv)) < 0) {
			sprintf(p3buf, "send_request: Error in fifo listener: %s\n",
				strerror(errno));
			p3errmsg(p3MSG_ERR, p3buf);
			stat = -1;
			goto out;
		}
		// Handle FIFO response
		if (admfifo->fifo_in > 0 && FD_ISSET(admfifo->fifo_in, &fdset)) {
			tv.tv_sec = 0;
			tv.tv_usec = p3MILLISEC(10);
			if ((stat = select(0, 0, 0, 0, &tv)) < 0) {
				p3errmsg(p3MSG_ERR,
					"send_request: Error waiting for FIFO writes\n");
				stat = -1;
				goto out;
			}
			stat = get_response(request);
		} else {
			p3errmsg(p3MSG_ERR, "send_request: Fifo listener timed out\n");
			stat = -1;
		}
	} else {
		stat = fifo_handler();
	}

out:
	return(stat);
} /* end send_request */

/** \page P3ADMIN_MSGS Protected Point to Point Administration Messages
* <hr><b>send_request: Error sending request to system application:
*   <i>error reason</i></b>
* \par Description (ERR):
*   Writing a request to the FIFO for the system application failed.
* \par Response:
*   Troubleshoot system problem based on the error reason.
*
* <hr><b>send_request: Error in fifo listener: <i>error reason</i></b>
* \par Description (ERR):
*   There was an error while waiting for the FIFO response from
*   the system application.
* \par Response:
*   Troubleshoot system problem based on the error reason.
*
* <hr><b>send_request: Fifo listener timed out</b>
* \par Description (ERR):
*   The listener waits for the FIFO response from the system application
*   for up to 2 seconds.
* \par Response:
*   There may be a problem with the system application.  Otherwise, try
*   again later.
*
*/

/**
 * \par Function:
 * get_response
 *
 * \par Description:
 * Wait for a response from the system application and forward to
 * user interface application.
 *
 * \par Inputs:
 * - request: The request being waited on.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int get_response(p3comm *request)
{
	int sz, stat = 0;
	p3fifomsg fifomsg, *fmsg = &fifomsg;
	p3comm *response;

	if ((sz = read(admfifo->fifo_in, (void *) fmsg, sizeof(p3fifomsg))) < 0) {
		sprintf(p3buf,
			"get_response: Error receiving message from system application: %s\n",
			strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
	} else if (!sz || fmsg->datlen < sizeof(p3comm)) {
	p3errmsg(p3MSG_ERR, "get_response: Error receiving message from system\
 application:  Length less than communication structure\n");
	// Message response in shared block
	} else if ((response = (p3comm *) p3malloc(sizeof(p3comm))) == NULL) {
		sprintf(p3buf,
			"get_response: Failed to allocate fifo message buffer: %s\n",
			strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
	} else if ((sz = read(admfifo->fifo_in, (void *) response, sizeof(p3comm))) < 0) {
		sprintf(p3buf,
			"get_response: Error receiving response from system application:  %s\n",
			strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
	} else if (sz < sizeof(p3comm)) {
		p3errmsg(p3MSG_ERR, "get_response: Error receiving response from system\
 application:  Length less than communication structure\n");
		free(response);
		stat = -1;
	} else if (response->ID == request->ID) {
		if (response->datlen > 0) {
			if ((response->data = (char *) p3malloc(response->datlen)) == NULL) {
				sprintf(p3buf,
					"get_response: Failed to allocate fifo data buffer: %s\n",
					strerror(errno));
				p3errmsg (p3MSG_CRIT, p3buf);
				stat = -1;
				goto out;
			} else if ((sz = read(admfifo->fifo_in,
					(void *) response->data, response->datlen)) < 0) {
				sprintf(p3buf, "get_response: Error receiving response data\
 from system application:  %s\n", strerror(errno));
				p3errmsg(p3MSG_ERR, p3buf);
				free(response->data);
				free(response);
				response = NULL;
				stat = -1;
				goto out;
			} else if (sz < response->datlen) {
				p3errmsg(p3MSG_ERR, "get_response: Error receiving data from\
 system application:  Length less than size\n");
				free(response->data);
				free(response);
				stat = -1;
				goto out;
			}
		// TODO: Forward response to UI
		} else
			response->data = NULL;
// TODO: *** Temporary ***
printf("Response (%d) Tokens: %d %d %d %d, IDs: %d %d, Data(%d)\n  |%s|\n",
	response->ID, response->tokens[0], response->tokens[1], response->tokens[2],
	response->tokens[3], response->id1, response->id2, response->datlen, response->data);
		free(response->data);
		free(response);
	} else {
printf("DEBUG: Req ID %d != Resp ID %d\n", request->ID, response->ID);
	}

out:
	return(stat);
} /* end get_response */

/** \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>get_response: Failed to allocate fifo message buffer:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 fifo connection handler attempts to allocate a buffer for FIFO
 * messages.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>get_response: Error reading response from system application:
 *   <i>error reason</i></b>
 * \par Description (ERR):
 *   Reading a response from the FIFO for the system application failed.
 * \par Response:
 *   Troubleshoot system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * fifo_handler
 *
 * \par Description:
 * Listen for and process fifo connections and
 * fifo data from the P3 system application.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int fifo_handler()
{
	int stat = 0, numfds;
	fd_set fdset;
	struct timeval tv;
	struct sigaction p3sigact;

	// FIFO handler is already running
	if (admfifo->pid <= 0)
		goto out;

	// Spawn a child process to listen for secondary connections
	if ((admfifo->pid = fork()) < 0) {
		sprintf(p3buf, "init_fifo: Attempt to start\
fifo connection monitoring failed: %s\n", strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	// FIFO listener
	} else if (admfifo->pid == 0) {
		// Set interrupt handler
		p3signal(SIGQUIT, fifo_shutdown, p3sigact, stat);
		p3signal(SIGTERM, fifo_shutdown, p3sigact, stat);
		p3signal(SIGPIPE, fifo_shutdown, p3sigact, stat);
		if (stat < 0) {
			sprintf(p3buf, "fifo_handler: Error setting interrupt\
 handler: %s\n", strerror(errno));
			p3errlog(p3MSG_ERR, p3buf);
			stat = -1;
			goto out;
		}

		// Loop forever listening for FIFO responses
		while (1) {
			FD_ZERO(&fdset);
			numfds = admfifo->fifo_in + 1;
			FD_SET(admfifo->fifo_in, &fdset);
			// Wake up at least once every 10 seconds(?)
			tv.tv_sec = 10;
			tv.tv_usec = 0;
			if ((stat = select(numfds, &fdset, 0, 0, &tv)) < 0) {
				if (errno == EINTR)
					continue;
				sprintf(p3buf, "fifo_handler: Error in fifo\
 connection monitor: %s\n", strerror(errno));
				p3errlog(p3MSG_ERR, p3buf);
				fifo_shutdown(-1);
			}

			// Handle FIFO response
			if (admfifo->fifo_in > 0 && FD_ISSET(admfifo->fifo_in, &fdset)) {
				save_response();
			}
		}
	} else {
		p3errmsg(p3MSG_NOTICE, "Starting fifo connection monitoring\n");
	}

out:
	return(stat);
} /* end fifo_handler */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>fifo_handler:  Error setting interrupt handler:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 fifo connection monitor attempts to set an interrupt handler
 * to shutdown cleanly.  If there is an error, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>fifo_handler:  Error in fifo connection monitor:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 fifo connection monitor listens for secondary requesting
 * a connection.  If there is an error, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * save_response
 *
 * \par Description:
 * Save responses from the system application in a queue
 * for the parent user interface to process.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int save_response()
{
	int stat = 0;
	char mbuf[sizeof(p3fifomsg)], sbuf[sizeof(p3shm_data)];
	p3fifomsg *fmsg;
	p3shm_data sdat, *shmdata;
	p3fifoshare *share;

	if (read(admfifo->fifo_in, mbuf, sizeof(p3fifomsg)) < 0) {
		sprintf(p3buf, "save_response: Error reading response from system\
 application:  %s\n", strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	}
	fmsg = (p3fifomsg *) mbuf;
	// Response in shared block
	if ((fmsg->flag & p3FMSG_CMD) == p3FCMD_SHB) {
		if (read(admfifo->fifo_in, sbuf, sizeof(p3shm_data)) < 0) {
			sprintf(p3buf, "save_response: Error reading response from system\
	 application:  %s\n", strerror(errno));
			p3errmsg(p3MSG_ERR, p3buf);
			stat = -1;
			goto out;
		}
		shmdata = (p3shm_data *) sbuf;
		if ((share = attach_share(shmdata)) == NULL) {
			stat = -1;
			goto out;
		}

	// Response in FIFO message
	} else {
		// Get a FIFO shared block and store response there
		shmdata = &sdat;
		shmdata->size = fmsg->datlen;
		if ((share = get_fifo_share(shmdata)) == NULL) {
			stat = -1;
			goto out;
		}
		if (read(admfifo->fifo_in, share->loc, fmsg->datlen) < 0) {
			sprintf(p3buf, "get_response: Error receiving response from\
 system application:  %s\n", strerror(errno));
			p3errmsg(p3MSG_ERR, p3buf);
			stat = -1;
			goto out;
		}
	}

out:
	return(stat);
} /* end save_response */

/** \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>save_response: Error reading response from system application:
 *   <i>error reason</i></b>
 * \par Description (ERR):
 *   Reading a response from the FIFO for the system application failed.
 * \par Response:
 *   Troubleshoot system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * get_fifo_share
 *
 * \par Description:
 * Attach all shared blocks not already attached.
 *
 * \par Inputs:
 * - shmdata: The data for the shared block from the system application.
 *
 * \par Outputs:
 * - p3fifoshare *: If successful, the P3 FIFO shared block information.
 *   Otherwise, NULL is returned.
 */

p3fifoshare *get_fifo_share(p3shm_data *shmdata)
{
	p3fifoshare *newshare = NULL;

printf("DEBUG: Shmget\n");
	if ((shmdata->shmid = shmget(0, shmdata->size, IPC_CREAT+0660)) < 0) {
		sprintf(p3buf, "get_fifo_share: Error getting new shared\
 memory block: %s\n", strerror(errno));
		p3errlog(p3MSG_CRIT, p3buf);
		goto out;
	}
printf("DEBUG: Shmctl\n");
	shmctl(shmdata->shmid, IPC_RMID, NULL);

	newshare = attach_share(shmdata);

out:
	return(newshare);
} /* end attach_share */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>get_fifo_share: Failed to allocate fifo shared memory structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 fifo connection handler attempts to allocate a shared block
 * structure to get a response from the system application.  If this
 * fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>get_fifo_share:  Error getting new shared memory block:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 fifo connection handler attempts to allocate a shared memory
 * block to forward a small response from the system application to the
 * user interface.  If there is an error, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */


/**
 * \par Function:
 * attach_share
 *
 * \par Description:
 * Attach or allocate a shared blocks and put the information about it
 * on the list of shared blocks.
 *
 * \par Inputs:
 * - shmdata: The data for the shared block from the system application.
 *   Note that if the shmid field is -1, the shared block is simply
 *   allocated in local memory.
 *
 * \par Outputs:
 * - p3fifoshare *: If successful, the P3 FIFO shared block information.
 *   Otherwise, NULL is returned.
 */

p3fifoshare *attach_share(p3shm_data *shmdata)
{
	p3fifoshare *share = NULL;

	// Attach shared block
	if ((share = (p3fifoshare *)
			p3calloc(sizeof(p3fifoshare))) == NULL) {
		sprintf(p3buf, "attach_share: Failed to allocate fifo\
 shared memory structure: %s\n", strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		goto out;
	}
	if (shmdata->shmid > 0) {
printf("DEBUG: Attach share\n");
		if ((share->loc = (void *)
				shmat(shmdata->shmid, NULL, SHM_RND)) == (void *) -1) {
			sprintf(p3buf, "attach_share: Error attaching shared\
 memory block: %s\n", strerror(errno));
			p3errlog(p3MSG_CRIT, p3buf);
			free(share);
			share = NULL;
			goto out;
		}
	} else {
printf("DEBUG: Alloc local\n");
		if ((share->loc = (void *)
				p3calloc(shmdata->size)) == NULL) {
			sprintf(p3buf, "attach_share: Failed to allocate local\
 fifo shared block: %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			goto out;
		}
	}
	share->shmid = shmdata->shmid;
	share->size = shmdata->size;

	if (fifo_share == NULL)
		fifo_share = share;
	else
		share_tail->next = share;
	share_tail = share;

out:
	return(share);
} /* end attach_share */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>attach_share: Failed to allocate fifo shared memory structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 fifo connection handler attempts to allocate a shared block
 * structure to get a response from the system application.  If this
 * fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>attach_share:  Error attaching shared memory block:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 fifo connection handler attempts to attach a shared memory
 * block used by the system application to respond to a request.  If
 * there is an error, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>attach_share: Failed to allocate local fifo shared block:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 fifo connection handler attempts to allocate a local shared
 * block to store a response from the system application.  If this
 * fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * fifo_shutdown
 *
 * \par Description:
 * Shutdown the fifo connection handler cleanly.
 *
 * \par Inputs:
 * - status:  The status code for shutting down
 *
 * \par Outputs:
 * - None
 */

void fifo_shutdown(int status)
{
	p3fifoshare *share;

	close(admfifo->fifo_in);

	// Detach shared blocks
	share = fifo_share;
	while (share != NULL) {
		shmdt(share->loc);
		fifo_share = share->next;
		free(share);
		share = fifo_share;
	}

	exit(status);
} /* end fifo_shutdown */

