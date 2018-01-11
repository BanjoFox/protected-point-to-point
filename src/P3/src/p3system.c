/**
 * \file p3system.c
 * <h3>Protected Point to Point system library file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The Velocite Systems Protected Point to Point (P3) system is the
 * user space library for a virtual point to point connection over
 * the Internet.  A primary system listens for connections from
 * secondary systems, and when a request is accepted, establishes
 * an encrypted session.
 * <p>
 * When an encrypted Data session is established, an encrypted Control
 * session which is tunneled through the main session is established.  This
 * is immediately used to set new encryption keys for both the Data and
 * Control sessions.
 * <p>
 * All runtime administration is performed from the primary P3 system.
 * The user interface is interactive from the command line.
 */

/** \mainpage Velocite Systems P3 Solution Documentation
 * \section P3_OVERVIEW Overview:
 * The Velocite Systems Protected Point to Point Solution provides an
 * encrypted tunnel for multiple sessions to send data with it all
 * appearing to be a single session.  The keys used to encrypt/decrypt
 * the data are updated frequently using an encrypted Control Channel
 * within the tunnel.  The documentation for this application includes:
 *
 * \section P3_FUNC Functions:
 * The P3 Solution source code files are described in the Files section.
 * There are two tabs in this section:
 * - File List: Each function is described under the source code file where
 *   it is found.  The constants, macros, and data definitions used by 
 *   functions are described in the corresponding header file.
 * - Globals: All global variables and contant definitions used in the
 *   P3 Solution application are described in the Globals section.
 *
 * \section P3_STRU Structures:
 * - All data structures used in the P3 Solution application are described
 *   in the Data Structures section.
 *
 * \section P3_MSG Messages:
 * - The P3 Solution messages are explained in the Related Pages section.
 */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <p><hr><hr>
 * \section P3_SYSTEM P3 System Messages
 */

#define _p3_SYSTEM_C	0
#include "p3system.h"
#include "p3primary.h"
#include "p3pri_key_server.h"
#include "p3admin.h"

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

#ifndef _p3_SECONDARY
/** The main primary structure */
p3pri_main *p3main = NULL;
p3rng *rng = NULL;
#else
/** The main secondary structure */
p3sec_main *p3main = NULL;
#endif
// !!! NOTE: Temporarily allocated to fixed size array of 32 !!!
// !!! See allocation notes in parse_config !!!
p3host **remhosts = NULL;

/** Working buffer */
char workbuf[p3BUFSIZE], *p3buf = workbuf;

/**
 * \par Function:
 * init_system
 *
 * \par Description:
 * Initialize the P3 System application.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */
int init_system(int argc, char *argv[])
{
	int i, stat = 0;
	unsigned long l;

#ifndef _p3_SECONDARY
	// Create main p3 primary data structure
p3errmsg (p3MSG_DEBUG, "Init p3main\n");
	i = sizeof(p3pri_main) + sizeof(p3Network) + p3ADDR_SZ;
	if ((p3main = (p3pri_main *) p3calloc(i)) == NULL) {
		sprintf(p3buf, "init_system: Failed to allocate\
 main p3 data structure: %s\n", strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	l = (unsigned long) p3main + sizeof(p3pri_main);
	p3main->net = (p3Network *) l;
	l += sizeof(p3Network);
	p3main->net->addr = (char *) l;
	// Initialize Random Number Generator data
p3errmsg (p3MSG_DEBUG, "Init RNG\n");
	if ((stat = init_rng()) < 0) {
		goto out;
	}
#else
	// Create main p3 secondary data structure
	i = sizeof(p3sec_main) + sizeof(p3Network) + p3ADDR_SZ;
	if ((p3main = (p3sec_main *) p3calloc(i)) == NULL) {
		sprintf(p3buf, "init_system: Failed to allocate\
 main p3 data structure: %s\n", strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	l = (unsigned long) p3main + sizeof(p3sec_main);
	p3main->net = (p3Network *) l;
	l += sizeof(p3Network);
	p3main->net->addr = (char *) l;
#endif

	// Parse command line arguments
p3errmsg (p3MSG_DEBUG, "Parse command line\n");
	if ((stat = parse_cmdline(argc, argv)) != 0) {
		goto out;
	}
	if (p3main->home == NULL) {
		if ((p3main->home = (char *)
				p3malloc(strlen(p3ADM_HOME) + 1)) == NULL) {
			sprintf(p3buf, "init_system: Failed to allocate\
 home directory name: %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
		strcpy(p3main->home, p3ADM_HOME);
	}
	if (p3main->cfgdir == NULL) {
		if ((p3main->cfgdir = (char *)
				p3malloc(strlen(p3ADM_CFG) + 1)) == NULL) {
			sprintf(p3buf, "init_system: Failed to allocate\
 configuration directory name: %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
		strcpy(p3main->cfgdir, p3ADM_CFG);
	}
	if (p3main->msgdir == NULL) {
		if ((p3main->msgdir = (char *)
				p3malloc(strlen(p3ADM_MSG) + 1)) == NULL) {
			sprintf(p3buf, "init_system: Failed to allocate\
 message directory name: %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
		strcpy(p3main->msgdir, p3ADM_MSG);
	}
	if (init_utils(p3main->msgdir) < 0) {
		stat = -1;
		goto out;
	}

	// Initialize interface with kernel
	if ((stat = init_kernel_comm()) < 0)
		goto out;


#ifndef _p3_SECONDARY
	if (p3main->config == NULL) {
		if ((p3main->config = (char *)
				p3malloc(strlen(p3PRI_CONFIG) + 1)) == NULL) {
			sprintf(p3buf, "init_system: Failed to allocate\
 configuration filename: %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
		strcpy(p3main->config, p3PRI_CONFIG);
	}
#else
	if (p3main->config == NULL) {
		if ((p3main->config = (char *)
				p3malloc(strlen(p3SEC_CONFIG) + 1)) == NULL) {
			sprintf(p3buf, "init_system: Failed to allocate\
 configuration filename: %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
		strcpy(p3main->config, p3SEC_CONFIG);
	}
#endif

	// Parse configuration file
	if ((stat = parse_config()) < 0) {
		goto out;
	}

	// Initialize Raw socket for kernel module
// !!! TEMPORARY !!!
// !!! TEMPORARY !!!
#ifndef _p3_SECONDARY
#define RHOSTS	32
#else
#define RHOSTS 2
#endif
// !!! TEMPORARY !!!
// !!! TEMPORARY !!!
	for (i=0; i < RHOSTS; i++) {
		if (remhosts[i] != NULL && (stat = init_raw_socket(remhosts[i])) < 0) {
			goto out;
		}
	}
// !!! TEMPORARY !!!
// !!! TEMPORARY !!!
	for (i=0; i < 8; i++) {
		if (p3main->subnets[i] != NULL &&
				(stat = init_raw_socket(p3main->subnets[i])) < 0) {
			goto out;
		}
	}
// !!! TEMPORARY !!!
// !!! TEMPORARY !!!

#ifndef _p3_SECONDARY
	// Initialize listener
	if (p3listen(p3START, p3main->net) < 0) {
		p3errmsg (p3MSG_CRIT,
			"init_system: Failed to start primary listener\n");
		stat = -1;
		goto out;
	}
#else
	// Connect to the primary to initialize session
	// TODO: Use encryption to init session
	if (p3main->net == NULL) {
		p3errmsg (p3MSG_ERR, "init_system: No primary host defined\n");
		stat = -1;
		goto out;
	}
	while (p3session(p3START, p3main->net) != 0) {
		sleep(5);
		p3session(p3STOP, p3main->net);
	}
	p3session(p3STOP, p3main->net);
p3errmsg (p3MSG_DEBUG, "Session initialized\n");
#endif

	// Initialize user interface
	if ((stat = init_admin())  < 0) {
		goto out;
	}

out:
	return (stat);
} /* end init_system */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>init_system: Failed to allocate main p3 data structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 system attempts to allocate its main structure before any other
 * activity.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>init_system: Failed to allocate <i>type</i> directory name:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 primary attempts to allocate the P3 directory name during
 * initialization.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>init_system: Failed to allocate main p3 primary home directory name:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 primary attempts to allocate the configuration filename during
 * initialization.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 */

/**
 * \par Function:
 * pri_parse_cmdline
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

int parse_cmdline(int pc_argc, char *pc_argv[])
{
	int c, stat = 0;

	while (!stat &&
		   (c = getopt(pc_argc, pc_argv, ":c:f:ih:m:v")) != -1)
	{
		switch(c)
		{
		case 'c':
			if (strlen(optarg) > MAXPATHLEN) {
				p3errmsg (p3MSG_ERR,
					"pri_parse_cmdline: Configuration directory name too large\n");
				stat = -1;
				goto out;
			}
			if ((p3main->cfgdir = (char *) p3malloc(strlen(optarg) + 1)) == NULL) {
				sprintf(p3buf, "pri_parse_cmdline: Failed to allocate p3 primary\
 configuration directory name: %s\n", strerror(errno));
				p3errmsg (p3MSG_CRIT, p3buf);
				stat = -1;
				goto out;
			}
			strcpy(p3main->cfgdir, optarg);
			break;

			case 'f':
				if (strlen(optarg) > MAXPATHLEN) {
					p3errmsg (p3MSG_ERR,
						"pri_parse_cmdline: Configuration filename too large\n");
					stat = -1;
					goto out;
				}
				if ((p3main->config = (char *) p3malloc(strlen(optarg) + 1)) == NULL) {
					sprintf(p3buf, "pri_parse_cmdline: Failed to allocate p3 primary\
 configuration filename: %s\n", strerror(errno));
					p3errmsg (p3MSG_CRIT, p3buf);
					stat = -1;
					goto out;
				}
				strcpy(p3main->config, optarg);
				break;

			case 'i':
				p3main->flag |= p3MN_NINFO;
				break;

			case 'h':
				if (strlen(optarg) > MAXPATHLEN) {
					p3errmsg (p3MSG_ERR,
						"pri_parse_cmdline: Home directory name too large\n");
					stat = -1;
					goto out;
				}
				if ((p3main->home = (char *) p3malloc(strlen(optarg) + 1)) == NULL) {
					sprintf(p3buf, "pri_parse_cmdline: Failed to allocate p3 primary\
 home directory name: %s\n", strerror(errno));
					p3errmsg (p3MSG_CRIT, p3buf);
					stat = -1;
					goto out;
				}
				strcpy(p3main->home, optarg);
				break;

			case 'm':
				if (strlen(optarg) > MAXPATHLEN) {
					p3errmsg (p3MSG_ERR,
						"pri_parse_cmdline: Message directory name too large\n");
					stat = -1;
					goto out;
				}
				if ((p3main->msgdir = (char *) p3malloc(strlen(optarg) + 1)) == NULL) {
					sprintf(p3buf, "pri_parse_cmdline: Failed to allocate p3 primary\
 message directory name: %s\n", strerror(errno));
					p3errmsg (p3MSG_CRIT, p3buf);
					stat = -1;
					goto out;
				}
				strcpy(p3main->msgdir, optarg);
				break;

			case 'v':
				printf("%s, %s\n", p3APPLICATION, p3VERSION);
				stat = 1;
				break;

			case '?':
			case ':':
				printf("Invalid argument\n");
			default:
				printf("%s [-iv] [-h home_dir] [-c config_dir]\n", pc_argv[0]);
				printf("    [-m message_dir] [-f config_file]\n");
				printf("  i: Ignore Informational messages\n");
				printf("  v: Product Version\n");
				printf("  h home_dir: The path to the P3 primary home directory\n");
				printf("  c config_dir: The path to the P3 primary configuration directory\n");
				printf("  m messsage_dir: The path to the P3 primary messages directory\n");
				printf("  f config_file: The name of the P3 primary configuration\
 file, which is\n    concatenated with the home directory\n");
				stat = 1;
				break;
		}
	}

out:
	return(stat);
} /* end parse_cmdline */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>pri_parse_cmdline: Home directory name too large</b>
 * \par Description (ERR):
 * The P3 home directory name exceeds the maximum allowed.
 * \par Response:
 * Correct the command line argument.
 *
 * <hr><b>pri_parse_cmdline: Failed to allocate main p3 primary home directory name:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 primary attempts to allocate the home directory name during
 * initialization.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>pri_parse_cmdline: Configuration filename too large</b>
 * \par Description (ERR):
 * The P3 configuration filename exceeds the maximum allowed.
 * \par Response:
 * Correct the command line argument.
 *
 * <hr><b>pri_parse_cmdline: Failed to allocate main p3 primary configuration
 * filename: <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 primary attempts to allocate the configuration filename during
 * initialization.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 */

#ifndef _p3_SECONDARY

/**
 * \par Function:
 * parse_config
 *
 * \par Description:
 * Parse the P3 Primary configuration file.  If there are errors with
 * individual definitions, parsing continues but an error status is
 * returned.  This allows as many errors as possible to be detected
 * in a single parsing of the configuration file.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int parse_config()
{
	int i, j, num, stat = 0;
	int line = 0, localdef = 0, remhost = 0;
	unsigned long l;
	char *pos, *datapos, *slashpos;
	p3primarycfg pricfg;
	p3sechostcfg shcfg;
	p3subnetcfg *sncfg = NULL;
	p3Network *snet = NULL;
char msgbuf[1024];

	// Configuration file does not exist
	sprintf(p3buf, "%s%c%s", p3main->cfgdir, PATH_SEPARATOR, p3main->config);
	if ((p3main->cfg_data = fopen(p3buf, "r")) == NULL) {
		if (init_config() < 0) {
			stat = -1;
			goto out;
		}
	}
	// Initialize defaults for kernel module
	memset(&pricfg, 0, sizeof(p3primarycfg));
	pricfg.port = p3PRI_PORT;
	p3main->net->port = p3PRI_PORT;
	pricfg.array_size = p3PCFG_ARSZ;
	pricfg.rekey_wait = p3PCFG_RKWT;
	pricfg.datidx_wait = p3PCFG_DIWT;
	pricfg.ctlidx_wait = p3PCFG_CIWT;
	pricfg.hb_wait = p3PCFG_HBWT;
	pricfg.hb_fail = p3PCFG_HBFL;
	memset(&shcfg, 0, sizeof(p3sechostcfg));
	shcfg.rk_wait = pricfg.rekey_wait;
	shcfg.ditime = pricfg.datidx_wait;
	shcfg.citime = pricfg.ctlidx_wait;
	shcfg.hb_wait = pricfg.hb_wait;
	shcfg.hb_fail = pricfg.hb_fail;

// !!! TEMPORARY !!!
// !!! TEMPORARY !!!
	// Initialize remote host array
	// TODO: Add number of hosts in local host definition and use to
	// allocate exact size of array.  Put into Red Black Tree and handle
	// new definitions separately.
	if ((remhosts = (p3host **) p3calloc(32 * sizeof(p3host *))) == NULL) {
		sprintf(p3buf, "parse_config: Failed to allocate p3 remote host\
 array: %s\n", strerror(errno));
		p3errmsg(p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	p3utils->remhost = remhosts;
	// TODO: Handle local subnets
	if ((p3main->subnets = (p3host **) p3calloc(8 * sizeof(p3host *))) == NULL) {
		sprintf(p3buf, "parse_config: Failed to allocate p3 local subnet\
 array: %s\n", strerror(errno));
		p3errmsg(p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	p3utils->flag |= 32;
// !!! TEMPORARY !!!
// !!! TEMPORARY !!!

	// Parse configuration file
	while (!feof(p3main->cfg_data) && !ferror(p3main->cfg_data)) {
		line++;
		if ((pos = fgets(p3buf, p3BUFSIZE, p3main->cfg_data)) == NULL) {
			continue;
		}
		// Empty line or comment
		if (p3buf[0] == '\n' || p3buf[0] == '#') {
			continue;
		}
		if ((pos = strrchr(p3buf,'\n')) != NULL) {
			pos[0] = '\0';
		}
		// Remove spaces and non-ASCII characters
		for (i=0,j=0; i <= strlen(p3buf); i++) {
			if (!isspace(p3buf[i]) && isprint(p3buf[i]))
				p3buf[j++] = p3buf[i];
		}
		p3buf[j] = '\0';
		// Get standalone keywords or verify that a value exists.
		if (strncmp(p3buf, LOCALSTART, sizeof(LOCALSTART)) == 0) {
			localdef = 1;
			continue;
		} else if (strncmp(p3buf, LOCALEND, sizeof(LOCALEND)) == 0) {
		   	localdef = 0;
		   	// Send local defs to kernel module
		   	p3buf[0] = primarycfg;
		   	memcpy(&p3buf[1], &pricfg, sizeof(p3primarycfg));
		   	if (send_ioctl((unsigned char *) p3buf, sizeof(p3primarycfg) + 1)
					< 0) {
				stat = -1;
				goto out;
		   	}
		   	continue;
		} else if ((datapos = strchr(p3buf, '=')) == NULL) {
			sprintf(p3buf, "parse_config: %s:%d No equal sign\n", p3main->config, line);
			p3errmsg(p3MSG_ERR, p3buf);
			stat = -1;
			continue;
		}
		datapos[0] = '\0';
		datapos++;
		if (strlen(datapos) == 0) {
			sprintf(p3buf, "parse_config: %s:%d No value\n", p3main->config, line);
			p3errmsg(p3MSG_ERR, p3buf);
			stat = -1;
			continue;
		}
		// Test for secondary definition (primary definitions have no "path")
		slashpos = strchr(p3buf, '/');
		if (slashpos != NULL && localdef) {
			sprintf(p3buf, "parse_config: %s:%d Remote Secondary definition before\
 local Primary complete\n", p3main->config, line);
			p3errmsg(p3MSG_ERR, p3buf);
			stat = -1;
			continue;
		// Parse local primary definitions
		} else if (slashpos == NULL) {
			// Get primary P3 host network definition
			if (!strcmp(p3buf,"ip")) {
p3errmsg(p3MSG_DEBUG, " ==> Get primary local IP\n");
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid ip value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
				i = atoi(datapos);
				if (i == 4)
					pricfg.flag |= p3HST_IPV4;
				else if (i == 6)
					pricfg.flag |= p3HST_IPV6;
				else {
					sprintf(p3buf, "parse_config: %s:%d Invalid ip value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
			} else if (!strcmp(p3buf,"address")) {
p3errmsg(p3MSG_DEBUG, " ==> Get address\n");
				strcpy(p3main->net->addr, datapos);
				if (pricfg.flag & p3HST_IPV4) {
					if (inet_pton(AF_INET,(const char *)datapos, (void *)&pricfg.addr.v4) <= 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid IPv4 address value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
					memcpy(&p3utils->lochost.v4, (void *)&pricfg.addr.v4,
						sizeof(struct in_addr));
				} else if (pricfg.flag & p3HST_IPV6) {
					if (inet_pton(AF_INET6,(const char *)datapos, (void *)&pricfg.addr.v6) <= 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid IPv6 address value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
					memcpy(&p3utils->lochost.v6, (void *)&pricfg.addr.v6, sizeof(struct in6_addr));
				} else {
					sprintf(p3buf, "parse_config: %s:%d IP version not set for network definition\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
			} else if (!strcmp(p3buf,"port")) {
p3errmsg(p3MSG_DEBUG, " ==> Get port\n");
				if ((num = atoi(datapos)) < 0 || 65536 < num) {
					sprintf(p3buf, "parse_config: %s:%d Invalid port value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				} else {
					pricfg.port = num;
					p3main->net->port = num;
				}
			// Get clustering state
			} else if (!strcmp(p3buf,"cluster_state")) {
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid cluster_state value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				} else {
					if ((num = atoi(datapos)) > 1 || num < 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid cluster_state value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					} else if (num && (p3main->cluster =
							(p3cluster *) p3calloc(sizeof(p3cluster))) == NULL) {
						sprintf(p3buf, "parse_config: Failed to allocate p3\
 clustering data structure: %s\n", strerror(errno));
						p3errmsg(p3MSG_CRIT, p3buf);
						stat = -1;
						goto out;
					}
				}
			} else if (!strcmp(p3buf,"load_balance")) {
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid load_balance value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				} else {
					if ((num = atoi(datapos)) > 2 || num < 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid load_balance value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
					if (num) {
						if (p3main->cluster == NULL) {
						sprintf(p3buf, "parse_config: %s:%d Clustering must be enable for load_balance\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
						} else
							p3main->cluster->flag |= num;
					}
				}
			// Get random number generator definition
			} else if (!strcmp(p3buf,"key_generation")) {
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid key_generation value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				} else {
					if ((num = atoi(datapos)) > 3) {
						sprintf(p3buf, "parse_config: %s:%d Invalid key_generation value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					} else
						p3main->flag |= num << p3MN_RNSHF;
				}
			// Get key definitions
			} else if (!strcmp(p3buf,"rekey_wait")) {
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid rekey_wait value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				} else {
					pricfg.rekey_wait = atoi(datapos);
				}
			} else if (!strcmp(p3buf,"array_size")) {
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid key_array_size value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				} else {
					pricfg.array_size = atoi(datapos);
				}
			} else if (!strcmp(p3buf,"data_array_time")) {
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid data_array_time value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				} else {
					pricfg.datidx_wait = atoi(datapos);
				}
			} else if (!strcmp(p3buf,"control_array_time")) {
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid control_array_time value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				} else {
					pricfg.ctlidx_wait = atoi(datapos);
				}
			// Get heartbeat definitions
			} else if (!strcmp(p3buf,"heartbeat_time")) {
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid heartbeat_time value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				} else {
					pricfg.hb_wait = atoi(datapos);
				}
			} else if (!strcmp(p3buf,"heartbeat_fail")) {
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid heartbeat_fail value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				} else {
					pricfg.hb_fail = atoi(datapos);
				}
			} else if (!strncmp(p3buf,"subnet", 6)) {
// TODO: Local subnet definition needs to be improved
//   Currently uses a host address on the subnet to send Raw packet
p3errmsg(p3MSG_DEBUG, " ==> Get primary subnet\n");
				// Find subnet configuration in list or create a new one
				if (isallnum(&p3buf[6]) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid subnet ID\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
					continue;
				}
				if ((num = atoi(&p3buf[6])) <= 0 || num > 8) {
					sprintf(p3buf, "parse_config: %s:%d Invalid subnet\
 id value: %d\n", p3main->config, line, num);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
					goto out;
				// Add new subnet
				} else if (p3main->subnets[num] == NULL && (p3main->subnets[num]
						= (p3host *) p3calloc(sizeof(p3host))) == NULL) {
					sprintf(p3buf, "parse_config: Failed to allocate p3\
 local subnet: %s\n", strerror(errno));
					p3errmsg(p3MSG_CRIT, p3buf);
					stat = -1;
					goto out;
				}
// TODO: Get local subnet IP version
				if (pricfg.flag & p3HST_IPV4) {
					if (inet_pton(AF_INET,(const char *)datapos,
							(void *)&p3main->subnets[num]->addr.v4) <= 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid IPv4 subnet\
 value\n", p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
					p3main->subnets[num]->flag |= p3HST_IPV4;
					memcpy(&pricfg.subnet.v4, &p3main->subnets[num]->addr.v4,
						sizeof(struct in_addr));
				} else if (pricfg.flag & p3HST_IPV6) {
					if (inet_pton(AF_INET6,(const char *)datapos,
							(void *)&p3main->subnets[num]->addr.v6) <= 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid IPv6 subnet\
 value\n", p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
					p3main->subnets[num]->flag |= p3HST_IPV6;
					// Copy to pricfg
				} else {
					sprintf(p3buf, "parse_config: %s:%d IP version not set for\
 subnet definition\n", p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}

			// Get overrides
		   	// Get clustering definitions
		   	// Get failover definitions

			} else {
				sprintf(p3buf, "parse_config: %s:%d Invalid parameter\n",
						p3main->config, line);
				p3errmsg (p3MSG_ERR, p3buf);
				stat = -1;
			}
		// Parse remote secondary definitions
		} else {
			slashpos[0] = '\0';
			slashpos++;
sprintf(msgbuf, "Parse sec host: %s :: %s :: %s\n", p3buf, slashpos, datapos);
p3errmsg(p3MSG_DEBUG, msgbuf);
			// Secondary ID is first field in definition
			if ((num = atoi(p3buf)) <= 0) {
				sprintf(p3buf, "parse_config: %s:%d Invalid secondary host id value: %d\n",
						p3main->config, line, num);
				p3errmsg (p3MSG_ERR, p3buf);
				stat = -1;
			} else if (num != (shcfg.flag & p3HST_ID)) {
// !!! TEMPORARY !!!
// !!! TEMPORARY !!!
sprintf(msgbuf, "Get remote host structure %d\n", num);
p3errmsg(p3MSG_DEBUG, msgbuf);
				if (num > 32) {
					p3errmsg(p3MSG_ERR, "Invalid remote host number (> 32)\n");
					stat = -1;
					goto out;
				}
// !!! TEMPORARY !!!
// !!! TEMPORARY !!!
				if (remhosts[num] == NULL && (remhosts[num] = (p3host *)
						p3calloc(sizeof(p3host))) == NULL) {
					sprintf(p3buf, "parse_config: Failed to allocate remote host\
 definition: %s\n", strerror(errno));
					p3errmsg(p3MSG_CRIT, p3buf);
					stat = -1;
					goto out;
				}
				i = sizeof(p3Network) + p3ADDR_SZ;
				if ((snet = (p3Network *) p3calloc(i)) == NULL) {
					sprintf(p3buf, "parse_config: Failed to allocate remote host\
 definition: %s\n", strerror(errno));
					p3errmsg(p3MSG_CRIT, p3buf);
					stat = -1;
					goto out;
				}
				l = (unsigned long) snet + sizeof(p3Network);
				snet->addr = (char *) l;
				snet->next = p3main->seclist;
				p3main->seclist = snet;
				if ((shcfg.flag & p3HST_ID) >= 0) {
					// Send secondary definition to kernel module
					if ((shcfg.flag & p3HST_ID) != 0) {
sprintf(msgbuf, "Sec host cfg: %d, %d, %d, %d, %d, %d, %x\n",
		shcfg.subnetsz, shcfg.rk_wait, shcfg.hb_wait, shcfg.hb_fail,
		shcfg.ditime, shcfg.citime, shcfg.flag);
p3errmsg(p3MSG_DEBUG, msgbuf);
						if (update_sechost(&shcfg) < 0) {
							stat = -1;
							goto out;
						}
						memset(&shcfg, 0, sizeof(p3sechostcfg));
						shcfg.rk_wait = pricfg.rekey_wait;
						shcfg.ditime = pricfg.datidx_wait;
						shcfg.citime = pricfg.ctlidx_wait;
						shcfg.hb_wait = pricfg.hb_wait;
						shcfg.hb_fail = pricfg.hb_fail;
						sncfg = NULL;
					}
				}
				shcfg.flag = num;
			}
			// Get secondary P3 host definition
			if (!strcmp(slashpos,"ip")) {
p3errmsg(p3MSG_DEBUG, " ==> Get remote secondary IP\n");
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid ip value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
				i = atoi(datapos);
				if (i == 4) {
					shcfg.flag |= p3HST_IPV4;
					remhosts[num]->flag |= p3HST_IPV4;
				} else if (i == 6) {
					shcfg.flag |= p3HST_IPV6;
					remhosts[num]->flag |= p3HST_IPV6;
				} else {
					sprintf(p3buf, "parse_config: %s:%d Invalid ip value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
			} else if (!strcmp(slashpos,"address")) {
p3errmsg(p3MSG_DEBUG, " ==> Get address\n");
				//TODO: Make sure order is correct
				strcpy(snet->addr, datapos);
				if (shcfg.flag & p3HST_IPV4) {
					if (inet_pton(AF_INET,(const char *)datapos, (void *)&shcfg.addr.v4) <= 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid IPv4 address value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
					memcpy(&remhosts[num]->addr.v4, (void *)&shcfg.addr.v4,
						sizeof(struct in_addr));
				} else if (shcfg.flag & p3HST_IPV6) {
					if (inet_pton(AF_INET6,(const char *)datapos, (void *)&shcfg.addr.v6) <= 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid IPv6 address value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
					memcpy(&remhosts[num]->addr.v6, (void *)&shcfg.addr.v6, sizeof(struct in6_addr));
				} else {
					sprintf(p3buf, "parse_config: %s:%d IP version not set for network definition\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
			// Get subnet definitions
			} else if (!strncmp(slashpos,"subnet", 6)) {
p3errmsg(p3MSG_DEBUG, " ==> Get subnet\n");
				// Find subnet configuration in list or create a new one
				if (isallnum(&slashpos[6]) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid subnet ID\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
					continue;
				}
				if ((num = atoi(&slashpos[6])) <= 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid subnet id value: %d\n",
							p3main->config, line, num);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				} else if (sncfg == NULL || sncfg->id != num) {
					sncfg = shcfg.subnets;
					while (sncfg != NULL) {
						if (sncfg->id == num)
							break;
						sncfg = sncfg->next;
					}
					// Add new subnet
					if (sncfg == NULL) {
						if ((sncfg = getsubnetcfg()) == NULL) {
							stat = -1;
							goto out;
						}
						sncfg->next = shcfg.subnets;
						shcfg.subnets = sncfg;
						shcfg.subnetsz++;
					}
					sncfg->id = num;
				}
				if (shcfg.flag & p3HST_IPV4) {
					if (inet_pton(AF_INET,(const char *)datapos, (void *)&sncfg->net.v4) <= 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid IPv4 subnet value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
				} else if (shcfg.flag & p3HST_IPV6) {
					if (inet_pton(AF_INET6,(const char *)datapos, (void *)&sncfg->net.v6) <= 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid IPv6 subnet value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
				} else {
					sprintf(p3buf, "parse_config: %s:%d IP version not set for subnet definition\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
			} else if (!strncmp(slashpos,"mask", 4)) {
p3errmsg(p3MSG_DEBUG, " ==> Get mask\n");
				// Find subnet configuration in list or create a new one
				if (isallnum(&slashpos[4]) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid subnet ID\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
					continue;
				}
				if ((num = atoi(&slashpos[4])) <= 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid subnet id value: %d\n",
							p3main->config, line, num);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				} else if (sncfg == NULL || sncfg->id != num) {
					sncfg = shcfg.subnets;
					while (sncfg != NULL) {
						if (sncfg->id == num)
							break;
						sncfg = sncfg->next;
					}
					// Add new subnet
					if (sncfg == NULL) {
						if ((sncfg = getsubnetcfg()) == NULL) {
							stat = -1;
							goto out;
						}
						sncfg->next = shcfg.subnets;
						shcfg.subnets = sncfg;
						shcfg.subnetsz++;
					}
					sncfg->id = num;
				}
				if (shcfg.flag & p3HST_IPV4) {
					if (inet_pton(AF_INET,(const char *)datapos, (void *)&sncfg->mask) <= 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid IPv4 address value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
				} else if (shcfg.flag & p3HST_IPV6) {
					// TODO: Handle IPv6 subnet mask equivalent
   					sprintf(p3buf, "parse_config: %s:%d IPv6 mask not supported\n",
   							p3main->config, line);
   					p3errmsg (p3MSG_ERR, p3buf);
				} else {
					sprintf(p3buf, "parse_config: %s:%d IP version not set for subnet mask definition\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
			// Get overrides
			} else if (!strcmp(slashpos,"key_type")) {
p3errmsg(p3MSG_DEBUG, " ==> Get key type\n");
				if (!strcmp(datapos, "AES128")) {
					shcfg.flag |= p3KTYPE_AES128 << p3HST_KTSHF;
				} else if (!strcmp(datapos, "AES256")) {
				   	shcfg.flag |= p3KTYPE_AES256 << p3HST_KTSHF;
				} else {
					sprintf(p3buf, "parse_config: %s:%d Invalid key_type value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
			} else if (!strcmp(slashpos,"rekey_wait")) {
p3errmsg(p3MSG_DEBUG, " ==> Get rekey wait\n");
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid rekey_wait value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
				shcfg.rk_wait = atoi(datapos);
			} else if (!strcmp(slashpos,"key_array")) {
p3errmsg(p3MSG_DEBUG, " ==> Get key array\n");
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid key_array value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
				num = atoi(datapos);
				if (num == 1)
					shcfg.flag |= p3HST_ARRAY;
				else if (num != 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid key_array value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
			} else if (!strcmp(slashpos,"data_array_time")) {
p3errmsg(p3MSG_DEBUG, " ==> Get ditime\n");
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid data_array_time value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
				shcfg.ditime = atoi(datapos);
			} else if (!strcmp(slashpos,"control_array_time")) {
p3errmsg(p3MSG_DEBUG, " ==> Get citime\n");
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid control_array_time value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
				shcfg.citime = atoi(datapos);
			} else if (!strcmp(slashpos,"heartbeat_time")) {
p3errmsg(p3MSG_DEBUG, " ==> Get hb_time\n");
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid heartbeat_time value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
				shcfg.hb_wait = atoi(datapos);
			} else if (!strcmp(slashpos,"heartbeat_fail")) {
p3errmsg(p3MSG_DEBUG, " ==> Get hb_fail\n");
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid heartbeat_fail value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
				shcfg.hb_fail = atoi(datapos);
			}
		}
	}
	// Send last secondary definition to kernel module
	if ((shcfg.flag & p3HST_ID) != 0) {
sprintf(msgbuf, "Sec host cfg last: %d, %d, %d, %d, %d, %d, %x\n",
		shcfg.subnetsz, shcfg.rk_wait, shcfg.hb_wait, shcfg.hb_fail,
		shcfg.ditime, shcfg.citime, shcfg.flag);
p3errmsg(p3MSG_DEBUG, msgbuf);
		if (update_sechost(&shcfg) < 0) {
			stat = -1;
			goto out;
		}
	}
	fclose(p3main->cfg_data);

out:
	return(stat);
} /* end parse_config */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>parse_config: <i>config file</i> <i>line number</i>
 * No equal sign</b>
 * \par Description (ERR):
 * The P3 primary configuration file parser did not find an equal sign
 * for a parameter that requires a value.
 * \par Response:
 * Correct the configuration error.
 *
 * <hr><b>parse_config: <i>config file</i> <i>line number</i>
 * No value</b>
 * \par Description (ERR):
 * The P3 primary configuration file parser did not find a value
 * for a parameter that requires one.
 * \par Response:
 * Correct the configuration error.
 *
 * <hr><b>parse_config: <i>config file</i> <i>line number</i>
 * No equal sign</b>
 * \par Description (ERR):
 * The P3 primary configuration file parser found a P3 secondary definition
 * before the "localend" keyword was found.
 * \par Response:
 * Correct the configuration error.
 *
 * <hr><b>parse_config: <i>config file</i> <i>line number</i>
 * Invalid <i>parameter name</i> value</b>
 * \par Description (ERR):
 * The P3 primary configuration file parser found an invalid value
 * for the given parameter.
 * \par Response:
 * Correct the configuration error.
 *
 * <hr><b>parse_config: <i>config file</i> <i>line number</i>
 * Invalid parameter</b>
 * \par Description (ERR):
 * The P3 primary configuration file parser found an invalid parameter.
 * \par Response:
 * Correct the configuration error.
 *
 * <hr><b>parse_config: Failed to allocate p3 clustering data structure:
 * <i>reason code</i></b>
 * \par Description (CRIT):
 * The P3 primary attempts to allocate the clustering data structure,
 * if configured.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * update_sechost
 *
 * \par Description:
 * Build a remote secondary host definition to be sent to the kernel module
 * in an ioctl buffer.  The list of subnet configurations is freed as the
 * command buffer is built.
 *
 * \par Inputs:
 * - sechost: The secondary host definition, which contains a list of
 *   subnets associated with it.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int update_sechost(p3sechostcfg *sechost)
{
	int bufsz, stat = 0;
	unsigned char *iocbuf, *iocptr;
	p3subnetcfg *sncfg;

	if (sechost == NULL) {
		stat = -1;
		goto out;
	}

	// Build the ioctl buffer
	bufsz = 1 + sizeof(p3sechostcfg) + (sechost->subnetsz * sizeof(p3subnetcfg));
	if ((iocbuf = (unsigned char *) p3malloc(bufsz)) == NULL) {
		sprintf(p3buf, "update_sechost: Failed to allocate kernel module command buffer: %s\n",
				strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	iocbuf[0] = (unsigned char) secondaryhostcfg;
	iocptr = iocbuf + 1;
	memcpy (iocptr, sechost, sizeof(p3sechostcfg));
	iocptr += sizeof(p3sechostcfg);
	while (sechost->subnetsz) {
		memcpy(iocptr, sechost->subnets, sizeof(p3subnetcfg));
		iocptr += sizeof(p3subnetcfg);
		// Release subnet storage
		sncfg = sechost->subnets;
		sechost->subnets = sncfg->next;
		free(sncfg);
		// Decrement counter to verify it is accurate
		sechost->subnetsz--;
		if (sechost->subnets == NULL)
			break;
	}
	if (sechost->subnetsz) {
		p3errmsg (p3MSG_WARN, "update_sechost: Counter greater than subnet list\n");
		stat = -1;
	} else if (sechost->subnets != NULL) {
		p3errmsg (p3MSG_ERR, "update_sechost: Counter less than subnet list\n");
		stat = -1;
		while (sechost->subnets != NULL) {
			sncfg = sechost->subnets;
			sechost->subnets = sncfg->next;
			free(sncfg);
		}
	}

   	if (stat == 0 && send_ioctl(iocbuf, bufsz) < 0)
		stat = -1;

	free(iocbuf);

out:
	return(stat);
} /* end update_sechost */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>update_sechost: Failed to allocate kernel module command buffer:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 primary failed to allocate the buffer for sending a remote
 * secondary configuration to the kernel module.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>update_sechost: Counter greater than subnet list</b>
 * \par Description (WARN):
 * The number of subnets defined was greater than the counter in the
 * remote secondary host definition indicated.  This means there was a
 * logic error in generating the list.
 * \par Response:
 * Report the problem to Velocite Systems support.
 *
 * <hr><b>update_sechost: Counter less than subnet list</b>
 * \par Description (ERR):
 * The number of subnets defined was less than the counter in the
 * remote secondary host definition indicated.  This means that not
 * all subnet definitions could be sent to the kernel module.
 * \par Response:
 * Report the problem to Velocite Systems support.
 *
 */

/**
 * \par Function:
 * system_handler
 *
 * \par Description:
 * Loop continuously until the system application is stopped.  In
 * each loop, check for the following:
 * - User Interface requests through a FIFO
 * - Secondary connection requests
 * - Key management requirements
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int system_handler()
{
	int stat = 0, numfds = 0;
	char iobuf[64];
	fd_set fdset;
	struct timeval tv;
	p3msgdata msgdata;
	p3Network *snet;

	while (1) {
		FD_ZERO(&fdset);

		// Monitor listener for secondaries
		FD_SET(p3main->net->sd, &fdset);
		if (numfds <= p3main->net->sd)
			numfds = p3main->net->sd + 1;

		// Handle secondary session
		// TODO: Use Red Black tree
//		snet = p3main->seclist;
//		while (snet != NULL) {
//			if (snet->flag & p3NET_ST_UP) {
//				FD_SET(snet->sd, &fdset);
//				if (numfds <= snet->sd)
//					numfds = snet->sd + 1;
//			}
//			snet = snet->next;
//		}

		// Monitor FIFOs
		FD_SET(admin->fifo1_in, &fdset);
		if (numfds <= admin->fifo1_in)
			numfds = admin->fifo1_in + 1;
		FD_SET(admin->fifo2_in, &fdset);
		if (numfds <= admin->fifo2_in)
			numfds = admin->fifo2_in + 1;
		FD_SET(admin->fifo3_in, &fdset);
		if (numfds <= admin->fifo3_in)
			numfds = admin->fifo3_in + 1;
		// Wake up at least once every second
		tv.tv_sec = 0;
		tv.tv_usec = p3MILLISEC(1000);
		// Build entropy constantly
//		tv.tv_usec = p3MICROSEC(211);
		if ((stat = select(numfds, &fdset, 0, 0, &tv)) < 0) {
			if (errno == EINTR)
				continue;
			sprintf(p3buf, "system_handler: Error in system\
 application monitor: %s\n", strerror(errno));
			p3errmsg(p3MSG_ERR, p3buf);
			stat = -1;
			goto out;
		}

		// Handle listener request
		if (p3main->net->sd > 0 && FD_ISSET(p3main->net->sd, &fdset)) {
			// Request from secondary resets initial key
			snet = p3main->seclist;
			while (snet != NULL) {
				if (snet->flag & p3NET_ST_UP) {
p3errmsg(p3MSG_DEBUG, "Init secondary\n");
					strcpy(iobuf, "INIT");
					// TODO: Support IPv6
					inet_pton(AF_INET, snet->addr, (void *)&iobuf[4]);
					if (send_ioctl((unsigned char *)iobuf, 8) == 0)
						snet->flag &= ~p3NET_ST_UP;
				}
				snet = snet->next;
			}
		}

		// Handle secondary session activity
//		snet = p3main->seclist;
//		while (snet != NULL) {
//			if (snet->sd > 0 && FD_ISSET(snet->sd, &fdset)) {
//				// Status updates
//			}
//			snet = snet->next;
//		}

		// Handle FIFO request
		if (admin->fifo1_in > 0 && FD_ISSET(admin->fifo1_in, &fdset)) {
			msgdata.fifo_in = admin->fifo1_in;
			msgdata.pipe_in = admin->pipe1_in;
			msgdata.fifo_out = admin->fifo1_out;
			msgdata.pipe_out = admin->pipe1_out;
			msgdata.adm_gid = admin->adm_gid1;
			if (admin_handler(&msgdata) < 0) {
				close(admin->fifo1_in);
				close(admin->fifo1_out);
				admin->fifo1_in = -1;
				admin->fifo1_out = -1;
				openFIFO();
			}
		}

		if (admin->fifo2_in > 0 && FD_ISSET(admin->fifo2_in, &fdset)) {
			msgdata.fifo_in = admin->fifo2_in;
			msgdata.pipe_in = admin->pipe2_in;
			msgdata.fifo_out = admin->fifo2_out;
			msgdata.pipe_out = admin->pipe2_out;
			msgdata.adm_gid = admin->adm_gid2;
			if (admin_handler(&msgdata) < 0) {
				close(admin->fifo2_in);
				close(admin->fifo2_out);
				admin->fifo2_in = -1;
				admin->fifo2_out = -1;
				openFIFO();
			}
		}

		if (admin->fifo3_in > 0 && FD_ISSET(admin->fifo3_in, &fdset)) {
			msgdata.fifo_in = admin->fifo3_in;
			msgdata.pipe_in = admin->pipe3_in;
			msgdata.fifo_out = admin->fifo3_out;
			msgdata.pipe_out = admin->pipe3_out;
			msgdata.adm_gid = admin->adm_gid3;
			if (admin_handler(&msgdata) < 0) {
				close(admin->fifo3_in);
				close(admin->fifo3_out);
				admin->fifo3_in = -1;
				admin->fifo3_out = -1;
				openFIFO();
			}
		}

		// Perform key management
		// TODO: Call buffer handler more efficiently
		buffer_handler();

		// Build entropy
		// TODO: Run get entropy as thread
		get_entropy();
	}

out:
	close(p3utils->msgfile);
	return(stat);
} /* end system_handler */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>system_handler:  Error in system application monitor:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 system application monitor listens for requests from the
 * user interface.  If there is an error, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

#else /* ifndef _p3_SECONDARY */

/**
 * \par Function:
 * parse_config
 *
 * \par Description:
 * Parse the P3 Secondary configuration file.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 *   - >0 = Config does not exist, use defaults
 */

int parse_config()
{
	int i, j, num, stat = 0;
	int line = 0, localdef = 0, remhost = 0;
	unsigned long l;
	char *pos, *datapos, *slashpos;
	p3secondarycfg seccfg;
	p3prihostcfg phcfg;
	p3subnetcfg *sncfg = NULL;
char msgbuf[1024];

	// Configuration file does not exist
	sprintf(p3buf, "%s%c%s", p3main->cfgdir, PATH_SEPARATOR, p3main->config);
	if ((p3main->cfg_data = fopen(p3buf, "r")) == NULL) {
		if (init_config() < 0) {
			stat = -1;
			goto out;
		}
	}
	// Initialize defaults for kernel module
	memset(&seccfg, 0, sizeof(p3secondarycfg));
	memset(&phcfg, 0, sizeof(p3prihostcfg));
	phcfg.port = p3PRI_PORT;
	p3main->net->port = p3PRI_PORT;

// !!! TEMPORARY !!!
// !!! TEMPORARY !!!
	// Initialize remote host array
	// TODO: Add number of hosts in local host definition and use to
	// allocate exact size of array.  Put into Red Black Tree and handle
	// new definitions separately.
	if ((remhosts = (p3host **) p3calloc(2 * sizeof(p3host *))) == NULL) {
		sprintf(p3buf, "parse_config: Failed to allocate p3 remote host\
 array: %s\n", strerror(errno));
		p3errmsg(p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	p3utils->remhost = remhosts;
	// TODO: Handle local subnets
	if ((p3main->subnets = (p3host **) p3calloc(8 * sizeof(p3host *))) == NULL) {
		sprintf(p3buf, "parse_config: Failed to allocate p3 local subnet\
 array: %s\n", strerror(errno));
		p3errmsg(p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	p3utils->flag |= 2;
// !!! TEMPORARY !!!
// !!! TEMPORARY !!!

	// Parse configuration file
	while (!feof(p3main->cfg_data) && !ferror(p3main->cfg_data)) {
		line++;
		if ((pos = fgets(p3buf, p3BUFSIZE, p3main->cfg_data)) == NULL) {
			continue;
		}
		// Empty line or comment
		if (p3buf[0] == '\n' || p3buf[0] == '#') {
			continue;
		}
		if ((pos = strrchr(p3buf,'\n')) != NULL) {
			pos[0] = '\0';
		}
		// Remove spaces and non-ASCII characters
		for (i=0,j=0; i <= strlen(p3buf); i++) {
			if (!isspace(p3buf[i]) && isprint(p3buf[i]))
				p3buf[j++] = p3buf[i];
		}
		p3buf[j] = '\0';
		// Get standalone keywords or verify that a value exists.
		if (strncmp(p3buf, LOCALSTART, sizeof(LOCALSTART)) == 0) {
			localdef = 1;
			continue;
		} else if (strncmp(p3buf, LOCALEND, sizeof(LOCALEND)) == 0) {
		   	localdef = 0;
		   	// Send local defs to kernel module
		   	p3buf[0] = secondarycfg;
		   	memcpy(&p3buf[1], &seccfg, sizeof(p3secondarycfg));
		   	if (send_ioctl((unsigned char *) p3buf, sizeof(p3secondarycfg) + 1) < 0) {
				stat = -1;
				goto out;
		   	}
		   	continue;
		} else if ((datapos = strchr(p3buf, '=')) == NULL) {
			sprintf(p3buf, "parse_config: %s:%d No equal sign\n", p3main->config, line);
			p3errmsg(p3MSG_ERR, p3buf);
			stat = -1;
			continue;
		}
		datapos[0] = '\0';
		datapos++;
		if (strlen(datapos) == 0) {
			sprintf(p3buf, "parse_config: %s:%d No value\n", p3main->config, line);
			p3errmsg(p3MSG_ERR, p3buf);
			stat = -1;
			continue;
		}
		// Test for secondary definition (secondary definitions have no "path")
		slashpos = strchr(p3buf, '/');
		if (slashpos != NULL && localdef) {
			sprintf(p3buf, "parse_config: %s:%d Remote Secondary definition before\
 local secondary complete\n", p3main->config, line);
			p3errmsg(p3MSG_ERR, p3buf);
			stat = -1;
			continue;
		// Parse local secondary definitions
		} else if (slashpos == NULL) {
			// Get secondary P3 host network definition
			if (!strcmp(p3buf,"ip")) {
p3errmsg(p3MSG_DEBUG, " ==> Get secondary local IP\n");
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid ip value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
				num = atoi(datapos);
				if (num == 4)
					seccfg.flag |= p3HST_IPV4;
				else if (num == 6)
					seccfg.flag |= p3HST_IPV6;
				else {
					sprintf(p3buf, "parse_config: %s:%d Invalid ip value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
			} else if (!strcmp(p3buf,"address")) {
p3errmsg(p3MSG_DEBUG, " ==> Get address\n");
				if (seccfg.flag & p3HST_IPV4) {
					strcpy(p3main->net->addr, datapos);
					if (inet_pton(AF_INET,(const char *)datapos, (void *)&seccfg.addr.v4) <= 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid IPv4 address value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
					memcpy(&p3utils->lochost.v4, (void *)&seccfg.addr.v4,
						sizeof(struct in_addr));
				} else if (seccfg.flag & p3HST_IPV6) {
					if (inet_pton(AF_INET6,(const char *)datapos, (void *)&seccfg.addr.v6) <= 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid IPv6 address value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
					memcpy(&p3utils->lochost.v6, (void *)&seccfg.addr.v6, sizeof(struct in6_addr));
				} else {
					sprintf(p3buf, "parse_config: %s:%d IP version not set for network definition\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
			// Get clustering state
			} else if (!strcmp(p3buf,"cluster_state")) {
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid cluster_state value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				} else {
					if ((num = atoi(datapos)) > 1 || num < 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid cluster_state value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					} else if (num && (p3main->cluster =
							(p3cluster *) p3calloc(sizeof(p3cluster))) == NULL) {
						sprintf(p3buf, "parse_config: Failed to allocate p3 clustering\
 data structure: %s\n", strerror(errno));
						p3errmsg(p3MSG_CRIT, p3buf);
						stat = -1;
						goto out;
					}
				}
			} else if (!strcmp(p3buf,"load_balance")) {
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid load_balance value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				} else {
					if ((num = atoi(datapos)) > 2 || num < 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid load_balance value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
					if (num) {
						if (p3main->cluster == NULL) {
						sprintf(p3buf, "parse_config: %s:%d Clustering must be enable for load_balance\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
						} else
							p3main->cluster->flag |= num;
					}
				}
			} else if (!strncmp(p3buf,"subnet", 6)) {
// TODO: Local subnet definition needs to be improved
//   Currently uses a host address on the subnet to send Raw packet
p3errmsg(p3MSG_DEBUG, " ==> Get secondary subnet\n");
				// Find subnet configuration in list or create a new one
				if (isallnum(&p3buf[6]) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid subnet ID\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
					continue;
				}
				if ((num = atoi(&p3buf[6])) <= 0 || num > 8) {
					sprintf(p3buf, "parse_config: %s:%d Invalid subnet\
 id value: %d\n", p3main->config, line, num);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
					goto out;
				// Add new subnet
				} else if (p3main->subnets[num] == NULL && (p3main->subnets[num]
						= (p3host *) p3calloc(sizeof(p3host))) == NULL) {
					sprintf(p3buf, "parse_config: Failed to allocate p3\
 local subnet: %s\n", strerror(errno));
					p3errmsg(p3MSG_CRIT, p3buf);
					stat = -1;
					goto out;
				}
// TODO: Get local subnet IP version
				if (seccfg.flag & p3HST_IPV4) {
					if (inet_pton(AF_INET,(const char *)datapos,
							(void *)&p3main->subnets[num]->addr.v4) <= 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid IPv4 subnet\
 value\n", p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
					p3main->subnets[num]->flag |= p3HST_IPV4;
					memcpy(&seccfg.subnet.v4, &p3main->subnets[num]->addr.v4,
						sizeof(struct in_addr));
				} else if (seccfg.flag & p3HST_IPV6) {
					if (inet_pton(AF_INET6,(const char *)datapos,
							(void *)&p3main->subnets[num]->addr.v6) <= 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid IPv6 subnet\
 value\n", p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
					p3main->subnets[num]->flag |= p3HST_IPV6;
				} else {
					sprintf(p3buf, "parse_config: %s:%d IP version not set for\
 subnet definition\n", p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
			} else {
				sprintf(p3buf, "parse_config: %s:%d Invalid parameter\n",
						p3main->config, line);
				p3errmsg (p3MSG_ERR, p3buf);
				stat = -1;
			}

		// Parse remote primary definitions
		} else {
			slashpos[0] = '\0';
			slashpos++;
			// Primary ID is first field in definition
			if ((num = atoi(p3buf)) <= 0) {
				sprintf(p3buf, "parse_config: %s:%d Invalid primary host\
 id value: %d\n", p3main->config, line, num);
				p3errmsg (p3MSG_ERR, p3buf);
				stat = -1;
			} else if (num != (phcfg.flag & p3HST_ID)) {
// !!! TEMPORARY !!!
// !!! TEMPORARY !!!
				if (num > 2) {
					p3errmsg(p3MSG_ERR, "Invalid remote host number (> 32)\n");
					stat = -1;
					goto out;
				}
// !!! TEMPORARY !!!
// !!! TEMPORARY !!!
				if (remhosts[num] == NULL && (remhosts[num] = (p3host *)
						p3calloc(sizeof(p3host))) == NULL) {
					sprintf(p3buf, "parse_config: Failed to allocate remote host\
 definition: %s\n", strerror(errno));
					p3errmsg(p3MSG_CRIT, p3buf);
					stat = -1;
					goto out;
				}
				i = sizeof(p3Network) + p3ADDR_SZ;
				if ((phcfg.flag & p3HST_ID) >= 0) {
					// Send primary definition to kernel module
					if ((phcfg.flag & p3HST_ID) != 0) {
sprintf(msgbuf, "Pri host cfg: %d, %x\n", phcfg.subnetsz, phcfg.flag);
p3errmsg(p3MSG_DEBUG, msgbuf);
						if (update_prihost(&phcfg) < 0) {
							stat = -1;
							goto out;
						}
						memset(&phcfg, 0, sizeof(p3prihostcfg));
						phcfg.port = p3PRI_PORT;
						p3main->net->port = p3PRI_PORT;
						sncfg = NULL;
					}
				}
				phcfg.flag = num;
			}
			// Get primary P3 host definition
			if (!strcmp(slashpos,"ip")) {
				if (isallnum(datapos) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid ip value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
				i = atoi(datapos);
				if (i == 4) {
					phcfg.flag |= p3HST_IPV4;
					remhosts[num]->flag |= p3HST_IPV4;
				} else if (i == 6) {
					phcfg.flag |= p3HST_IPV6;
					remhosts[num]->flag |= p3HST_IPV6;
				} else {
					sprintf(p3buf, "parse_config: %s:%d Invalid ip value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
			} else if (!strcmp(slashpos,"address")) {
				strcpy(p3main->net->addr, datapos);
				if (phcfg.flag & p3HST_IPV4) {
					if (inet_pton(AF_INET,(const char *)datapos, (void *)&phcfg.addr.v4) <= 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid IPv4 address value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
					memcpy((void *)&remhosts[num]->addr.v4, (void *)&phcfg.addr.v4, sizeof(struct in_addr));
//					if (!remhost) {
//						remhost = 1;
//						memcpy(&p3utils->remhost.v4, (void *)&phcfg.addr.v4, sizeof(struct in_addr));
//					}
				} else if (phcfg.flag & p3HST_IPV6) {
					if (inet_pton(AF_INET6,(const char *)datapos, (void *)&phcfg.addr.v6) <= 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid IPv6 address value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
					memcpy((void *)&remhosts[num]->addr.v6, (void *)&phcfg.addr.v6, sizeof(struct in6_addr));
//					if (!remhost) {
//						remhost = 1;
//						memcpy(&p3utils->remhost.v6, (void *)&phcfg.addr.v6, sizeof(struct in6_addr));
//					}
				} else {
					sprintf(p3buf, "parse_config: %s:%d IP version not set for network definition\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
			} else if (!strcmp(slashpos,"port")) {
p3errmsg(p3MSG_DEBUG, " ==> Get port\n");
				if ((num = atoi(datapos)) < 0 || 65536 < num) {
					sprintf(p3buf, "parse_config: %s:%d Invalid port value\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				} else {
					phcfg.port = num;
					p3main->net->port = num;
				}
			// Get subnet definitions
			} else if (!strncmp(slashpos,"subnet", 6)) {
				// Find subnet configuration in list or create a new one
				if (isallnum(&slashpos[6]) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid subnet ID\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
					continue;
				}
				if ((num = atoi(&slashpos[6])) <= 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid subnet id value: %d\n",
							p3main->config, line, num);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				} else if (sncfg == NULL || sncfg->id != num) {
					sncfg = phcfg.subnets;
					while (sncfg != NULL) {
						if (sncfg->id == num)
							break;
						sncfg = sncfg->next;
					}
					// Add new subnet
					if (sncfg == NULL) {
						if ((sncfg = getsubnetcfg()) == NULL) {
							stat = -1;
							goto out;
						}
						sncfg->next = phcfg.subnets;
						phcfg.subnets = sncfg;
						phcfg.subnetsz++;
					}
					sncfg->id = num;
				}
				if (phcfg.flag & p3HST_IPV4) {
					if (inet_pton(AF_INET,(const char *)datapos, (void *)&sncfg->net.v4) <= 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid IPv4 subnet value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
				} else if (phcfg.flag & p3HST_IPV6) {
					if (inet_pton(AF_INET6,(const char *)datapos, (void *)&sncfg->net.v6) <= 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid IPv6 subnet value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
				} else {
					sprintf(p3buf, "parse_config: %s:%d IP version not set for subnet definition\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
			} else if (!strncmp(slashpos,"mask", 4)) {
				// Find subnet configuration in list or create a new one
				if (isallnum(&slashpos[4]) < 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid subnet ID\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
					continue;
				}
				if ((num = atoi(&slashpos[4])) <= 0) {
					sprintf(p3buf, "parse_config: %s:%d Invalid subnet id value: %d\n",
							p3main->config, line, num);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				} else if (sncfg == NULL || sncfg->id != num) {
					sncfg = phcfg.subnets;
					while (sncfg != NULL) {
						if (sncfg->id == num)
							break;
						sncfg = sncfg->next;
					}
					// Add new subnet
					if (sncfg == NULL) {
						if ((sncfg = getsubnetcfg()) == NULL) {
							stat = -1;
							goto out;
						}
						sncfg->next = phcfg.subnets;
						phcfg.subnets = sncfg;
						phcfg.subnetsz++;
					}
					sncfg->id = num;
				}
				if (phcfg.flag & p3HST_IPV4) {
					if (inet_pton(AF_INET,(const char *)datapos, (void *)&sncfg->mask) <= 0) {
						sprintf(p3buf, "parse_config: %s:%d Invalid IPv4 address value\n",
								p3main->config, line);
						p3errmsg (p3MSG_ERR, p3buf);
						stat = -1;
					}
				} else if (phcfg.flag & p3HST_IPV6) {
					// TODO: Handle IPv6 subnet mask equivalent
					sprintf(p3buf, "parse_config: %s:%d IPv6 mask not supported\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
				} else {
					sprintf(p3buf, "parse_config: %s:%d IP version not set for subnet mask definition\n",
							p3main->config, line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
				}
			}
		}
	}
	// Send last primary definition to kernel module
	if ((phcfg.flag & p3HST_ID) != 0) {
sprintf(msgbuf, "Pri host cfg last: %d, %x\n", phcfg.subnetsz, phcfg.flag);
p3errmsg(p3MSG_DEBUG, msgbuf);
		if (update_prihost(&phcfg) < 0) {
			stat = -1;
			goto out;
		}
	}
	fclose(p3main->cfg_data);

out:
	return(stat);
} /* end parse_config */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>parse_config: <i>config file</i> <i>line number</i>
 * No equal sign</b>
 * \par Description (ERR):
 * The P3 secondary configuration file parser did not find an equal sign
 * for a parameter that requires a value.
 * \par Response:
 * Correct the configuration error.
 *
 * <hr><b>parse_config: <i>config file</i> <i>line number</i>
 * No value</b>
 * \par Description (ERR):
 * The P3 secondary configuration file parser did not find a value
 * for a parameter that requires one.
 * \par Response:
 * Correct the configuration error.
 *
 * <hr><b>parse_config: <i>config file</i> <i>line number</i>
 * No equal sign</b>
 * \par Description (ERR):
 * The P3 secondary configuration file parser found a P3 secondary definition
 * before the "localend" keyword was found.
 * \par Response:
 * Correct the configuration error.
 *
 * <hr><b>parse_config: <i>config file</i> <i>line number</i>
 * Invalid <i>parameter name</i> value</b>
 * \par Description (ERR):
 * The P3 secondary configuration file parser found an invalid value
 * for the given parameter.
 * \par Response:
 * Correct the configuration error.
 *
 * <hr><b>parse_config: <i>config file</i> <i>line number</i>
 * Invalid parameter</b>
 * \par Description (ERR):
 * The P3 secondary configuration file parser found an invalid parameter.
 * \par Response:
 * Correct the configuration error.
 *
 * <hr><b>parse_config: Failed to allocate p3 clustering data structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 secondary attempts to allocate the clustering data structure,
 * if configured.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 **/

/**
 * \par Function:
 * update_prihost
 *
 * \par Description:
 * Build a remote primary host definition to be sent to the kernel module
 * in an ioctl buffer.  The list of subnet configurations is freed as the
 * command buffer is built.
 *
 * \par Inputs:
 * - prihost: The primary host definition, which contains a list of
 *   subnets associated with it.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int update_prihost(p3prihostcfg *prihost)
{
	int bufsz, stat = 0;
	unsigned char *iocbuf, *iocptr;
	p3subnetcfg *sncfg;

	if (prihost == NULL) {
		stat = -1;
		goto out;
	}

	// Build the ioctl buffer
	bufsz = 1 + sizeof(p3prihostcfg) + (prihost->subnetsz * sizeof(p3subnetcfg));
	if ((iocbuf = (unsigned char *) p3malloc(bufsz)) == NULL) {
		sprintf(p3buf, "update_prihost: Failed to allocate kernel module command buffer: %s\n",
				strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	iocbuf[0] = (unsigned char) primaryhostcfg;
	iocptr = iocbuf + 1;
	memcpy (iocptr, prihost, sizeof(p3prihostcfg));
	iocptr += sizeof(p3prihostcfg);
	while (prihost->subnetsz) {
		memcpy(iocptr, prihost->subnets, sizeof(p3subnetcfg));
		iocptr += sizeof(p3subnetcfg);
		// Release subnet storage
		sncfg = prihost->subnets;
		prihost->subnets = sncfg->next;
		free(sncfg);
		// Decrement counter to verify it is accurate
		prihost->subnetsz--;
		if (prihost->subnets == NULL)
			break;
	}
	if (prihost->subnetsz) {
		p3errmsg (p3MSG_WARN, "update_prihost: Counter greater than subnet list\n");
		stat = -1;
	} else if (prihost->subnets != NULL) {
		p3errmsg (p3MSG_ERR, "update_prihost: Counter less than subnet list\n");
		stat = -1;
		while (prihost->subnets != NULL) {
			sncfg = prihost->subnets;
			prihost->subnets = sncfg->next;
			free(sncfg);
		}
	}

   	if (stat == 0 && send_ioctl(iocbuf, bufsz) < 0)
		stat = -1;

	free(iocbuf);

out:
	return(stat);
} /* end update_prihost */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>update_prihost: Failed to allocate kernel module command buffer:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 primary failed to allocate the buffer for sending a remote
 * primary configuration to the kernel module.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>update_prihost: Counter greater than subnet list</b>
 * \par Description (WARN):
 * The number of subnets defined was greater than the counter in the
 * remote primary host definition indicated.  This means there was a
 * logic error in generating the list.
 * \par Response:
 * Report the problem to Velocite Systems support.
 *
 * <hr><b>update_prihost: Counter less than subnet list</b>
 * \par Description (ERR):
 * The number of subnets defined was less than the counter in the
 * remote primary host definition indicated.  This means that not
 * all subnet definitions could be sent to the kernel module.
 * \par Response:
 * Report the problem to Velocite Systems support.
 *
 */

/**
 * \par Function:
 * system_handler
 *
 * \par Description:
 * Loop continuously until the system application is stopped.  In
 * each loop, check for User Interface requests through a FIFO.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int system_handler()
{
	int stat = 0, numfds = 0;
	fd_set fdset;
	struct timeval tv;
	p3msgdata msgdata;

	while (1) {
		FD_ZERO(&fdset);
		FD_SET(admin->fifo1_in, &fdset);
		if (numfds <= admin->fifo1_in)
			numfds = admin->fifo1_in + 1;
		// Wake up at least once every 10 seconds(?)
		tv.tv_sec = 0;
		tv.tv_usec = p3MILLISEC(10000);
		if ((stat = select(numfds, &fdset, 0, 0, &tv)) < 0) {
			if (errno == EINTR)
				continue;
			sprintf(p3buf, "system_handler: Error in system\
 application monitor: %s\n", strerror(errno));
			p3errmsg(p3MSG_ERR, p3buf);
			stat = -1;
			goto out;
		}

		// Handle FIFO request
		if (admin->fifo1_in > 0 && FD_ISSET(admin->fifo1_in, &fdset)) {
			msgdata.fifo_in = admin->fifo1_in;
			msgdata.pipe_in = admin->pipe1_in;
			msgdata.fifo_out = admin->fifo1_out;
			msgdata.pipe_out = admin->pipe1_out;
			msgdata.adm_gid = admin->adm_gid1;
			admin_handler(&msgdata);
		}
	}

out:
	close(p3utils->msgfile);
	return(stat);
} /* end system_handler */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>system_handler:  Error in system application monitor:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 system application monitor listens for requests from the
 * user interface.  If there is an error, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

#endif /* _p3_SECONDARY */

/**
 * \par Function:
 * init_config
 *
 * \par Description:
 * Initialize the configuration file for first time use.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_config()
{
	int idx, stat = 0;
	FILE *p3dir;

	// Validate that the home directory exists, create it if not
	if ((p3dir = fopen(p3main->home, "r")) != NULL) {
		fclose(p3dir);
	} else {
		idx = 0;
		while (idx < strlen(p3main->home)) {
			// Skip preceeding path separators
			while (p3main->home[idx] == PATH_SEPARATOR) {
				p3buf[idx] = p3main->home[idx];
				idx++;
			}
			// Get next directory in path
			while (p3main->home[idx] != PATH_SEPARATOR) {
				p3buf[idx] = p3main->home[idx];
				idx++;
			}
			// Attempt to open directory, create if not found
			if ((p3dir = fopen(p3buf, "r")) != NULL) {
				fclose(p3dir);
			} else if (mkdir(p3buf, p3SHARE_MODE) < 0) {
				sprintf(p3buf, "init_config: Failed to create P3 directory path %s: %s",
						p3main->home, strerror(errno));
				p3errmsg(p3MSG_CRIT, p3buf);
				stat = -1;
				goto out;
			}
		}
	}

	// Create configuration file
	if ((p3dir = fopen(p3main->cfgdir, "r")) == NULL) {
		sprintf(p3buf, "init_config: Failed to open configuration\
 directory %s: %s\n", p3main->cfgdir, strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	} else {
		fclose(p3dir);
		sprintf(p3buf, "%s%c%s", p3main->cfgdir, PATH_SEPARATOR, p3main->config);
		if ((p3main->cfg_data = fopen(p3buf, "w")) == NULL) {
			sprintf(p3buf, "init_config: Failed to create primary\
 configuration file %s%c%s: %s\n", p3main->cfgdir,
				 PATH_SEPARATOR, p3main->config, strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
	}

	// TODO: Set default values

out:
	return (stat);
} /* end init_config */

/**
 * \page P3SYSTEM_MSGS Protected Point to Point System Messages
 * <hr><b>init_config: Failed to create P3 home directory path:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 system application attempts to create the home directory path
 * the first time the application is used.  If this fails, there is a
 * system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>init_config: Failed to open configuration directory:
 * <i>directory name</i>: <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 system application attempts to access the configuration
 * directory the first time the application is used.  If this fails,
 * there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>init_config: Failed to create configuration file:
 * <i>file path</i>: <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 system application attempts to create the configuration file
 * the first time the application is used.  If this fails, there is a
 * system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */
