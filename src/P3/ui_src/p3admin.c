/**
 * \file p3admin.c
 * <h3>Protected Point to Point administrator interface file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The adminstrator interface is used to manage the key server as well as
 * secondary P3 systems.
 */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <p><hr><hr>
 * \section P3_ADMIN P3 Administration Messages
 */

#define _p3_ADMIN_C 1
#include "p3admin.h"
#include "p3fifo.h"
#include <jni.h>

/** Main administration structure */
p3admin *admin;
char appname[] = p3APPLICATION;

/** List of communication structures */
p3commlist *commlist = NULL;
p3commlist *cltail = NULL;

/** Communication structure with reusable data field */
p3comm pcomm;

/**
 * \par Function:
 * init_admin
 *
 * \par Description:
 * Initialize and run the P3 administration library functions.
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

int init_admin(int argc, char *argv[])
{
	int i, stat = 0;
	unsigned long l;

	// Allocate and initialize administration structures
	i = sizeof(p3admin) + sizeof(p3config) + sizeof(p3command)
		+ sizeof(p3local) + sizeof(p3user);
	if ((admin = (p3admin *) p3calloc(i)) == NULL) {
		sprintf(p3buf, "init_admin: Failed to allocate main administration\
 structure: %s\n", strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	l = (unsigned long) admin + sizeof(p3admin);
	admin->config = (p3config *) l;
	l += sizeof(p3config);
	admin->command = (p3command *) l;
	l += sizeof(p3command);
	admin->admuser = (p3user *) l;
	l += sizeof(p3user);
	admin->config->local = (p3local *) l;
	admin->command->config = admin->config;
	admin->ID = 1;

	// TODO: *** Temporary ***
	admin->admuser->level = 1;

	if (init_utils() < 0) {
		stat = -1;
		goto out;
	}

	if ((stat = adm_parse_cmdline(argc, argv)) != 0) {
		goto out;
	}

	if (admin->home == NULL && (admin->home = (char *)
			p3malloc(sizeof(p3ADM_HOME))) == NULL) {
		sprintf(p3buf, "init_admin: Failed to allocate\
 home directory name: %s\n", strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
		strcpy(admin->home, p3ADM_HOME);
	}

	if (admin->config->path == NULL && (admin->config->path = (char *)
			p3malloc(sizeof(p3CFG_PATH))) == NULL) {
		sprintf(p3buf, "init_admin: Failed to allocate\
 configuration directory name: %s\n", strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
		strcpy(admin->config->path, p3CFG_PATH);
	}

#ifdef _p3_PRIMARY
	admin->config->local->port = p3ADM_PORT;
	admin->config->local->rekey = p3ADM_REKEY;
	admin->config->local->arraysz = p3ADM_ARRAYSZ;
	admin->config->local->datarray = p3ADM_DATARRAY;
	admin->config->local->ctlarray = p3ADM_CTLARRAY;
	admin->config->local->heartbeat = p3ADM_HRTBT_TIME;
	admin->config->local->heartfail = p3ADM_HRTBT_FAIL;
	admin->config->local->flag = p3LHCFG_DEFAULT;
#endif
	if (setup_admin() < 0) {
		stat = -1;
		goto out;
	}

out:
	return(stat);
} /* end init_admin */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>p3admin: Failed to allocate main administration structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 administration interface attempts to allocate its main structure
 * before any other activity.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * adm_parse_cmdline
 *
 * \par Description:
 * Parse command line arguments supplied to the P3 Primary application.
 * Supported arguments are:
 * <ul>
 * <li><b>-i</b>: Ignore Informational messages</li>
 * <li><b>-v</b>: Product Version</li>
 * <li><b>-c config_dir</b>: The P3 configuration directory path</li>
 * <li><b>-p config_file</b>: The P3 primary configuration file</li>
 * <li><b>-s config_file</b>: The P3 secondary configuration file</li>
 * </ul>
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

int adm_parse_cmdline(int pc_argc, char *pc_argv[])
{
	int i, c, stat = 0;
	
	while (!stat &&
		   (c = getopt(pc_argc, pc_argv, ":c:h:ip:s:v")) != -1) {
		switch(c) {

			case 'i':
				util->flag |= p3UTL_NINFO;
				break;

			case 'c':
				if (optarg == NULL) {
					p3errmsg (p3MSG_ERR, "Missing argument value\n");
					stat = -1;
					goto out;
				}
				if (strlen(optarg) > (MAXPATHLEN - 1)) {
					p3errmsg (p3MSG_ERR,
							"Configuration directory name too long\n");
					stat = -1;
					goto out;
				}
				i = strlen(optarg) - 1;
				if (optarg[i] == PATH_SEPARATOR) {
					i = strlen(optarg) + 1;
					stat = 1;
				} else
					i = strlen(optarg) + 2;
				if ((admin->config->path =
						(char *) p3malloc(i)) == NULL) {
					sprintf(p3buf, "Failed to allocate configuration directory\
 name: %s\n", strerror(errno));
					p3errmsg (p3MSG_CRIT, p3buf);
					stat = -1;
					goto out;
				}
				if (stat) {
					strcpy(admin->config->path, optarg);
					stat = 0;
				} else
					sprintf(admin->config->path, "%s%c", optarg, PATH_SEPARATOR);
				break;

			case 'h':
				if (optarg == NULL) {
					p3errmsg (p3MSG_ERR, "Missing argument value\n");
					stat = -1;
					goto out;
				}
				if (strlen(optarg) > (MAXPATHLEN - 1)) {
					p3errmsg (p3MSG_ERR,
							"Home directory name too long\n");
					stat = -1;
					goto out;
				}
				i = strlen(optarg) - 1;
				if (optarg[i] == PATH_SEPARATOR) {
					i = strlen(optarg) + 1;
					stat = 1;
				} else
					i = strlen(optarg) + 2;
				if ((admin->home =
						(char *) p3malloc(i)) == NULL) {
					sprintf(p3buf, "Failed to allocate home directory\
 name: %s\n", strerror(errno));
					p3errmsg (p3MSG_CRIT, p3buf);
					stat = -1;
					goto out;
				}
				if (stat) {
					strcpy(admin->home, optarg);
					stat = 0;
				} else
					sprintf(admin->home, "%s%c", optarg, PATH_SEPARATOR);
				break;

			case 'p':
				if (optarg == NULL) {
					p3errmsg (p3MSG_ERR, "Missing argument value\n");
					stat = -1;
					goto out;
				}
				if (strlen(optarg) > MAXPATHLEN) {
					p3errmsg (p3MSG_ERR,
							"Primary configuration file name too long\n");
					stat = -1;
					goto out;
				}
				if ((admin->config->pricfg =
						(char *) p3malloc(strlen(optarg) + 1)) == NULL) {
					sprintf(p3buf, "Failed to allocate primary configuration\
 filename: %s\n", strerror(errno));
					p3errmsg (p3MSG_CRIT, p3buf);
					stat = -1;
					goto out;
				}
				strcpy(admin->config->pricfg, optarg);
				break;

			case 's':
				if (optarg == NULL) {
					p3errmsg (p3MSG_ERR, "Missing argument value\n");
					stat = -1;
					goto out;
				}
				if (strlen(optarg) > MAXPATHLEN) {
					p3errmsg (p3MSG_ERR,
							"Secondary configuration file name too long\n");
					stat = -1;
					goto out;
				}
				if ((admin->config->seccfg =
						(char *) p3malloc(strlen(optarg) + 1)) == NULL) {
					sprintf(p3buf, "Failed to allocate secondary configuration\
 filename: %s\n", strerror(errno));
					p3errmsg (p3MSG_CRIT, p3buf);
					stat = -1;
					goto out;
				}
				strcpy(admin->config->seccfg, optarg);
				break;

			case 'v':
				printf("Velocite Systems P3 Administration Library, %s\n", p3VERSION);
				stat = 1;
				break;

			case ':':
			case '?':
				printf("Invalid argument list\n");
			default:
				printf("\n%s [-iv] [-h home_dir] [-c config_dir]\n", appname);
				printf("    [-p pri_config_file] [-s sec_config_file]\n");
				printf("  i: Ignore Informational messages\n");
				printf("  v: Product Version\n");
				printf("  h home_dir: The P3 home directory path where the\n");
				printf("     system application FIFOs are located\n");
				printf("  c config_dir: The P3 configuration directory path\n");
				printf("  p config_file: The P3 primary configuration file\n");
				printf("  s config_file: The P3 secondary configuration file\n");
				printf("\n");
				stat = 1;
				break;
		}
	}

out:
	return(stat);
} /* end adm_parse_cmdline */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>Failed to allocate <i>location</i> name:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 administration interface attempts to allocate path and file names
 * for configuration data.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b><i>Location</i> name too long</b>
 * \par Description (ERR):
 * The path or file name is longer that allowed by the operating system.
 * \par Response:
 * Correct the path or file name.
 *
 */

