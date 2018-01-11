/**
 * \file p3primaryplus.c
 * <h3>Protected Point to Point main primaryplus file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The Velocite Systems Protected Point to Point (P3) Primary system
 * is the master end point in a virtual point to point connection over
 * the Internet.  It listens for connections from secondary P3 systems,
 * and when a request is accepted, establishes an IPSec encrypted session.
 * <p>
 * When an encrypted Data session is established, an encrypted Control
 * session which is tunneled through the main session is established.  This
 * is immediately used to set new encryption keys for both the Data and
 * Control sessions.
 * <p>
 * All runtime administration is performed from the primaryplus P3 system.
 * The user interface is interactive from the command line.
 */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <p><hr><hr>
 * \section P3_PRIMARYPLUS Primary Plus Secondary P3 Application Messages
 */

#define _p3_PRIMARY_C	0
#define _p3_SECONDARY_C	0
#include "p3system.h"
#include "p3primary.h"
#include "p3pri_key_server.h"
#include "p3secondary.h"
#include "p3sec_key_handler.h"
#include "p3primaryplus.h"
#include "p3share.h"

/** The main primaryplus structure */
p3pri_main *primain = NULL;
/** The main secondary structure */
p3sec_main *secmain = NULL;

/**
 * ioctl_cmd defines the format of the ioctl data passed between the user and kernel space
 * where the command meanings are:
 * - noop: Unused
 * - primarycfg: Configuration data for a P3 primary system
 * - secondarycfg: Configuration data for a P3 secondary system
 * - prihostcfg: Configuration data for a remote P3 primary system
 * - sechostcfg: Configuration data for a remote P3 secondary system
 * - datakey1: The initial data key for a new P3 session
 * - controlkey1: The initial control key for a new P3 session
 */
enum ioctl_cmd iocmd;

/** Working buffer */
char tbuf[p3BUFSIZE], *p3buf = tbuf;

/**
 * \par Function:
 * main
 *
 * \par Description:
 * Initialize the P3 Primary application, then listen for requests from
 * secondary P3 systems.
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
	int stat = 0;
	char msg_time[64];
	time_t now = time(NULL);
	const struct tm mtm, *msgtm = &mtm;

	// Create main p3 primary data structure
	if ((primain = (p3pri_main *) p3calloc(sizeof(p3pri_main))) == NULL) {
		MSG_TIME(msg_time, &now, msgtm)
		fprintf(stderr, "%s (CRITICAL)  p3pri_main: Failed to allocate main p3\
 primary data structure: %s\n", msg_time, strerror(errno));
		stat = -1;
		goto out;
	}
	// Parse command line arguments
	if ((stat = priplus_parse_cmdline(argc, argv)) != 0) {
		goto out;
	}
	if (primain->home == NULL &&
		(primain->home = (char *) p3malloc(strlen(p3PRI_HOME) + 1)) == NULL) {
		sprintf(p3buf, "p3priplus_main: Failed to allocate p3 primaryplus home\
 directory name: %s\n", strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	strcpy(primain->home, p3PRI_HOME);
	if (primain->config == NULL &&
		(primain->config = (char *) p3malloc(strlen(p3PRI_CONFIG) + 1)) == NULL) {
		sprintf(p3buf, "p3priplus_main: Failed to allocate p3 primaryplus\
 configuration filename: %s\n", strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}

	// Initialize interface with kernel
	if ((stat = init_kernel_comm()) < 0)
		goto out;

	// Parse configuration file
	strcpy(primain->config, p3PRI_CONFIG);
	if ((stat = priplus_parse_config()) < 0) {
		goto out;
	}

	// Initialize user interface
	if ((stat = init_pri_ui()) < 0) {
		goto out;
	}

	// Initialize secondary user interface
	if ((stat = init_sec_ui()) < 0) {
		goto out;
	}

	// Initialize network
	if ((stat = init_pri_net()) < 0) {
		goto out;
	}

	// Initialize secondary network
	if ((stat = init_sec_net()) < 0) {
		goto out;
	}

	// TODO: Listen for requests from secondary systems
	// TODO: Attempt to connect to primary system
	// Attempt to shut down cleanly

out:
	if (stat > 0)
		stat = 0;
	return (stat);
} /* end P3 primaryplus main */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>p3priplus_main: Failed to allocate main p3 primaryplus data structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 primaryplus attempts to allocate its main structure before any other
 * activity.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>p3priplus_main: Failed to allocate main p3 primaryplus home directory name:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 primaryplus attempts to allocate the home directory name during
 * initialization.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>p3priplus_main: Failed to allocate main p3 primaryplus home directory name:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 primaryplus attempts to allocate the configuration filename during
 * initialization.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 */

/**
 * \par Function:
 * priplus_parse_cmdline
 *
 * \par Description:
 * Parse command line arguments supplied to the P3 Primary application.
 * 
 * \par Inputs:
 * - pc_argc: Number of arguments
 * - pc_argv: Argument vector
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 *   - >0 = Informational
 */

int priplus_parse_cmdline(int pc_argc, char *pc_argv[])
{
	int c, stat = 0;
	
	while (!stat &&
		   (c = getopt(pc_argc, pc_argv, ":f:ih:v")) != -1)
	{
		switch(c)
		{
			case 'f':
				if (strlen(optarg) > MAXPATHLEN) {
					p3errmsg (p3MSG_ERR,
						"priplus_parse_cmdline: Configuration filename too large");
					stat = -1;
					goto out;
				}
				if ((primain->config = (char *) p3malloc(strlen(optarg) + 1)) == NULL) {
					sprintf(p3buf, "priplus_parse_cmdline: Failed to allocate p3\
 primaryplus configuration filename: %s\n", strerror(errno));
					p3errmsg (p3MSG_CRIT, p3buf);
					stat = -1;
					goto out;
				}
				break;

			case 'i':
				p3utils->flag |= p3UTL_NINFO;
				break;

			case 'h':
				if (strlen(optarg) > MAXPATHLEN) {
					p3errmsg (p3MSG_ERR,
						"priplus_parse_cmdline: Home directory name too large");
					stat = -1;
					goto out;
				}
				if ((primain->config = (char *) p3malloc(strlen(optarg) + 1)) == NULL) {
					sprintf(p3buf, "priplus_parse_cmdline: Failed to allocate p3\
 primaryplus home directory name: %s\n", strerror(errno));
					p3errmsg (p3MSG_CRIT, p3buf);
					stat = -1;
					goto out;
				}
				break;

			case 'v':
				printf("%s, %s\n", p3APPLICATION, p3VERSION);
				stat = 1;
				break;

			case ':':
				printf("Missing argument\n");
			default:
				printf("%s [-iv] [-h home_dir] [-f config_file]\n", pc_argv[0]);
				printf("  i: Ignore Informational messages\n");
				printf("  v: Product Version\n");
				printf("  h home_dir: The path to the P3 primaryplus home\
 directory\n");
				printf("  f config_file: The name of the P3 primaryplus\
 configuration file, which is\n    concatenated with the home directory\n");
				stat = 1;
				break;
		}
	}

out:
	return(stat);
} /* end priplus_parse_cmdline */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point Messages
 * <hr><b>priplus_parse_cmdline: Home directory name too large</b>
 * \par Description (ERR):
 * The P3 home directory name exceeds the maximum allowed.
 * \par Response:
 * Correct the command line argument.
 *
 * <hr><b>priplus_parse_cmdline: Failed to allocate main p3 primaryplus home
 * directory name:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 primaryplus attempts to allocate the home directory name during
 * initialization.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>priplus_parse_cmdline: Configuration filename too large</b>
 * \par Description (ERR):
 * The P3 configuration filename exceeds the maximum allowed.
 * \par Response:
 * Correct the command line argument.
 *
 * <hr><b>priplus_parse_cmdline: Failed to allocate main p3 primaryplus configuration
 * filename: <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 primaryplus attempts to allocate the configuration filename during
 * initialization.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 */

/**
 * \par Function:
 * priplus_parse_config
 *
 * \par Description:
 * Parse the P3 Primary configuration file.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int priplus_parse_config()
{
	int pc_cfg = 0;
	
//out:
	return(pc_cfg);
} /* end priplus_parse_config */
