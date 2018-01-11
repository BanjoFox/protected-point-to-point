/**
 * \file p3command.c
 * <h3>Protected Point to Point command interface file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * The command interface is used to manage communication with the
 * P3 System.
 */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <p><hr><hr>
 * \section P3_COMMAND P3 Command Handler Messages
 */

#define _p3_COMMAND_C
#include "p3command.h"
#include "p3anonymous.h"
#include "p3admin.h"
#include "p3fifo.h"

/** Main command structure */
p3command *command;

/**
 * \par Function:
 * init_command
 *
 * \par Description:
 * Initialize and run the P3 command library functions.
 * 
 * \par Inputs:
 * - admin_main: The main administration structure
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */
int init_command(p3command *admin_cmd)
{
	int stat = 0;

	command = admin_cmd;

	// Get the System to UI FIFO name
	if (command->config->local->s2uname == NULL) {
		// If not set on the command line, use the default
		if ((command->config->local->s2uname = (char *)
				p3malloc(sizeof(p3LOC_S2UNAME))) == NULL) {
			sprintf(p3buf, "init_config: Failed to allocate System\
 to UI FIFO name: %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
		strcpy(command->config->local->s2uname, p3LOC_S2UNAME);
	}

	// Get the UI to System FIFO name
	if (command->config->local->u2sname == NULL) {
		// If not set on the command line, use the default
		if ((command->config->local->u2sname = (char *)
				p3malloc(sizeof(p3LOC_U2SNAME))) == NULL) {
			sprintf(p3buf, "init_config: Failed to allocate System\
 to UI FIFO name: %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
		strcpy(command->config->local->u2sname, p3LOC_U2SNAME);
	}

#ifdef _p3_PRIMARY
	if ((stat = init_anonymous(command)) < 0) {
		goto out;
	}
#endif

	// Open FIFO
	// Verify P3 System is running

out:
	return(stat);
}

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>init_command: Failed to allocate FIFO directory name:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 command handler attempts to allocate the default FIFO directory
 * where the System to UI named pipe is created.  If this fails,
 * there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>init_command: Failed to allocate <i>direction</i> FIFO name:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 command handler attempts to allocate the names of the FIFOs,
 * one in each direction.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * handle_cmd
 *
 * \par Description:
 * Handle input from the P3 System.
 * 
 * \par Inputs:
 * - comm: The p3comm structure created by parsing the communication.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int handle_cmd(p3comm *comm)
{
	int stat = 0;

//sprintf(p3buf, "Cmd Tokens: %d %d %d %d, IDs: %d %d, Data(%d)\n  |%s|\n",
//	comm->tokens[0], comm->tokens[1], comm->tokens[2], comm->tokens[3],
//	comm->id1, comm->id2, comm->datlen, comm->data);
//p3errlog(p3MSG_DEBUG, p3buf);

	if (comm->tokens[0] == p3L1CMD) {
		if ((stat = handle_command(comm)) < 0) {
			goto out;
		} else if (stat > 0) {
			sprintf(p3buf, "handle_cmd: Error in command: %s\n",
					errmsg);
			p3errmsg (p3MSG_WARN, p3buf);
			stat = 0;
		}
	} else if (comm->tokens[0] == p3L1SESSION) {
		if ((stat = handle_session(comm)) < 0) {
			goto out;
		} else if (stat > 0) {
			sprintf(p3buf, "handle_cmd: Error in command: %s\n",
					errmsg);
			p3errmsg (p3MSG_WARN, p3buf);
			stat = 0;
		}
	}

out:
	return (stat);
} /* end handle_cmd */

/**
 * \par Function:
 * handle_command
 *
 * \par Description:
 * Handle the commands by forwarding data appropriately:
 * - Forward requests from the UI to the P3 System.
 * - Forward responses from the P3 System to the UI.
 *
 * \par Inputs:
 * - cmd: The parsed definition values.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - >0 = Parser error
 *   - <0 = System error
 */

int handle_command(p3comm *cmd)
{
	int i, stat = 0;
	char *argv[4], arg0[p3SYSCMDSZ], arg1[3] = "-a";
	p3user *user;
	p3comm *updtcomm = NULL;
	p3commlist *updtlist;
	p3fifomsg fmsg;

	switch (cmd->tokens[1]) {

	case p3L2USER:
		user = admin->users;
		while (user != NULL) {
			if (user->ID == cmd->id1)
				break;
			user = user->next;
		}
		if (user == NULL) {
			if ((user = (p3user *) p3calloc(sizeof(p3user))) == NULL) {
				sprintf(p3buf, "handle_command: Failed to allocate user\
 structure: %s\n", strerror(errno));
				p3errmsg (p3MSG_CRIT, p3buf);
				stat = -1;
				goto out;
			}
			user->ID = cmd->id2;
			// Add remote to head of list
			if (admin->users != NULL)
				user->next = admin->users;
			admin->users = user;
		}
		if (cmd->tokens[2] == p3L3NAME) {
			if ((user->name = (char *) p3malloc(cmd->datlen)) == NULL) {
				sprintf(p3buf, "handle_command: Failed to allocate user\
 name buffer: %s\n", strerror(errno));
				p3errmsg (p3MSG_CRIT, p3buf);
				stat = -1;
				goto out;
			}
			strcpy(user->name, cmd->data);
		} else if (cmd->tokens[2] == p3L3UID) {
			if ((user->uid = (char *) p3malloc(cmd->datlen)) == NULL) {
				sprintf(p3buf, "handle_command: Failed to allocate user ID\
 buffer: %s\n", strerror(errno));
				p3errmsg (p3MSG_CRIT, p3buf);
				stat = -1;
				goto out;
			}
			strcpy(user->uid, cmd->data);
		} else if (cmd->tokens[2] == p3L3PASSWORD) {
		} else if (cmd->tokens[2] == p3L3LEVEL) {
			i = strtol(cmd->data, NULL, 10);
			if (i < 1 || 3 > i) {
				sprintf(errmsg, "Invalid user level: %s", cmd->data);
				stat = 1;
				goto out;
			}
			user->level = i;
		}
		break;

	case p3L2UPDATE:
		stat = save_config();
		if (cmd->data == NULL || strcmp(cmd->data, "ALL") == 0)
			i = 7;
		else if (strcmp(cmd->data, "LOCAL") == 0)
			i = 1;
		else if (strcmp(cmd->data, "GROUP") == 0)
			i = 2;
		else if (strcmp(cmd->data, "REMOTE") == 0)
			i = 4;
		else {
			sprintf(errmsg, "Invalid configuration update request");
			stat = 1;
			goto out;
		}
		if ((stat = get_comm_data(cmd, 10)) < 0)
			goto out;
		if (i & 1) {
			cmd->tokens[0] = p3L1LOCAL;
			if ((stat = get_list(cmd, 1)) < 0)
				goto out;
		}
#ifdef _p3_PRIMARY
		if (i & 2) {
			cmd->tokens[0] = p3L1GROUP;
			strcpy(cmd->data, "ALL");
			if ((stat = get_list(cmd, 1)) < 0)
				goto out;
		}
#endif
		if (i & 4) {
			cmd->tokens[0] = p3L1REMOTE;
			strcpy(cmd->data, "ALL");
			if ((stat = get_list(cmd, 1)) < 0)
				goto out;
		}
		if (commlist == NULL)
			goto out;
		while (commlist != NULL) {
			updtcomm = commlist->comm;
			updtcomm->ID = admin->ID++;
			updtlist = commlist;
			commlist = commlist->next;
			free(updtlist);
			fmsg.flag = p3FMSG_RSVP;
			i = send_request(updtcomm, &fmsg);
			free (updtcomm);
			if (i < 0)
				stat = -1;
		}
		break;

	case p3L2STATUS:
		if (cmd->tokens[2] == p3L3LENGTH) {
			i = strtol(cmd->data, NULL, 10);
			if (i > 0)
				command->stat_len = i;
		} else if (cmd->tokens[2] == p3L3START) {
		}
		break;

	case p3L2SYSCFG:
		if (cmd->tokens[2] == p3L3IFCONFIG) {
			strcpy(arg0, p3IFCONFIG);
			argv[0] = arg0;
			argv[1] = arg1;
			if (cmd->data != NULL && strlen(cmd->data) > 1) {
				argv[2] = cmd->data;
				argv[3] = NULL;
			} else
				argv[2] = NULL;
			external_cmd(p3IFCONFIG, argv, cmd);
			// Return result to UI
		} else if (cmd->tokens[2] == p3L3ROUTE) {
			strcpy(arg0, p3ROUTE);
			argv[0] = arg0;
			if (cmd->data != NULL && strlen(cmd->data) > 1) {
				argv[1] = cmd->data;
				argv[2] = NULL;
			} else
				argv[1] = NULL;
			external_cmd(p3ROUTE, argv, cmd);
			// Return result to UI
		} else if (cmd->tokens[2] == p3L3MEMORY) {
			strcpy(arg0, p3FREE);
			argv[0] = arg0;
			argv[1] = NULL;
			external_cmd(p3FREE, argv, cmd);
			// Return result to UI
		}
		p3errmsg(p3MSG_NONE, cmd->data);
		break;

	default:
		sprintf(errmsg, "Invalid command field: %s",
				level2[(unsigned int) cmd->tokens[1]]);
		stat = 1;
		break;

	}

out:
	return(stat);
} /* end handle_command */

/**
 * \par Function:
 * handle_session
 *
 * \par Description:
 * Handle the commands by forwarding data appropriately:
 * - Forward requests from the UI to the P3 System.
 * - Forward responses from the P3 System to the UI.
 *
 * \par Inputs:
 * - sess: The parsed definition values.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - >0 = Parser error
 *   - <0 = System error
 */

int handle_session(p3comm *sess)
{
	int stat = 0;
#ifdef _p3_PRIMARY
	int i;
	p3remote *remote, *trmt;
	p3subnet *subnet;
#endif

	switch (sess->tokens[1]) {

#ifdef _p3_PRIMARY
	case p3L2ANONSET:
sprintf(p3buf, "Handle anonset\n");
p3errlog(p3MSG_DEBUG, p3buf);
		if (strncmp(sess->data, "OFF", sess->datlen) == 0) {
			// Attach new shared memory blocks
			if (attach_shares() < 0) {
				stat = -1;
				goto out;
			}
			command->flag &= ~(p3CMD_ANONSET | p3CMD_ANONNET | p3CMD_ANONSRDY);
			// Terminate listener
			if (command->anonpid > 0) {
				kill(command->anonpid, SIGTERM);
				// TODO: Use SIGKILL if shutdown unsuccessful
				command->anonpid = -1;
			}
		} else if (strncmp(sess->data, "ON", sess->datlen) == 0) {
			command->flag &= ~(p3CMD_ANONNET | p3CMD_ANONSRDY);
			command->flag |= p3CMD_ANONSET;
			if (anon_handler() < 0) {
				command->flag &= ~(p3CMD_ANONSET | p3CMD_ANONNET);
			}
		} else if (strncmp(sess->data, "ONLY", sess->datlen) == 0) {
			command->flag |= p3CMD_ANONSET | p3CMD_ANONNET;
			if ((command->flag & p3CMD_ANONSRDY) && anon_handler() < 0) {
				command->flag &= ~(p3CMD_ANONSET | p3CMD_ANONNET | p3CMD_ANONSRDY);
			}
		} else {
			sprintf(errmsg, "Anonymous settings must be OFF, ON, or ONLY");
			stat = 1;
		}
		break;

	case p3L2ANONIPVER:
		if (sess->data[0] == '4' && !(command->flag & p3HST_IPV6)) {
			command->flag &= ~p3LHCFG_IPVER;
			command->flag |= p3HST_IPV4;
		} else if (sess->data[0] == '6' && !(command->flag & p3HST_IPV4)) {
			command->flag &= ~p3LHCFG_IPVER;
			command->flag |= p3HST_IPV6;
		} else {
			sprintf(errmsg, "Invalid subnet IP version: %s", sess->data);
			stat = 1;
		}
		break;

	case p3L2ANONNET:
		// If not IPv6, test address for IPv4
		if (!(command->flag & p3HST_IPV6)) {
			if (parse_ipaddr(sess->data, &command->subnet.v4, &command->mask.v4,
					(p3HST_IPV4 | p3ADR_MASK), errmsg) < 0) {
				stat = 1;
				goto out;
			}
			command->flag |= p3HST_IPV4;
		// If not IPv4, test address for IPv6
		} else if (!(command->flag & p3HST_IPV4)) {
			if (parse_ipaddr(sess->data, &command->subnet.v6, &command->mask.v6,
					(p3HST_IPV6 | p3ADR_MASK), errmsg) < 0) {
				stat = 1;
				goto out;
			}
			command->flag |= p3HST_IPV6;
		// Both IPv4 and IPv6 set
		} else {
			command->flag &= ~p3LHCFG_IPVER;
			sprintf(errmsg, "Error in IP version");
			stat = 1;
			goto out;
		}
		command->flag |= p3CMD_ANONSRDY;
		if ((command->flag & p3CMD_ANONSET) && anon_handler() < 0) {
			command->flag &= ~(p3CMD_ANONSET | p3CMD_ANONNET | p3CMD_ANONSRDY);
		}
		break;

	case p3L2ANONPORT:
		i = strtol(sess->data, NULL, 10);
		if (NOT_PORT(i)) {
			sprintf(errmsg, "Invalid anonymous port: %d", i);
			stat = 1;
			goto out;
		}
		command->aport = i;
		break;

	case p3L2ANONCMD:
		if (sess->tokens[2] == p3L3REPORT) {
			if (strncmp(sess->data, "ALL", 3) == 0) {
				remote = command->aremotes;
				while (remote != NULL) {
					if (!(remote->flag & p3RMT_REPORT)) {
						// Send remote definition to user interface
					}
					remote = remote->next;
				}
			} else if (strncmp(sess->data, "FLUSH", 3) == 0) {
				remote = command->aremotes;
				trmt = NULL;
				while (remote != NULL) {
					// Remove from queue
					if (remote->flag & p3RMT_REPORT) {
						if (trmt == NULL)
							command->aremotes = remote->next;
						else
							trmt->next = remote->next;
						remote = remote->next;
					} else {
						trmt = remote;
						remote = remote->next;
					}
				}
			}
		} else if (sess->tokens[2] == p3L3ACCEPT) {
			// Find selected remote definition
			remote = command->aremotes;
			trmt = NULL;
			while (remote != NULL) {
				if (remote->ID == sess->id1) {
					remote->ID = command->config->remotes++;
					// Remove from anonymous queue
					if (trmt == NULL)
						command->aremotes = remote->next;
					else
						trmt->next = remote->next;
					remote->next = NULL;
					// Put on remote definition queue
					if (command->config->remote == NULL) {
						command->config->remote = remote;
						command->config->rmttail = remote;
					} else {
						command->config->rmttail->next = remote;
						command->config->rmttail = remote;
					}
					// TODO: Write to configuration file
					break;
				}
				trmt = remote;
				remote = remote->next;
			}
		}
		break;

	case p3L2ANON:
		remote = command->aremotes;
		while (remote != NULL) {
			if (remote->ID == sess->id2)
				break;
			remote = remote->next;
		}
		if (remote == NULL) {
			if ((remote = (p3remote *) p3calloc(sizeof(p3remote))) == NULL) {
				sprintf(p3buf, "handle_local_defs: Failed to allocate remote\
 structure: %s\n", strerror(errno));
				p3errmsg (p3MSG_CRIT, p3buf);
				stat = -1;
				goto out;
			}
			remote->ID = sess->id2;
			// Add remote to head of list
			if (command->aremotes != NULL)
				remote->next = command->aremotes;
			command->aremotes = remote;
		}
		if (sess->tokens[2] == p3L3ADDRESS) {
			// If not IPv6, test address for IPv4
			if (!(remote->flag & p3HST_IPV6)) {
				if (parse_ipaddr(sess->data, NULL, NULL,
						(p3HST_IPV4 | p3ADR_STR), errmsg) < 0) {
					stat = 1;
					goto out;
				}
				remote->flag |= p3HST_IPV4;
			// If not IPv4, test address for IPv6
			} else if (!(remote->flag & p3HST_IPV4)) {
				if (parse_ipaddr(sess->data, NULL, NULL,
						(p3HST_IPV6 | p3ADR_STR), errmsg) < 0) {
					stat = 1;
					goto out;
				}
				remote->flag |= p3HST_IPV6;
			// Both IPv4 and IPv6 set
			} else {
				remote->flag &= ~p3LHCFG_IPVER;
				sprintf(errmsg, "Error in IP version");
				stat = 1;
				goto out;
			}
			if ((remote->addr = (char *) p3malloc(sess->datlen))
					== NULL) {
				sprintf(p3buf, "handle_local_defs: Failed to allocate remote\
	 address buffer: %s\n", strerror(errno));
				p3errmsg (p3MSG_CRIT, p3buf);
				stat = -1;
				goto out;
			}
			strcpy(remote->addr, sess->data);
		} else if (sess->tokens[2] == p3L3SUBNET) {
			subnet = command->aremotes->subnets;
			while (subnet != NULL) {
				if (subnet->ID == sess->id2)
					break;
				subnet = subnet->next;
			}
			if (subnet == NULL) {
				if ((subnet = (p3subnet *) p3calloc(sizeof(p3subnet))) == NULL) {
					sprintf(p3buf, "handle_local_defs: Failed to allocate subnet\
	 structure: %s\n", strerror(errno));
					p3errmsg (p3MSG_CRIT, p3buf);
					stat = -1;
					goto out;
				}
				subnet->ID = sess->id2;
				// Add subnet to head of list
				if (command->aremotes->subnets != NULL)
					subnet->next = command->aremotes->subnets;
				command->aremotes->subnets = subnet;
			}
			if (sess->tokens[3] == p3L3IPVER) {
				if (sess->data[0] == '4' && !(subnet->flag & p3HST_IPV6)) {
					subnet->flag &= ~p3LHCFG_IPVER;
					subnet->flag |= p3HST_IPV4;
				} else if (sess->data[0] == '6' && !(subnet->flag & p3HST_IPV4)) {
					subnet->flag &= ~p3LHCFG_IPVER;
					subnet->flag |= p3HST_IPV6;
				} else {
					sprintf(errmsg, "Invalid subnet IP version: %s",
							sess->data);
					stat = 1;
				}
			} else if (sess->tokens[3] == p3L3ADDRESS) {
				// If not IPv6, test address for IPv4
				if (!(subnet->flag & p3HST_IPV6)) {
					if (parse_ipaddr(sess->data, NULL, NULL,
							(p3HST_IPV4 | p3ADR_STR), errmsg) < 0) {
						stat = 1;
						goto out;
					}
					subnet->flag |= p3HST_IPV4;
				// If not IPv4, test address for IPv6
				} else if (!(subnet->flag & p3HST_IPV4)) {
					if (parse_ipaddr(sess->data, NULL, NULL,
							(p3HST_IPV6 | p3ADR_STR), errmsg) < 0) {
						stat = 1;
						goto out;
					}
					subnet->flag |= p3HST_IPV6;
				// Both IPv4 and IPv6 set
				} else {
					subnet->flag &= ~p3LHCFG_IPVER;
					sprintf(errmsg, "Error in IP version");
					stat = 1;
					goto out;
				}
				if ((subnet->addr = (char *) p3malloc(sess->datlen))
						== NULL) {
					sprintf(p3buf, "handle_local_defs: Failed to allocate remote\
	 subnet address buffer: %s\n", strerror(errno));
					p3errmsg (p3MSG_CRIT, p3buf);
					stat = -1;
					goto out;
				}
				strcpy(subnet->addr, sess->data);
			}
		}
		break;

	case p3L2STATUS:
		if (sess->tokens[2] == p3L3CONNECT) {
		} else if (sess->tokens[2] == p3L3PERFORM) {
		} else if (sess->tokens[2] == p3L3LOGS) {
		} else if (sess->tokens[2] == p3L3STATUS) {
		}
		break;

	case p3L2KEY:
		if (sess->tokens[2] == p3L3NEW) {
		} else if (sess->tokens[2] == p3L3ADDRESS) {
		} else if (sess->tokens[2] == p3L3BUILDARRAY) {
		} else if (sess->tokens[2] == p3L3LOCALARRAY) {
		} else if (sess->tokens[2] == p3L3FLUSHARRAY) {
		}
		break;
#endif

#ifdef _p3_SECONDARY
	case p3L2ANONCMD:
		if (sess->tokens[2] == p3L3CONNECT) {
			if ((stat = init_anonymous(command)) < 0)
				goto out;
		}
		break;
#endif

	default:
		sprintf(errmsg, "Invalid command field: %s",
				level2[(unsigned int) sess->tokens[1]]);
		stat = 1;
		break;

	}

out:
	return(stat);
} /* end handle_session */

/**
 * \par Function:
 * external_cmd
 *
 * \par Description:
 * Execute an external command and return the resulting output
 * in the supplied string buffer.
 *
 * \par Inputs:
 * - cmd: The command to be executed including the fully qualified path
 * - argv: The argument vector for the external command.
 * - comm: The p3comm structure in which the data buffer is returned.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int external_cmd(char *cmd, char *argv[], p3comm *comm)
{
	int i, stat = 0, ctop_fd[2], size, lsize = 0;
	unsigned long l;
	char buf[p3BUFSIZE];
	pid_t pid;
	p3stringlist *slist = NULL, *slend = NULL, *string;

	if ((pipe(ctop_fd)) < 0) {
		sprintf(p3buf, "external_cmd: Failed to open pipe: %s\n",
				strerror(errno));
		p3errmsg (p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	}

	if ((pid = fork()) < 0) {
		sprintf(p3buf, "external_cmd: Failed to start process: %s\n",
				strerror(errno));
		p3errmsg (p3MSG_ERR, p3buf);
		stat = -1;
		goto out;

	/* Child process */
	} else if (pid == 0) {
		close(ctop_fd[0]);
		if (ctop_fd[1] != STDOUT_FILENO)
		{
			close(STDOUT_FILENO);
			if ((dup2(ctop_fd[1], STDOUT_FILENO)) != STDOUT_FILENO) {
				sprintf(p3buf, "external_cmd: Failed to redirect output: %s\n",
						strerror(errno));
				p3errmsg (p3MSG_ERR, p3buf);
				stat = -1;
				goto out;
			}
		}
		execv (cmd, argv);
		exit(0);

	/* Parent process */
	} else {
		close(ctop_fd[1]);
		while (1) {
			size = read(ctop_fd[0], buf, p3BUFSIZE);
			if (size > 0) {
				// All data is less than base buffer size
				if (size < p3BUFSIZE && slist == NULL) {
					if ((stat = get_comm_data(comm, size + 1)) < 0)
						goto out;
					memcpy(comm->data, buf, size);
					comm->data[size] = '\0';
					break;
				// Last of multiple buffers
				} else if (size < p3BUFSIZE) {
					if ((stat = get_comm_data(comm, lsize + size + 1)) < 0)
						goto out;
					// Write data to single buffer
					i = 0;
					while (lsize > 0) {
						memcpy(&comm->data[i], slist->data, slist->len);
						i += slist->len;
						lsize -= slist->len;
						string = slist;
						slist = slist->next;
						free(string);
					}
					memcpy(&comm->data[i], buf, size);
					comm->data[(i + size)] = '\0';
					break;
				// One of multiple buffers
				} else {
					if ((string = (p3stringlist *)
							p3calloc(sizeof(p3stringlist) + size)) == NULL) {
						sprintf(p3buf, "external_cmd: Failed to allocate\
 output buffer: %s\n", strerror(errno));
						p3errmsg (p3MSG_CRIT, p3buf);
						stat = -1;
						goto out;
					}
					// Add data to string list
					lsize += size;
					string->len = size;
					l = (unsigned long) string + sizeof(p3stringlist);
					string->data = (char *) l;
					memcpy(string->data, buf, size);
					if (slist == NULL) {
						slist = string;
						slend = string;
					} else {
						slend->next = string;
						slend = string;
					}
				}
			// Data in buffers is multiple of p3BUFSIZE
			} else if (slist != NULL) {
				if ((stat = get_comm_data(comm, lsize + 1)) < 0)
					goto out;
				// Write data to single buffer
				i = 0;
				while (lsize > 0) {
					memcpy(&comm->data[i], slist->data, slist->len);
					comm->data[(i + slist->len)] = '\0';
					i += slist->len;
					lsize -= slist->len;
					string = slist;
					slist = slist->next;
					free(string);
				}
				break;
			// No data read
			} else {
				break;
			}
		}
	}

out:
	return(stat);
} /* end external_cmd */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>external_cmd: Failed to open pipe: <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 external command handler attempts to open a communications pipe
 * to the external command.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>external_cmd: Failed to start process: <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 external command handler attempts to start a new process to
 * execute the external command.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>external_cmd: Failed to redirect output: <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 external command handler attempts to redirect the output from
 * the external command to itself.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * command_shutdown
 *
 * \par Description:
 * Shutdown the command interface cleanly
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int command_shutdown()
{
	int stat = 0;

#ifdef _p3_PRIMARY
	// Attach any new shared memory blocks
	if (attach_shares() == 0) {
		save_anonymous();
	} else {
		p3errmsg (p3MSG_ERR,
				"command_shutdown: Failed to attach shared memory blocks\n");
	}
	// Terminate listener
	if (command->anonpid > 0) {
		kill(command->anonpid, SIGTERM);
		// TODO: Use SIGKILL if shutdown unsuccessful
		command->anonpid = -1;
	}
#endif

	return (stat);
} /* end command_shutdown */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>command_shutdown: Failed to attach shared memory blocks</b>
 * \par Description (ERR):
 * While shutting down, the command handler attempts to attach any new
 * shared memory blocks to save anonymous connection definitions to a
 * temporary file.  If this fails, the remote definitions will have to
 * be reapplied.
 * \par Response:
 * Verify which remote definitions are unavailable and reapply them.
 *
 */