/**
 * \par Function:
 * setup_admin
 *
 * \par Description:
 * Initialize the adminstrator data structures:
 * - Parser arrays:  Commands and configuration data are defined by
 *   URI constructs (ie. /level1/level2/level3/etc) and the possible
 *   values at each level are kept in sorted arrays.  Once the construct
 *   has been parsed, only the index into each level's array needs to
 *   be passed between components in the <i>tokens</i> array of the
 *   p3admin structure.
 * - Local host structure:  The information about the local host
 *   is parsed from the configuration file.
 * - Groups:  Information about secondary groups is parsed from the
 *   configuration file and maintained in a p3group structure array
 *   for each level.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int setup_admin()
{
	int i, stat = 0;

	// Validate admin user access

	// Build Level 1 parser array
	i = sizeof(p3L1CMD_TOK) + sizeof(p3L1FREE_TOK) + sizeof(p3L1GETNEXT_TOK)
		+ sizeof(p3L1GROUP_TOK) + sizeof(p3L1LOCAL_TOK)
		+ sizeof(p3L1REMOTE_TOK) + sizeof(p3L1SESSION_TOK);
	if ((level1[0] = (char *) p3malloc(i)) == NULL) {
		sprintf(p3buf, "setup_admin: Failed to allocate Level 1\
 parser array: %s\n", strerror(errno));
		p3errmsg(p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	// NOTE: Insert new tokens in alphabetical order
	set_token(level1, p3L1CMD, p3L1CMD_TOK);
	set_token(level1, p3L1FREE, p3L1FREE_TOK);
	set_token(level1, p3L1GETNEXT, p3L1GETNEXT_TOK);
	set_token(level1, p3L1GROUP, p3L1GROUP_TOK);
	set_token(level1, p3L1LOCAL, p3L1LOCAL_TOK);
	set_token(level1, p3L1REMOTE, p3L1REMOTE_TOK);
	strcpy(level1[p3L1SESSION], p3L1SESSION_TOK);

	// Build Level 2 parser array
	i = sizeof(p3L2ADDRESS_TOK) + sizeof(p3L2ANON_TOK)
		+ sizeof(p3L2ANONCMD_TOK) + sizeof(p3L2ANONIPVER_TOK)
		+ sizeof(p3L2ANONNET_TOK) + sizeof(p3L2ANONPORT_TOK) + sizeof(p3L2ANONSET_TOK)
		+ sizeof(p3L2ARRAY_TOK) + sizeof(p3L2CATEGORY_TOK)
		+ sizeof(p3L2ENCRYPTION_TOK) + sizeof(p3L2FIFO_TOK)
		+ sizeof(p3L2GENKEY_TOK) + sizeof(p3L2GET_TOK) + sizeof(p3L2GROUP_TOK)
		+ sizeof(p3L2HEARTBEAT_TOK) + sizeof(p3L2HOSTNAME_TOK)
		+ sizeof(p3L2IPVER_TOK) + sizeof(p3L2KEY_TOK) + sizeof(p3L2NAME_TOK)
		+ sizeof(p3L2PORT_TOK) + sizeof(p3L2REKEY_TOK) + sizeof(p3L2SHUTDOWN_TOK)
		+ sizeof(p3L2STATUS_TOK) + sizeof(p3L2SUBNET_TOK) + sizeof(p3L2SYSCFG_TOK)
		+ sizeof(p3L2UPDATE_TOK) + sizeof(p3L2USER_TOK);
	if ((level2[0] = (char *) p3malloc(i)) == NULL) {
		sprintf(p3buf, "setup_admin: Failed to allocate Level 2\
 parser array: %s\n", strerror(errno));
		p3errmsg(p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	// NOTE: Insert new tokens in alphabetical order
	set_token(level2, p3L2ADDRESS, p3L2ADDRESS_TOK);
	set_token(level2, p3L2ANON, p3L2ANON_TOK);
	set_token(level2, p3L2ANONCMD, p3L2ANONCMD_TOK);
	set_token(level2, p3L2ANONIPVER, p3L2ANONIPVER_TOK);
	set_token(level2, p3L2ANONNET, p3L2ANONNET_TOK);
	set_token(level2, p3L2ANONPORT, p3L2ANONPORT_TOK);
	set_token(level2, p3L2ANONSET, p3L2ANONSET_TOK);
	set_token(level2, p3L2ARRAY, p3L2ARRAY_TOK);
	set_token(level2, p3L2CATEGORY, p3L2CATEGORY_TOK);
	set_token(level2, p3L2ENCRYPTION, p3L2ENCRYPTION_TOK);
	set_token(level2, p3L2FIFO, p3L2FIFO_TOK);
	set_token(level2, p3L2GENKEY, p3L2GENKEY_TOK);
	set_token(level2, p3L2GET, p3L2GET_TOK);
	set_token(level2, p3L2GROUP, p3L2GROUP_TOK);
	set_token(level2, p3L2HEARTBEAT, p3L2HEARTBEAT_TOK);
	set_token(level2, p3L2HOSTNAME, p3L2HOSTNAME_TOK);
	set_token(level2, p3L2IPVER, p3L2IPVER_TOK);
	set_token(level2, p3L2KEY, p3L2KEY_TOK);
	set_token(level2, p3L2NAME, p3L2NAME_TOK);
	set_token(level2, p3L2PORT, p3L2PORT_TOK);
	set_token(level2, p3L2REKEY, p3L2REKEY_TOK);
	set_token(level2, p3L2SHUTDOWN, p3L2SHUTDOWN_TOK);
	set_token(level2, p3L2STATUS, p3L2STATUS_TOK);
	set_token(level2, p3L2SUBNET, p3L2SUBNET_TOK);
	set_token(level2, p3L2SYSCFG, p3L2SYSCFG_TOK);
	set_token(level2, p3L2UPDATE, p3L2UPDATE_TOK);
	strcpy(level2[p3L2USER], p3L2USER_TOK);

	// Build Level 3 parser array
	i = sizeof(p3L3ACCEPT_TOK) + sizeof(p3L3ADDRESS_TOK) + sizeof(p3L3BUILDARRAY_TOK)
		+ sizeof(p3L3CONNECT_TOK) + sizeof(p3L3CONTROL_TOK) + sizeof(p3L3DATA_TOK)
		+ sizeof(p3L3DIRECTORY_TOK) + sizeof(p3L3FAIL_TOK)
		+ sizeof(p3L3FLUSHARRAY_TOK) + sizeof(p3L3IFCONFIG_TOK)
		+ sizeof(p3L3IPVER_TOK) + sizeof(p3L3LENGTH_TOK) + sizeof(p3L3LEVEL_TOK)
		+ sizeof(p3L3LOCALARRAY_TOK) + sizeof(p3L3LOGS_TOK)
		+ sizeof(p3L3MEMORY_TOK) + sizeof(p3L3NAME_TOK) + sizeof(p3L3NEW_TOK)
		+ sizeof(p3L3PACKETS_TOK) + sizeof(p3L3PASSWORD_TOK)
		+ sizeof(p3L3PERFORM_TOK) + sizeof(p3L3PORT_TOK) + sizeof(p3L3REPORT_TOK)
		+ sizeof(p3L3ROUTE_TOK) + sizeof(p3L3S2UNAME_TOK) + sizeof(p3L3SIZE_TOK)
		+ sizeof(p3L3STATUS_TOK) + sizeof(p3L3START_TOK) + sizeof(p3L3SUBNET_TOK)
		+ sizeof(p3L3TIME_TOK) + sizeof(p3L3UID_TOK) + sizeof(p3L3U2SNAME_TOK)
		+ sizeof(p3L3USE_TOK);
	if ((level3[0] = (char *) p3malloc(i)) == NULL) {
		sprintf(p3buf, "setup_admin: Failed to allocate Level 3\
 parser array: %s\n", strerror(errno));
		p3errmsg(p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	// NOTE: Insert new tokens in alphabetical order
	set_token(level3, p3L3ACCEPT, p3L3ACCEPT_TOK);
	set_token(level3, p3L3ADDRESS, p3L3ADDRESS_TOK);
	set_token(level3, p3L3BUILDARRAY, p3L3BUILDARRAY_TOK);
	set_token(level3, p3L3CONNECT, p3L3CONNECT_TOK);
	set_token(level3, p3L3CONTROL, p3L3CONTROL_TOK);
	set_token(level3, p3L3DATA, p3L3DATA_TOK);
	set_token(level3, p3L3DIRECTORY, p3L3DIRECTORY_TOK);
	set_token(level3, p3L3FAIL, p3L3FAIL_TOK);
	set_token(level3, p3L3FLUSHARRAY, p3L3FLUSHARRAY_TOK);
	set_token(level3, p3L3IFCONFIG, p3L3IFCONFIG_TOK);
	set_token(level3, p3L3IPVER, p3L3IPVER_TOK);
	set_token(level3, p3L3LENGTH, p3L3LENGTH_TOK);
	set_token(level3, p3L3LEVEL, p3L3LEVEL_TOK);
	set_token(level3, p3L3LOCALARRAY, p3L3LOCALARRAY_TOK);
	set_token(level3, p3L3LOGS, p3L3LOGS_TOK);
	set_token(level3, p3L3MEMORY, p3L3MEMORY_TOK);
	set_token(level3, p3L3NAME, p3L3NAME_TOK);
	set_token(level3, p3L3NEW, p3L3NEW_TOK);
	set_token(level3, p3L3PACKETS, p3L3PACKETS_TOK);
	set_token(level3, p3L3PASSWORD, p3L3PASSWORD_TOK);
	set_token(level3, p3L3PERFORM, p3L3PERFORM_TOK);
	set_token(level3, p3L3PORT, p3L3PORT_TOK);
	set_token(level3, p3L3REPORT, p3L3REPORT_TOK);
	set_token(level3, p3L3ROUTE, p3L3ROUTE_TOK);
	set_token(level3, p3L3S2UNAME, p3L3S2UNAME_TOK);
	set_token(level3, p3L3SIZE, p3L3SIZE_TOK);
	set_token(level3, p3L3START, p3L3START_TOK);
	set_token(level3, p3L3STATUS, p3L3STATUS_TOK);
	set_token(level3, p3L3SUBNET, p3L3SUBNET_TOK);
	set_token(level3, p3L3TIME, p3L3TIME_TOK);
	set_token(level3, p3L3U2SNAME, p3L3U2SNAME_TOK);
	set_token(level3, p3L3UID, p3L3UID_TOK);
	strcpy(level3[p3L3USE], p3L3USE_TOK);

	// Build Level 4 parser array
	i = sizeof(p3L4ADDRESS_TOK) + sizeof(p3L4IPVER_TOK)
		+ sizeof(p3L4PACKETS_TOK) + sizeof(p3L4TIME_TOK);
	
	if ((level4[0] = (char *) p3malloc(i)) == NULL) {
		sprintf(p3buf, "setup_admin: Failed to allocate Level 4\
 parser array: %s\n", strerror(errno));
		p3errmsg(p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	// NOTE: Insert new tokens in alphabetical order
	set_token(level4, p3L4ADDRESS, p3L4ADDRESS_TOK);
	set_token(level4, p3L4IPVER, p3L4IPVER_TOK);
	set_token(level4, p3L4PACKETS, p3L4PACKETS_TOK);
	strcpy(level4[p3L4TIME], p3L4TIME_TOK);

/* *** Print list of tokens
for (i=0; i < p3L1MAX; i++)
	printf("Level 1.%d: %s\n", i, level1[i]);
for (i=0; i < p3L2MAX; i++)
	printf("Level 2.%d: %s\n", i, level2[i]);
for (i=0; i < p3L3MAX; i++)
	printf("Level 3.%d: %s\n", i, level3[i]);
for (i=0; i < p3L4MAX; i++)
	printf("Level 4.%d: %s\n", i, level4[i]);
	// Parse host configuration file
	admin->config->flag &= ~p3CFG_PARMS;
 *** */

