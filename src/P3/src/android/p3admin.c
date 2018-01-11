/**
 * \file p3admin.c
 * <h3>Protected Point to Point adminstrator interface file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The adminstrator interface is used to manage the key server as well as
 * secondary P3 systems.
 */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <p><hr><hr>
 * \section P3_ADMIN P3 Administration Messages
 */

#define _p3ADMIN_C
#include "p3admin.h"
#include "p3system.h"
#include "p3utils.h"

/** Administration data structure */
p3admin *admin = NULL;

/** Working buffer */
char tbuf[4092], *admbuf = tbuf;

/**
 * \par Function:
 * init_admin
 *
 * \par Description:
 * Initialize the adminstrator interface.  This includes allocating
 * the administration data structure, opening the FIFO to communicate
 * with the user interface, and allocating the shared memory block to
 * send responses to the user interface.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 *   - int: Status
 *	 - 0 = Success
 *	 - <0 = Error
 */

int init_admin()
{
	int i, stat = 0;
	unsigned long l;

	// Create main p3 admin data structure
	i = sizeof(p3admin) + (6 * (sizeof(p3main->home) + sizeof(p3FIFO1OUT) + 2));
	if ((admin = (p3admin *) p3calloc(i)) == NULL) {
		sprintf(p3buf, "p3admin: Failed to allocate main p3\
 administration data structure: %s\n", strerror(errno));
		p3errmsg(p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	l = (unsigned int) admin + sizeof(p3admin);
	admin->pipe1_in = (char *) l;
	sprintf(admin->pipe1_in, "%s%c%s", p3main->home, PATH_SEPARATOR, p3FIFO1IN);
	l += strlen(admin->pipe1_in) + 1;
	admin->pipe1_out = (char *) l;
	sprintf(admin->pipe1_out, "%s%c%s", p3main->home, PATH_SEPARATOR, p3FIFO1OUT);
	l += strlen(admin->pipe1_out) + 1;
	admin->pipe2_in = (char *) l;
	sprintf(admin->pipe2_in, "%s%c%s", p3main->home, PATH_SEPARATOR, p3FIFO2IN);
	l += strlen(admin->pipe2_in) + 1;
	admin->pipe2_out = (char *) l;
	sprintf(admin->pipe2_out, "%s%c%s", p3main->home, PATH_SEPARATOR, p3FIFO2OUT);
	l += strlen(admin->pipe2_out) + 1;
	admin->pipe3_in = (char *) l;
	sprintf(admin->pipe3_in, "%s%c%s", p3main->home, PATH_SEPARATOR, p3FIFO3IN);
	l += strlen(admin->pipe3_in) + 1;
	admin->pipe3_out = (char *) l;
	sprintf(admin->pipe3_out, "%s%c%s", p3main->home, PATH_SEPARATOR, p3FIFO3OUT);

	if (openFIFO() < 0) {
		stat = -1;
		goto out;
	}

out:
	return (stat);
} /* end init_admin */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>init_admin: Failed to allocate main p3 admin data structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 administration interface attempts to allocate its main structure
 * before any other activity.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr>
 * <b>init_admin: Failed to create administration FIFO:
 *   <i>error reason</i></b>
 * \par Description (ERR):
 *   Creating the FIFO for communication with the user interface
 *   failed.
 * \par Response:
 *   Troubleshoot system problem based on the error reason.
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

	if (admin->fifo1_in <= 0 &&
			(admin->fifo1_in = open(admin->pipe1_in, p3FIFO_READ)) < 0) {
		sprintf(p3buf, "openFIFO: Failed to open Level 1 administration\
 FIFO (%s) for reading: %s\n", admin->pipe1_in, strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	} else if (admin->adm_gid1 <= 0 &&
			fstat(admin->fifo1_in, &fifostat) < 0) {
		sprintf(p3buf, "openFIFO: Failed to get Level 1 administration\
 FIFO (%s) group ID: %s\n", admin->pipe1_in, strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	} else
		admin->adm_gid1 = fifostat.st_gid;
	if ((admin->fifo1_out = open(admin->pipe1_out, p3FIFO_WRITE)) < 0) {
		if (errno != ENXIO) {
			sprintf(p3buf, "openFIFO: Failed to open Level 1 administration\
 FIFO (%s) for writing: %s\n", admin->pipe1_out, strerror(errno));
			p3errmsg(p3MSG_ERR, p3buf);
			stat = -1;
			goto out;
		}
	} else if (fstat(admin->fifo1_out, &fifostat) < 0 ) {
		sprintf(p3buf, "openFIFO: Failed to get Level 1 administration\
 FIFO (%s) group ID: %s\n", admin->pipe1_out, strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	} else if (fifostat.st_gid != admin->adm_gid1) {
		p3errmsg(p3MSG_ERR,
			"openFIFO: Level 1 administration FIFO group ID mismatch\n");
		stat = -1;
		goto out;
	}

#ifdef _p3_PRIMARY
	if (admin->fifo2_in <= 0 &&
			(admin->fifo2_in = open(admin->pipe2_in, p3FIFO_READ)) < 0) {
		sprintf(p3buf, "openFIFO: Failed to open Level 2 administration\
 FIFO (%s) for reading: %s\n", admin->pipe2_in, strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	} else if (admin->adm_gid2 <= 0 &&
			fstat(admin->fifo2_in, &fifostat) < 0) {
		sprintf(p3buf, "openFIFO: Failed to get Level 2 administration\
 FIFO (%s) group ID: %s\n", admin->pipe2_in, strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	} else
		admin->adm_gid2 = fifostat.st_gid;
	if ((admin->fifo2_out = open(admin->pipe2_out, p3FIFO_WRITE)) < 0) {
		if (errno != ENXIO) {
			sprintf(p3buf, "openFIFO: Failed to open Level 2 administration\
 FIFO (%s) for writing: %s\n", admin->pipe2_out, strerror(errno));
			p3errmsg(p3MSG_ERR, p3buf);
			stat = -1;
			goto out;
		}
	} else if (fstat(admin->fifo2_out, &fifostat) < 0 ) {
		sprintf(p3buf, "openFIFO: Failed to get Level 2 administration\
 FIFO (%s) group ID: %s\n", admin->pipe2_out, strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	} else if (fifostat.st_gid != admin->adm_gid2) {
		p3errmsg(p3MSG_ERR,
			"openFIFO: Level 2 administration FIFO group ID mismatch\n");
		stat = -1;
		goto out;
	}

	if (admin->fifo3_in <= 0 &&
			(admin->fifo3_in = open(admin->pipe3_in, p3FIFO_READ)) < 0) {
		sprintf(p3buf, "openFIFO: Failed to open Level 3 administration\
 FIFO (%s) for reading: %s\n", admin->pipe3_in, strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	} else if (admin->adm_gid3 <= 0 &&
			fstat(admin->fifo3_in, &fifostat) < 0) {
		sprintf(p3buf, "openFIFO: Failed to get Level 3 administration\
 FIFO (%s) group ID: %s\n", admin->pipe3_in, strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	} else
		admin->adm_gid3 = fifostat.st_gid;
	if ((admin->fifo3_out = open(admin->pipe3_out, p3FIFO_WRITE)) < 0) {
		if (errno != ENXIO) {
			sprintf(p3buf, "openFIFO: Failed to open Level 3 administration\
 FIFO (%s) for writing: %s\n", admin->pipe3_out, strerror(errno));
			p3errmsg(p3MSG_ERR, p3buf);
			stat = -1;
			goto out;
		}
	} else if (fstat(admin->fifo3_out, &fifostat) < 0 ) {
		sprintf(p3buf, "openFIFO: Failed to get Level 3 administration\
 FIFO (%s) group ID: %s\n", admin->pipe3_out, strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	} else if (fifostat.st_gid != admin->adm_gid3) {
		p3errmsg(p3MSG_ERR,
			"openFIFO: Level 3 administration FIFO group ID mismatch\n");
		stat = -1;
		goto out;
	}
#endif

out:
  return(stat);

} /* end openFIFO */

 /** \page P3SYSTEM_MSGS Protected Point to Point System Messages
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
 * admin_handler
 *
 * \par Description:
 * Handle input from the adminstrator interface.
 * 
 * \par Inputs:
 * - msgdata: The definition of the FIFO to be read for
 *   the request.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int admin_handler(p3msgdata *msgdata)
{
	int stat = 0;
	p3comm *comm;
	p3fifomsg fmsg;
	struct stat fifostat;

	// Read input from admin FIFO
	if ((comm = get_request(msgdata)) == NULL) {
		stat = -1;
		goto out;
	}

// *** TODO: Temporary !!!
printf("Comm (%d): Tokens: %d %d %d %d, IDs: %d %d, Data(%d)\n  |%s|\n",
	comm->ID, comm->tokens[0], comm->tokens[1], comm->tokens[2],
	comm->tokens[3], comm->id1, comm->id2, comm->datlen, comm->data);
	if ((stat = get_comm_data(comm, 4)) < 0)
		goto out;
	strcpy(comm->data, "OK");
	comm->datlen = 3;

	// - Update configuration
	// - Get session status
	
	// Create shared memory block to return status

	// Open outbound FIFO if necessary
	if (msgdata->fifo_out <= 0 &&
			(msgdata->fifo_out = open(msgdata->pipe_out, p3FIFO_WRITE)) < 0) {
		if (errno != ENXIO)
			sprintf(p3buf, "admin_handler: Failed to open\
 FIFO (%s) for writing: User Interface not ready\n", msgdata->pipe_out);
		else
			sprintf(p3buf, "admin_handler: Failed to open\
 FIFO (%s) for writing: %s\n", msgdata->pipe_out, strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	} else if (fstat(msgdata->fifo_out, &fifostat) < 0 ) {
		sprintf(p3buf, "admin_handler: Failed to get \
 FIFO (%s) group ID: %s\n", msgdata->pipe_out, strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	} else if (fifostat.st_gid != msgdata->adm_gid) {
		p3errmsg(p3MSG_ERR, "admin_handler: FIFO group ID mismatch\n");
		stat = -1;
		goto out;
	}
	// Send response
	fmsg.flag = 0;
	if (send_response(comm, &fmsg, msgdata) < 0) {
		stat = -1;
	}
	if (comm->data != NULL)
		free(comm->data);
	free(comm);
	
out:
	return (stat);
} /* end admin_handler */

/**
 * \par Function:
 * get_request
 *
 * \par Description:
 * Receive a request from the user interface.
 *
 * \par Inputs:
 * - msgdata: The definition of the FIFO to be read for
 *   the request.
 *
 * \par Outputs:
 * - p3comm *: The request data.  If there is an error,
 *   NULL is returned.
 */

p3comm *get_request(p3msgdata *msgdata)
{
	int sz;
	p3fifomsg fifomsg, *fmsg = &fifomsg;
	p3comm *request = NULL;

	if ((sz = read(msgdata->fifo_in, (void *) fmsg, sizeof(p3fifomsg))) < 0) {
		sprintf(p3buf,
			"get_request: Error receiving message from user interface: %s\n",
			strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
	} else if (!sz || fmsg->datlen < sizeof(p3comm)) {
		p3errmsg(p3MSG_ERR, "get_request: Error receiving message from user\
 interface:  Length less than communication structure\n");
	} else if ((request = (p3comm *) p3calloc(sizeof(p3comm))) == NULL) {
		sprintf(p3buf,
			"get_request: Failed to allocate fifo message buffer: %s\n",
			strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
	} else if ((sz = read(msgdata->fifo_in, (void *) request, sizeof(p3comm))) < 0) {
		sprintf(p3buf,
			"get_request: Error receiving request from user interface:  %s\n",
			strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		free(request);
		request = NULL;
	} else if (sz < sizeof(p3comm)) {
		p3errmsg(p3MSG_ERR, "get_request: Error receiving message from user\
 interface:  Length less than communication structure\n");
		free(request);
		request = NULL;
	} else if (request->datlen > 0) {
		if ((request->data = (char *) p3malloc(request->datlen)) == NULL) {
			sprintf(p3buf,
				"get_request: Failed to allocate fifo data buffer: %s\n",
				strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
		} else if ((sz = read(msgdata->fifo_in,
				(void *) request->data, request->datlen)) < 0) {
			sprintf(p3buf,
				"get_request: Error receiving request data from user interface:  %s\n",
				strerror(errno));
			p3errmsg(p3MSG_ERR, p3buf);
			free(request->data);
			free(request);
			request = NULL;
		} else if (sz < request->datlen) {
			p3errmsg(p3MSG_ERR, "get_request: Error receiving message from user\
 interface:  Length less than communication structure\n");
			free(request->data);
			free(request);
			request = NULL;
		}
	} else
		request->data = NULL;

	return(request);
} /* end get_request */

/** \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>get_request: Failed to allocate fifo <i>type</i> buffer:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 fifo connection handler attempts to allocate a buffer for FIFO
 * messages.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>get_request: Error reading <i>type</i> from user interface:
 *   <i>error reason</i></b>
 * \par Description (ERR):
 *   Reading a response from the FIFO for the system application failed.
 * \par Response:
 *   Troubleshoot system problem based on the error reason.
 *
 */

/**
  * \par Function:
  * send_response
  *
  * \par Description:
  * Send a request to the user interface through the FIFO.
  *
  * \par Inputs:
  * - response: The response data to be sent.
  * - fmsg: The FIFO message information determining how the
  *   response is formatted.
  * - msgdata
  *
  * \par Outputs:
  * - int: Status
  *   - 0 = OK
  *   - <0 = Error
  */

int send_response(p3comm *response, p3fifomsg *fmsg, p3msgdata *msgdata)
{
 	int stat = 0;

 	fmsg->datlen = sizeof(p3comm) + response->datlen;
 	if (write(msgdata->fifo_out, (void *) fmsg, sizeof(p3fifomsg)) < 0) {
 		sprintf(p3buf, "send_response: Error sending response to system\
 application: %s\n", strerror(errno));
 		p3errmsg(p3MSG_ERR, p3buf);
 		stat = -1;
 	} else {
		if (write(msgdata->fifo_out, (void *) response, sizeof(p3comm)) < 0) {
			sprintf(p3buf, "send_response: Error sending response to system\
 application:  %s\n", strerror(errno));
			p3errmsg(p3MSG_ERR, p3buf);
			stat = -1;
		} else if (write(msgdata->fifo_out, response->data, response->datlen) < 0) {
			sprintf(p3buf, "send_response: Error sending response to user\
 interface:  %s\n", strerror(errno));
			p3errmsg(p3MSG_ERR, p3buf);
			stat = -1;
		}
	}

 	return(stat);
} /* end send_response */

/** \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>send_response: Error sending response to user interface:
 *   <i>error reason</i></b>
 * \par Description (ERR):
 *   Writing a response to the FIFO for the system application failed.
 * \par Response:
 *   Troubleshoot system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * get_comm_data
 *
 * \par Description:
 * Determine is the communication data buffer can be reused.
 * If not, release an existing one and get the appropriate
 * new buffer and set the length parameter.
 *
 * \par Inputs:
 * - comm: The P3 communication structure
 * - size: The size of data (including a NULL terminator) required
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = Success
 *   - <0 = Error
 */

int get_comm_data(p3comm *comm, int size)
{
	int stat = 0;

	// Buffer is reusable
	if (comm->datlen >= size && comm->data != NULL)
		goto out;

	if (comm->data != NULL) {
		free(comm->data);
		comm->data = NULL;
	}
	comm->datlen = size;
	if (comm->data == NULL && (comm->data = (char *)
			p3malloc(comm->datlen)) == NULL) {
		sprintf(p3buf, "get_comm_data: Failed to allocate\
 communication data buffer: %s\n", strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		comm->datlen = 0;
		stat = -1;
	}

out:
	return(stat);
} /* end get_comm_data */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>get_comm_data: Failed to allocate communication buffer:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 administration library attempts to allocate a buffer to hold the
 * command data.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