#ifdef _p3_PRIMARY
	admin->config->flag |= p3CFG_PRI;
	admin->config->flag |= (p3CFG_LOCAL | p3CFG_GROUP | p3CFG_REMOTE);
#endif
#ifdef _p3_SECONDARY
	admin->config->flag |= p3CFG_SEC;
	admin->config->flag |= (p3CFG_LOCAL | p3CFG_REMOTE);
#endif
	if (init_config(admin->config) < 0) {
		stat = -1;
		goto out;
	}
	if (get_config() < 0) {
		stat = -1;
		goto out;
	}
	if (init_command(admin->command) < 0) {
		stat = -1;
		goto out;
	}
	if (init_fifo() < 0) {
		stat = -1;
		goto out;
	}

out:
	return (stat);
} /* end setup_admin */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>setup_admin: Failed to allocate Level N parser array:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 administration interface attempts to allocate several levels of
 * parser arrays for passing commands and configuration data between the user
 * interface and main application.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * admin_request
 *
 * \par Description:
 * Handle a request string from the adminstrator user interface.
 *
 * \par Inputs:
 * - request: The request to be handled
 *
 * \par Outputs:
 * - char *: The response which may be one of the following:
 *   - OK: If no data is returned, this indicates success.
 *   - data: Successful return of data.
 *   - error message:  A string with the characters ERROR at the
 *     beginning indicates failure during processing.
 */

char *admin_request(char *request)
{
	int stat;
	char *response = NULL;

	if ((stat = parse_comm(request, &pcomm)) < 0) {
		response = get_errmsg();
	} else if (stat > 0) {
		sprintf(p3buf, "Error parsing command: %s", errmsg);
		p3errmsg(p3MSG_ERR, p3buf);
		response = get_errmsg();
	} else if (admin_handler(&pcomm) == NULL) {
		response = get_errmsg();
	} else {
		if (pcomm.tokens[1] == p3L2GET) {
			response = pcomm.data;
		} else {
			response = get_errmsg();
		}
	}

	return(response);
} /* end admin_request */

/**
 * \par Function:
 * admin_handler
 *
 * \par Description:
 * Handle input from the adminstrator user interface.
 * 
 * \par Inputs:
 * - uidata: Data from the user interface which is either
 *   terminated with either newline or NULL.
 *
 * \par Outputs:
 * - p3comm *: The response from either the library or the
 *   system application.  If there is an error, NULL is returned.
 */

p3comm *admin_handler(p3comm *uidata)
{
	int stat = 0;
	p3comm *uicomm = NULL;
	p3commlist *uilist;

	if (uidata == NULL) {
		goto out;
	}

//sprintf(p3buf, "Cmd Tokens: %d %d %d %d, IDs: %d %d, Data(%d)\n  |%s|\n",
//	uidata->tokens[0], uidata->tokens[1], uidata->tokens[2], uidata->tokens[3],
//	uidata->id1, uidata->id2, uidata->datlen, uidata->data);
//p3errlog(p3MSG_DEBUG, p3buf);
	if (uidata->tokens[1] == p3L2GET) {
		if (get_list(uidata, 0) == 0)
			uicomm = uidata;
	} else if ((uidata->tokens[0] == p3L1CMD || uidata->tokens[0] == p3L1SESSION)) {
		if (handle_cmd(uidata) < 0) {
			goto out;
		} else {
			uicomm = uidata;
			if (uicomm->datlen > 2) {
				uicomm->datlen = 3;
				strcpy(uicomm->data, "OK");
			}
		}
	// Handle get next p3comm item
	} else if (uidata->tokens[0] == p3L1GETNEXT) {
		if (commlist != NULL) {
			uicomm = commlist->comm;
			uilist = commlist;
			commlist = commlist->next;
			free(uilist);
		}
		if (commlist == NULL)
			cltail = NULL;
	} else if (uidata->tokens[0] == p3L1FREE) {
		if (uidata->data != NULL)
			free(uidata->data);
		free(uidata);
	// Handle configuration changes
	} else {
		if ((stat = handle_def(uidata)) < 0)
			goto out;
		uicomm = uidata;
	}

out:
	return (uicomm);
} /* end admin_handler */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>admin_handler: Parsing error: <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 administration interface received a command or configuration data
 * that incorrectly was formatted.
 * \par Response:
 * Notify the Velocite Systems P3 support team.
 *
 */

/**
 * \par Function:
 * init_comm
 *
 * \par Description:
 * Initialize a p3comm structure.  Note that the data length and pointer
 * are not modified which allows them to be reused.
 *
 * \par Inputs:
 * - comm_def: The structure for storing the results of the
 *   definition parsing.
 *
 * \par Outputs:
 * - None: The structure is initialized to appropriate values
 */

void init_comm(p3comm *comm_def)
{
	int i;

	for (i=0; i < p3MAX_TOKENS; i++)
		comm_def->tokens[i] = -1;
	comm_def->id1 = -1;
	comm_def->id2 = -1;
	comm_def->ID = admin->ID++;
	if (admin->ID < 0)
		admin->ID = 1;

} /* end init_comm */

/**
 * \par Function:
 * parse_comm
 *
 * \par Description:
 * Get the configuration definitions defined by the flags
 * set in the p3config structure.  The configuration input
 * is parsed to break out individual tokens and construct
 * a p3comm structure.  This is then used to handle the data.
 *
 * \par Inputs:
 * - comm_input: The configuration line to be parsed.
 * - comm_def: The structure for storing the results of the
 *   definition parsing.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - >0 = Top level token type not selected in flags
 *   - <0 = Error
 */

int parse_comm(char *comm_input, p3comm *comm_def)
{
	int i, idx, size, stat = 0;
	char *value = NULL, *token = comm_input;

	init_comm(comm_def);

	size = strlen(comm_input);
	if (comm_input[0] == '/')
		token++;
	// Break out tokens and find parameter value
	i = 0;
	value = NULL;
	for (idx=0; idx < size; idx++) {
		if (comm_input[idx] == '/' || comm_input[idx] == '\n' || comm_input[idx] == ' ')
			comm_input[idx] = '\0';
		else if (comm_input[idx] == '=') {
			i = 1;
			comm_input[idx] = '\0';
			// Set definition value to empty
			if (comm_def->data != NULL)
				comm_def->data[0] = '\0';
		} else if (i) {
			i = 0;
			// Reuse the value buffer if possible
			value = &comm_input[idx];
			if ((stat = get_comm_data(comm_def, strlen(value) + 1)) < 0)
				goto out;
		}
	}
	if (value != NULL)
		strcpy(comm_def->data, value);

	// Get level 1 index
	comm_def->tokens[0] = get_level_idx(level1, p3L1MAX, token);
	if (comm_def->tokens[0] < 0) {
		sprintf(errmsg, "No token found at top level");
		stat = 1;
		goto out;
	}
	next_token(token, &comm_input[size]);

	// Get level 2 index
	comm_def->tokens[1] = get_level_idx(level2, p3L2MAX, token);
	if (comm_def->tokens[1] < 0) {
		sprintf(errmsg, "No token found at second level");
		stat = 1;
		goto out;
	}
	// Set anonymous connection or user ID
	if (comm_def->tokens[1] == p3L2ANON || comm_def->tokens[1] == p3L2USER) {
		next_token(token, &comm_input[size]);
		comm_def->id1 = idx;
	}

	// Get level 3 index
	next_token(token, &comm_input[size]);
	comm_def->tokens[2] = get_level_idx(level3, p3L3MAX, token);
	if (comm_def->tokens[2] < 0) {
		goto out;
	}

	// Set subnet ID
	if (comm_def->tokens[1] == p3L3SUBNET) {
		next_token(token, &comm_input[size]);
		comm_def->id2 = idx;
	}

	// Get level 4 index
	next_token(token, &comm_input[size]);
	comm_def->tokens[3] = get_level_idx(level4, p3L4MAX, token);
	if (comm_def->tokens[3] < 0) {
		goto out;
	}

out:
	return(stat);
} /* end parse_comm */

/**
 * \par Function:
 * get_list
 *
 * \par Description:
 * Get a list of data for the user interface.
 *
 * \par Inputs:
 * - uicomm: The structure for storing the results of the
 *   definition parsing.
 * - update: If set, then the list is to be used to update
 *   configuration definitions in the system application.
 *   Otherwise, the list is to be used to provide the
 *   user interface with configuration information.
 *
 * \par Outputs:
 * - int: Status, note that the structure returns the
 *   number of items in the list
 *   - 0 = OK
 *   - <0 = Critical error
 */

int get_list(p3comm *uicomm, int update)
{
	int i, lsize = 0, stat = 0;
	int gettype = 0;
	char levels[p3MAX_TOKENS], num[16];
	p3local *local;
	p3remote *remote;
	p3subnet *subnet;
#ifdef _p3_PRIMARY
	int j, gid, usermt;
	char *enames[p3LHCFG_KEYALGMAX] = {"", "AES128", "AES256"};
	char *gnames[5] = {"", "ORG", "LOC", "SEC", "NET"};
	p3group *group;
	p3remotelist *rmtlist = NULL;
#endif

	switch (uicomm->tokens[0]) {
	case p3L1CMD:
		break;

	case p3L1LOCAL:
		for (i=1; i < p3MAX_TOKENS; i++)
			levels[i] = -1;
		if (admin->config != NULL && admin->config->local != NULL) {
			local = admin->config->local;
			if (update && !(local->flag & p3LHCFG_UPDATE))
				goto localresp;
			levels[0] = p3L1LOCAL;
			levels[1] = p3L2HOSTNAME;
			if ((stat = add_comm(levels, -1, -1, local->hostname)) < 0)
				goto out;
			lsize++;
			levels[1] = p3L2IPVER;
			if ((local->flag & p3HST_IPV6) &&
					(stat = add_comm(levels, -1, -1, "6")) < 0)
				goto out;
			else if ((local->flag & p3HST_IPV4) &&
					(stat = add_comm(levels, -1, -1, "4")) < 0)
				goto out;
			lsize++;
			levels[1] = p3L2ADDRESS;
			if ((stat = add_comm(levels, -1, -1, local->addr)) < 0)
				goto out;
			lsize++;
#ifdef _p3_PRIMARY
			levels[1] = p3L2PORT;
			sprintf(num, "%d", local->port);
			if ((stat = add_comm(levels, -1, -1, num)) < 0)
				goto out;
			lsize++;
			levels[1] = p3L2GENKEY;
			if ((local->flag & p3LOC_HWGEN) &&
					(stat = add_comm(levels, -1, -1, "HARDWARE")) < 0)
				goto out;
			else if (!(local->flag & p3LOC_HWGEN) &&
					(stat = add_comm(levels, -1, -1, "SOFTWARE")) < 0)
				goto out;
			lsize++;
			i = local->flag & p3LHCFG_KEYALG;
			if (0 < i && i < p3LHCFG_KEYALG) {
				levels[1] = p3L2ENCRYPTION;
				if ((stat = add_comm(levels, -1, -1, enames[i])) < 0)
					goto out;
				lsize++;
			}
#endif
			levels[2] = p3L3S2UNAME;
			if ((stat = add_comm(levels, -1, -1, local->s2uname)) < 0)
				goto out;
			lsize++;
			levels[2] = p3L3U2SNAME;
			if ((stat = add_comm(levels, -1, -1, local->u2sname)) < 0)
				goto out;
			lsize++;
#ifdef _p3_PRIMARY
			levels[1] = p3L2ARRAY;
			levels[2] = p3L3SIZE;
			sprintf(num, "%d", local->arraysz);
			if ((stat = add_comm(levels, -1, -1, num)) < 0)
				goto out;
			lsize++;
			levels[1] = p3L2HEARTBEAT;
			levels[2] = p3L3TIME;
			sprintf(num, "%d", local->heartbeat);
			if ((stat = add_comm(levels, -1, -1, num)) < 0)
				goto out;
			lsize++;
			levels[1] = p3L2HEARTBEAT;
			levels[2] = p3L3FAIL;
			sprintf(num, "%d", local->heartfail);
			if ((stat = add_comm(levels, -1, -1, num)) < 0)
				goto out;
			lsize++;
			levels[1] = p3L2REKEY;
			sprintf(num, "%d", local->rekey);
			if (local->flag & p3LHCFG_REKEYTM) {
				levels[2] = p3L3TIME;
				if ((stat = add_comm(levels, -1, -1, num)) < 0)
					goto out;
			} else if (local->flag & p3LHCFG_REKEYPKT) {
				levels[2] = p3L3PACKETS;
				if ((stat = add_comm(levels, -1, -1, num)) < 0)
					goto out;
			}
			lsize++;
			levels[1] = p3L2ARRAY;
			sprintf(num, "%d", local->datarray);
			if (local->flag & p3LHCFG_DATTM) {
				levels[2] = p3L3DATA;
				levels[3] = p3L4TIME;
				if ((stat = add_comm(levels, -1, -1, num)) < 0)
					goto out;
			} else if (local->flag & p3LHCFG_DATPKT) {
				levels[2] = p3L3DATA;
				levels[3] = p3L4PACKETS;
				if ((stat = add_comm(levels, -1, -1, num)) < 0)
					goto out;
			}
			lsize++;
			sprintf(num, "%d", local->ctlarray);
			if (local->flag & p3LHCFG_CTLTM) {
				levels[2] = p3L3CONTROL;
				levels[3] = p3L4TIME;
				if ((stat = add_comm(levels, -1, -1, num)) < 0)
					goto out;
			} else if (local->flag & p3LHCFG_CTLPKT) {
				levels[2] = p3L3CONTROL;
				levels[3] = p3L4PACKETS;
				if ((stat = add_comm(levels, -1, -1, num)) < 0)
					goto out;
			}
			lsize++;
#endif

#ifdef _p3_SECONDARY
			levels[1] = p3L2SUBNET;
			subnet = local->subnets;
			while (subnet != NULL) {
				levels[2] = p3L3IPVER;
				if ((subnet->flag & p3HST_IPV6) &&
						(stat = add_comm(levels, subnet->ID, -1, "6")) < 0)
					goto out;
				else if ((subnet->flag & p3HST_IPV4) &&
						(stat = add_comm(levels, subnet->ID, -1, "4")) < 0)
					goto out;
				lsize++;
				levels[2] = p3L3ADDRESS;
				if ((stat = add_comm(levels, subnet->ID, -1, subnet->addr)) < 0)
					goto out;
				lsize++;
				subnet = subnet->next;
			}
#endif
		}
localresp:
		sprintf(num, "%d", lsize);
		if ((stat = get_comm_data(uicomm, strlen(num) + 1)) < 0)
			goto out;
		strcpy(uicomm->data, num);
		break;

#ifdef _p3_PRIMARY
	case p3L1GROUP:
		group = admin->config->group;
		levels[0] = p3L1GROUP;
		if (strcmp(uicomm->data, "ALL") == 0)
			gettype = -1;
		else if (strcmp(uicomm->data, "ORG") == 0)
			gettype = p3GRP_ORG;
		else if (strcmp(uicomm->data, "LOC") == 0)
			gettype = p3GRP_LOC;
		else if (strcmp(uicomm->data, "SEC") == 0)
			gettype = p3GRP_SEC;
		else if (strcmp(uicomm->data, "NET") == 0)
			gettype = p3GRP_NET;
		else {
			p3errmsg(p3MSG_ERR, "Invalid get remote request\n");
			stat = -1;
			goto out;
		}
		while (group != NULL) {
			if (update && !(group->flag & p3LHCFG_UPDATE)) {
				group = group->next;
				continue;
			}
			if (gettype < 0 || group->level == gettype) {
				for (i=2; i < p3MAX_TOKENS; i++)
					levels[i] = -1;
				levels[1] = p3L2NAME;
				if ((stat = add_comm(levels, group->ID, -1, group->name)) < 0)
					goto out;
				lsize++;
				levels[1] = p3L2CATEGORY;
				if ((stat = add_comm(levels, group->ID, -1, gnames[group->level])) < 0)
					goto out;
				lsize++;
				i = group->flag & p3LHCFG_KEYALG;
				if (0 < i && i < p3LHCFG_KEYALG) {
					levels[1] = p3L2ENCRYPTION;
					if ((stat = add_comm(levels, group->ID, -1, enames[i])) < 0)
						goto out;
					lsize++;
				}
				if (group->arraysz > 0) {
					levels[1] = p3L2ARRAY;
					levels[2] = p3L3SIZE;
					sprintf(num, "%d", group->arraysz);
					if ((stat = add_comm(levels, group->ID, -1, num)) < 0)
						goto out;
					lsize++;
				}
				if (group->heartbeat > 0) {
					levels[1] = p3L2HEARTBEAT;
					levels[2] = p3L3TIME;
					sprintf(num, "%d", group->heartbeat);
					if ((stat = add_comm(levels, group->ID, -1, num)) < 0)
						goto out;
					lsize++;
				}
				if (group->heartfail > 0) {
					levels[1] = p3L2HEARTBEAT;
					levels[2] = p3L3FAIL;
					sprintf(num, "%d", group->heartfail);
					if ((stat = add_comm(levels, group->ID, -1, num)) < 0)
						goto out;
					lsize++;
				}
				if (group->rekey < 0) {
					levels[1] = p3L2REKEY;
					sprintf(num, "%d", group->rekey);
					if (group->flag & p3LHCFG_REKEYTM) {
						levels[2] = p3L3TIME;
						if ((stat = add_comm(levels, group->ID, -1, num)) < 0)
							goto out;
					} else if (group->flag & p3LHCFG_REKEYPKT) {
						levels[2] = p3L3PACKETS;
						if ((stat = add_comm(levels, group->ID, -1, num)) < 0)
							goto out;
					}
					lsize++;
				}
				if (group->datarray > 0) {
					levels[1] = p3L2ARRAY;
					sprintf(num, "%d", group->datarray);
					if (group->flag & p3LHCFG_DATTM) {
						levels[2] = p3L3DATA;
						levels[3] = p3L4TIME;
						if ((stat = add_comm(levels, group->ID, -1, num)) < 0)
							goto out;
					} else if (group->flag & p3LHCFG_DATPKT) {
						levels[2] = p3L3DATA;
						levels[3] = p3L4PACKETS;
						if ((stat = add_comm(levels, group->ID, -1, num)) < 0)
							goto out;
					}
					lsize++;
				}
				if (group->ctlarray > 0) {
					sprintf(num, "%d", group->ctlarray);
					if (group->flag & p3LHCFG_CTLTM) {
						levels[2] = p3L3CONTROL;
						levels[3] = p3L4TIME;
						if ((stat = add_comm(levels, group->ID, -1, num)) < 0)
							goto out;
					} else if (group->flag & p3LHCFG_CTLPKT) {
						levels[2] = p3L3CONTROL;
						levels[3] = p3L4PACKETS;
						if ((stat = add_comm(levels, group->ID, -1, num)) < 0)
							goto out;
					}
					lsize++;
				}
			}
			group = group->next;
		}
		sprintf(num, "%d", lsize);
		if ((stat = get_comm_data(uicomm, strlen(num) + 1)) < 0)
			goto out;
		strcpy(uicomm->data, num);
		break;
#endif

	case p3L1REMOTE:
		remote = admin->config->remote;
		levels[0] = p3L1REMOTE;
		if (strcmp(uicomm->data, "ALL") == 0)
			gettype = -1;
#ifdef _p3_PRIMARY
		else if (strncmp(uicomm->data, "GROUP", 5) == 0) {
			gettype = p3GROUPTYPE;
			if ((rmtlist = get_remote_spec(uicomm->data, gettype)) == NULL) {
				stat = -1;
				goto out;
			}
		} else if (strncmp(uicomm->data, "ID", 2) == 0) {
			gettype = p3REMOTETYPE;
			if ((rmtlist = get_remote_spec(uicomm->data, gettype)) == NULL) {
				stat = -1;
				goto out;
			}
		}
#endif
		else {
			p3errmsg(p3MSG_ERR, "Invalid get remote request\n");
			stat = -1;
			goto out;
		}
		while (remote != NULL) {
			if (update && !(remote->flag & p3LHCFG_UPDATE)) {
				remote = remote->next;
				continue;
			}
#ifdef _p3_PRIMARY
			if (gettype > 0) {
				usermt = 0;
				// OR groups to select remotes
				if (gettype == p3GROUPTYPE && rmtlist->len > 0) {
					for (j=0; j < p3MAX_GROUPS; j++) {
						get_rgroup(remote, j, gid);
						if (gid == 0xffff)
							continue;
						for (i=0; i < rmtlist->len; i++) {
							if (gid == rmtlist->list[i]) {
								usermt = 1;
								break;
							}
						}
						if (usermt)
							break;
					}
				// AND groups to select remotes
				} else if (gettype == p3GROUPTYPE && rmtlist->len < 0) {
					usermt = 1;
					for (i=0; i < (rmtlist->len * -1); i++) {
						for (j=0; j < p3MAX_GROUPS; j++) {
							get_rgroup(remote, j, gid);
printf("DEBUG: List (%d) %d, Remote group (%d) %d\n", i, rmtlist->list[i], j, gid);
							if (gid == rmtlist->list[i])
								break;
						}
						if (j == p3MAX_GROUPS) {
							usermt = 0;
							break;
						}
					}
				// Select remote from list
				} else if (gettype == p3REMOTETYPE) {
					for (i=0; i < (rmtlist->len); i++) {
						if (remote->ID == rmtlist->list[i]) {
							usermt = 1;
							break;
						}
					}
				}
				if (!usermt) {
					remote = remote->next;
					continue;
				}
			}
#endif
			for (i=2; i < p3MAX_TOKENS; i++)
				levels[i] = -1;
			levels[0] = p3L1REMOTE;
			levels[1] = p3L2HOSTNAME;
			if ((stat = add_comm(levels, remote->ID, -1,
					remote->hostname)) < 0)
				goto out;
			lsize++;
			levels[1] = p3L2IPVER;
			if ((remote->flag & p3HST_IPV6) &&
					(stat = add_comm(levels, remote->ID, -1, "6")) < 0)
				goto out;
			else if ((remote->flag & p3HST_IPV4) &&
					(stat = add_comm(levels, remote->ID, -1, "4")) < 0)
				goto out;
			lsize++;
			levels[1] = p3L2ADDRESS;
			if ((stat = add_comm(levels, remote->ID, -1, remote->addr)) < 0)
				goto out;
			lsize++;
#ifdef _p3_PRIMARY
			for (i=0; i < p3MAX_GROUPS; i++) {
				get_rgroup(remote, i, gid);
				if (gid != 0xffff) {
					levels[1] = p3L2CATEGORY;
					sprintf(p3buf, "%s,%s",
						gnames[grouplist[gid]->level], grouplist[gid]->name);
					if ((stat = add_comm(levels, remote->ID, -1, p3buf)) < 0)
						goto out;
					lsize++;
				}
			}
			i = remote->flag & p3LHCFG_KEYALG;
			if (0 < i && i < p3LHCFG_KEYALG) {
				levels[1] = p3L2ENCRYPTION;
				if ((stat = add_comm(levels, remote->ID, -1, enames[i])) < 0)
					goto out;
				lsize++;
			}
			if (remote->arraysz > 0) {
				levels[1] = p3L2ARRAY;
				levels[2] = p3L3SIZE;
				sprintf(num, "%d", remote->arraysz);
				if ((stat = add_comm(levels, remote->ID, -1, num)) < 0)
					goto out;
				lsize++;
			}
			if (remote->heartbeat > 0) {
				levels[1] = p3L2HEARTBEAT;
				levels[2] = p3L3TIME;
				sprintf(num, "%d", remote->heartbeat);
				if ((stat = add_comm(levels, remote->ID, -1, num)) < 0)
					goto out;
				lsize++;
			}
			if (remote->heartfail > 0) {
				levels[1] = p3L2HEARTBEAT;
				levels[2] = p3L3FAIL;
				sprintf(num, "%d", remote->heartfail);
				if ((stat = add_comm(levels, remote->ID, -1, num)) < 0)
					goto out;
				lsize++;
			}
			if (remote->rekey < 0) {
				levels[1] = p3L2REKEY;
				sprintf(num, "%d", remote->rekey);
				if (remote->flag & p3LHCFG_REKEYTM) {
					levels[2] = p3L3TIME;
					if ((stat = add_comm(levels, remote->ID, -1, num)) < 0)
						goto out;
				} else if (remote->flag & p3LHCFG_REKEYPKT) {
					levels[2] = p3L3PACKETS;
					if ((stat = add_comm(levels, remote->ID, -1, num)) < 0)
						goto out;
				}
				lsize++;
			}
			if (remote->datarray > 0) {
				levels[1] = p3L2ARRAY;
				sprintf(num, "%d", remote->datarray);
				if (remote->flag & p3LHCFG_DATTM) {
					levels[2] = p3L3DATA;
					levels[3] = p3L4TIME;
					if ((stat = add_comm(levels, remote->ID, -1, num)) < 0)
						goto out;
				} else if (remote->flag & p3LHCFG_DATPKT) {
					levels[2] = p3L3DATA;
					levels[3] = p3L4PACKETS;
					if ((stat = add_comm(levels, remote->ID, -1, num)) < 0)
						goto out;
				}
				lsize++;
			}
			if (remote->ctlarray > 0) {
				sprintf(num, "%d", remote->ctlarray);
				if (remote->flag & p3LHCFG_CTLTM) {
					levels[2] = p3L3CONTROL;
					levels[3] = p3L4TIME;
					if ((stat = add_comm(levels, remote->ID, -1, num)) < 0)
						goto out;
				} else if (remote->flag & p3LHCFG_CTLPKT) {
					levels[2] = p3L3CONTROL;
					levels[3] = p3L4PACKETS;
					if ((stat = add_comm(levels, remote->ID, -1, num)) < 0)
						goto out;
				}
				lsize++;
			}
#endif
			levels[2] = p3L3SUBNET;
			subnet = remote->subnets;
			while (subnet != NULL) {
				levels[3] = p3L4IPVER;
#ifdef _p3_PRIMARY
				if ((subnet->flag & p3HST_IPV6) &&
						(stat = add_comm(levels, subnet->ID,
								-1, "6")) < 0)
					goto out;
				else if ((subnet->flag & p3HST_IPV4) &&
						(stat = add_comm(levels, subnet->ID,
								-1, "4")) < 0)
					goto out;
#endif

#ifdef _p3_SECONDARY
				if ((subnet->flag & p3HST_IPV6) &&
						(stat = add_comm(levels, remote->ID,
								subnet->ID, "6")) < 0)
					goto out;
				else if ((subnet->flag & p3HST_IPV4) &&
						(stat = add_comm(levels, remote->ID,
								subnet->ID, "4")) < 0)
					goto out;
#endif
				lsize++;
				levels[3] = p3L4ADDRESS;
#ifdef _p3_PRIMARY
				if ((stat = add_comm(levels, subnet->ID, -1,
						subnet->addr)) < 0)
					goto out;
#endif

#ifdef _p3_SECONDARY
				if ((stat = add_comm(levels, remote->ID,
						subnet->ID, subnet->addr)) < 0)
					goto out;
#endif
				lsize++;
				subnet = subnet->next;
			}
			remote = remote->next;
		}
		sprintf(num, "%d", lsize);
		if ((stat = get_comm_data(uicomm, strlen(num) + 1)) < 0)
			goto out;
		strcpy(uicomm->data, num);
#ifdef _p3_PRIMARY
		if (rmtlist != NULL)
			free(rmtlist);
#endif
		break;

	case p3L1SESSION:

		sprintf(num, "%d", lsize);
		if ((stat = get_comm_data(uicomm, strlen(num) + 1)) < 0)
			goto out;
		strcpy(uicomm->data, num);
		break;

	default:
		p3errmsg(p3MSG_ERR, "get_list: Invalid command\n");
		break;
	}

out:
	return(stat);
} /* end init_comm */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>get_list: Invalid command</b>
 * \par Description (ERR):
 * The P3 administration library was requested to get a list of information
 * but the command requested was incorrect.
 * \par Response:
 * Notify the Velocite Systems P3 support team.
 *
 */

/**
 * \par Function:
 * add_comm
 *
 * \par Description:
 * Get a communication structure, set the values provided, and
 * add it to the communication list.
 *
 * \par Inputs:
 * - levels: The token indexes for each level
 * - id1, id2: The ID values for tokens that require them
 * - data: The data value string
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int add_comm(char *levels, int id1, int id2, char *data)
{
	int i, stat = 0;
	p3comm *comm = NULL;
	p3commlist *clist;

	if ((comm = (p3comm *) p3calloc(sizeof(p3comm))) == NULL) {
		sprintf(p3buf, "get_comm: Failed to allocate communication\
 structure: %s\n", strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}
	if ((clist = (p3commlist *) p3calloc(sizeof(p3commlist))) == NULL) {
		sprintf(p3buf, "get_comm: Failed to allocate communication\
 list structure: %s\n", strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		free(comm);
		stat = -1;
		goto out;
	}

	if (get_comm_data(comm, strlen(data) + 1) < 0) {
		free(comm);
		stat = -1;
		goto out;
	}
	strcpy(comm->data, data);
	for (i=0; i < p3MAX_TOKENS; i++) {
		comm->tokens[i] = levels[i];
	}
	comm->id1 = id1;
	comm->id2 = id2;

	clist->comm = comm;
	if (commlist == NULL) {
		commlist = clist;
		cltail = clist;
	} else {
		cltail->next = clist;
		cltail = clist;
	}

out:
	return(stat);
} /* end add_comm */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>get_comm: Failed to allocate communication list structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 administration library attempts to allocate a communication list
 * structure for response lists.  If this fails, there is a system wide
 * problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

#ifdef _p3_PRIMARY
/**
 * \par Function:
 * get_remote_spec
 *
 * \par Description:
 * Get the specification for secondary remotes when the request is
 * not for ALL remotes.
 *
 * \par Inputs:
 * - spec: The specification string in the get remotes request.
 * - type: The ID type:
 *   - p3GROUPTYPE = Group IDs
 *   - p3REMOTETYPE = Remote IDs
 *
 * \par Outputs:
 * - p3remotelist *: The list information for the ID numbers, which
 *   includes the number of items and an array of integers holding
 *   the list of IDs.  If the number of items is negative, the IDs
 *   are to be ANDed.  Otherwise, the IDs are ORed.
 *   If there is an error, NULL is returned.
 */

p3remotelist *get_remote_spec(char *spec, int type)
{
	int i, len, idx, items;
	int sep = 0, combine = 1;
	unsigned long l;
	p3remotelist *idlist = NULL;

	if (type == p3GROUPTYPE)
		idx = 5;
	else if (type == p3REMOTETYPE)
		idx = 2;
	else {
		p3errmsg(p3MSG_ERR,
			"Invalid get remote specification: Unknown type\n");
		goto out;
	}
	len = strlen(spec);
	while (idx < len) {
		if (spec[idx] == ',' || spec[idx] == ' ') {
			spec[idx] = '\0';
			idx++;
		} else
			break;
	}
	if (idx >= len) {
		sprintf(p3buf, "Invalid get remote specification: %s\n", spec);
		p3errmsg(p3MSG_ERR, p3buf);
		goto out;
	}
	items = 0;
	while (idx < len) {
		if (spec[idx] == '+' || spec[idx] == ',' || spec[idx] == ' ') {
			if (spec[idx] == '+')
				combine = -1;
			spec[idx] = '\0';
			if (!sep) {
				sep = 1;
				items++;
			}
		} else if (spec[idx] < '0' || '9' < spec[idx]) {
			p3errmsg(p3MSG_ERR,
				"Invalid get remote specification: Nonumeric value\n");
			goto out;
		} else
			sep = 0;
		idx++;
	}
	items++;

	i = sizeof(p3commlist) + (items * sizeof(int));
	if ((idlist = (p3remotelist *) p3calloc(i)) == NULL) {
		sprintf(p3buf, "get_comm: Failed to allocate remote list\
 structure: %s\n", strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		goto out;
	}
	l = (unsigned long) idlist + sizeof(p3commlist);
	idlist->list = (int *) l;

	idlist->len = items * combine;
	if (type == p3GROUPTYPE)
		idx = 5;
	else if (type == p3REMOTETYPE)
		idx = 2;
	i = 0;
	while (idx < len) {
		if (spec[idx] == '\0') {
			idx++;
			continue;
		}
		idlist->list[i++] = strtol(&spec[idx], NULL, 10);
		idx += strlen(&spec[idx]);
	}

out:
	return(idlist);
} /* end get_remote_spec */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>get_remote_spec: Invalid get remote specification:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * The request was made to retrieve specific remote host definitions
 * was in an incorrect format.
 * \par Response:
 * Notify the Velocite Systems P3 support team.
 *
 * <hr><b>get_remote_spec: Failed to allocate remote list structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 administration library attempts to allocate a remote list
 * structure for get remote requests.  If this fails, there is a system
 * wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

#endif

/**
 * \par Function:
 * admin_shutdown
 *
 * \par Description:
 * Shutdown the administration library cleanly.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Critical error
 *   - >0 = Parsing error
 */

int admin_shutdown()
{
	int stat = 0;

	stat = command_shutdown();

	return (stat);
} /* end admin_shutdown */

#ifdef _p3JNI
/**
 * \par Function:
 * adminAPI
 *
 * \par Description:
 * Called by programs that cannot pass the C standard
 * argument counter and vector to initialize the P3
 * administration library.
 *
 * \par Inputs:
 * - args: Argument list
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

JNIEXPORT jbyteArray JNICALL Java_p3priAPI_initAdmin
  (JNIEnv *env, jobject jobj, jstring jargs)
{
	int i, idx = 0, len, argc = 1;
	jboolean iscopy;
	const char *args = (*env)->GetStringUTFChars(env, jargs, &iscopy);
	char *abegin = (char *) args, *response = NULL;
	char **argv;
	jbyteArray jb;

	// Tokenize argument list
printf("DEBUG: Tokenize:\n  |%s|\n", args);
	len = strlen(args);
	while (idx < len && args[idx] == ' ') {
		idx++;
		len--;
		abegin++;
	}
	while (idx < len) {
		if (abegin[idx] == ' ') {
			abegin[idx++] = '\0';
			argc++;
			while (idx < len && abegin[idx] == ' ')
				abegin[idx++] = '\0';
		} else
			idx++;
	}
	if (abegin[len - 1] != '\0')
		argc++;
	i = (argc + 1) * sizeof(char *);
	if ((argv = (char **) p3malloc(i)) == NULL) {
		sprintf(p3buf, "init_admin: Failed to allocate argument list\
 vector: %s\n", strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		response = get_errmsg();
		goto out;
	}
	argv[0] = appname;
	argv[argc] = NULL;
	idx = 0;
	for (i=1; i < argc; i++) {
		argv[i] = &abegin[idx];
printf("DEBUG: Arg %d: %s\n", i, argv[i]);
		idx += strlen(&abegin[idx]);
		while (idx < len && abegin[idx] == '\0')
			idx++;
	}
	if (init_admin(argc, argv) != 0)
		response = get_errmsg();
	else {
		response = (char *) p3malloc(3);
		strcpy(response, "OK");
	}

out:
	if (response != NULL) {
		i = strlen(response);
		jb=(*env)->NewByteArray(env, i);
		(*env)->SetByteArrayRegion(env, jb, 0, i, (jbyte *)response);
	} else {
		jb=(*env)->NewByteArray(env, 2);
		(*env)->SetByteArrayRegion(env, jb, 0, 2, (jbyte *)"OK");
	}
	return(jb);
}

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>adminAPI: Failed to allocate argument list vector:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 administration interface attempts to allocate an argument list
 * vector to hold arguments from the command line.  If this fails, there
 * is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * sendRequest
 *
 * \par Description:
 * Called by programs using the JNI interface to make an
 * administrative request.
 *
 * \par Inputs:
 * - jrequest: The request Java string.
 *
 * \par Outputs:
 * - jbyteArray: A java byte array containing the response.
 */

JNIEXPORT jbyteArray JNICALL Java_p3priAPI_sendRequest
  (JNIEnv *env, jobject jobj, jstring jrequest)
{
	int sz;
	char *response;
	jboolean iscopy;
	const char *request = (*env)->GetStringUTFChars(env, jrequest, &iscopy);
	jbyteArray jb;

	if (request == NULL) {
	    jb=(*env)->NewByteArray(env, 2);
	    (*env)->SetByteArrayRegion(env, jb, 0, 2, (jbyte *)"");
	    goto out;
	}
	response = admin_request((char *)request);
	(*env)->ReleaseStringUTFChars(env, jrequest, request);
	sz = strlen(response) + 1;
	if (sz > 1) {
		jb=(*env)->NewByteArray(env, sz);
		(*env)->SetByteArrayRegion(env, jb, 0, sz, (jbyte *)response);
	} else {
		jb=(*env)->NewByteArray(env, 2);
		(*env)->SetByteArrayRegion(env, jb, 0, 2, (jbyte *)"OK");
	}

out:
	return(jb);
}

/**
 * \par Function:
 * getNextRequest
 *
 * \par Description:
 * Called by programs using the JNI interface to make an
 * administrative request.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - jbyteArray: A java byte array containing the response.
 */

JNIEXPORT jbyteArray JNICALL Java_p3priAPI_getNextRequest
  (JNIEnv *env, jobject jobj)
{
	int i, sz, id;
	char *errmsg;
	p3comm *plist;
	jbyteArray jb;

	init_comm(&pcomm);
	pcomm.tokens[0] = p3L1GETNEXT;
	if ((plist = admin_handler(&pcomm)) == NULL) {
		errmsg = get_errmsg();
		sz = strlen(errmsg) + 1;
		jb=(*env)->NewByteArray(env, sz);
		(*env)->SetByteArrayRegion(env, jb, 0, sz, (jbyte *)errmsg);
	} else {
		p3buf[0] = '\0';
		id = plist->id1;
		if (plist->tokens[0] >= 0) {
			i = plist->tokens[0];
			sprintf(p3buf, "%s/%s", p3buf, level1[i]);
		}
		if (plist->tokens[0] == p3L1REMOTE || plist->tokens[0] == p3L1GROUP) {
			sprintf(p3buf, "%s/%d", p3buf, id);
			id = plist->id2;
		}
		if (plist->tokens[1] >= 0) {
			i = plist->tokens[1];
			sprintf(p3buf, "%s/%s", p3buf, level2[i]);
		}
		if (plist->tokens[1] == p3L2SUBNET || plist->tokens[1] == p3L2USER ||
				plist->tokens[1] == p3L2ANON) {
			sprintf(p3buf, "%s/%d", p3buf, id);
			id = plist->id2;
		}
		if (plist->tokens[2] >= 0) {
			i = plist->tokens[2];
			sprintf(p3buf, "%s/%s", p3buf, level3[i]);
		}
		if (plist->tokens[2] == p3L3SUBNET) {
			sprintf(p3buf, "%s/%d", p3buf, id);
		}
		if (plist->tokens[3] >= 0) {
			i = plist->tokens[3];
			sprintf(p3buf, "%s/%s", p3buf, level4[i]);
		}
		sprintf(p3buf, "%s=%s", p3buf, plist->data);
		sz = strlen(p3buf) + 1;
		jb=(*env)->NewByteArray(env, sz);
		(*env)->SetByteArrayRegion(env, jb, 0, sz, (jbyte *)p3buf);
		// Free communication structure list element
		plist->tokens[0] = p3L1FREE;
		admin_handler(plist);
	}

	return(jb);
}

#endif /* _p3JNI */
