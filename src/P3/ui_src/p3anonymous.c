/**
 * \file p3anonymous.c
 * <h3>Protected Point to Point anonymous connections program file</h3>
 *
 * Copyright (C) Velocite 2010
 *
 * The P3 anonymous functions manage anonymous connections from
 * P3 secondaries to P3 primaries to automatically supply remote
 * host configuration data.
 */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <p><hr><hr>
 * \section P3_ANON P3 Anonymous Connection Messages
 */

#define _p3_ANONYMOUS_C	1
#include "p3anonymous.h"
#include "p3admin.h"

/** Local host network structure */
p3network *localnet = NULL;

#ifdef _p3_PRIMARY
/** Share memory list anchor */
p3share *anon_share = NULL;
/* End of shared blocks */
void *sharemax;

/** Configuration parser buffer */
char anonbuf[p3CFG_MAXLEN];

/** Remote definition counter */
int remotes = -2;
#endif

/**
 * \par Function:
 * init_anonymous
 *
 * \par Description:
 * Initialize the anonymous connection functions.
 * 
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_anonymous()
{
	int i, stat = 0;
#ifdef _p3_PRIMARY
	int line = 0;
	char filename[p3BUFSIZE];
	FILE *afile = NULL;
	p3remote *remote;
	p3comm adef, *anon_def = &adef;
#endif

	if (command->config == NULL) {
		p3errmsg (p3MSG_ERR, "init_anonymous: No configuration data\n");
		stat = -1;
		goto out;
	}

	i = sizeof(p3network);
	if (localnet == NULL && (localnet = (p3network *) p3calloc(i)) == NULL) {
		sprintf(p3buf, "init_anonymous: Failed to allocate anonymous\
 structures: %s\n", strerror(errno));
		p3errmsg (p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}

#ifdef _p3_PRIMARY
	if (command->config->local == NULL ||
			command->config->local->addr == NULL) {
		p3errmsg (p3MSG_ERR,
			"init_anonymous: Local host address undefined\n");
		stat = -1;
		goto out;
	}
	localnet->addr = command->config->local->addr;
	localnet->port = command->config->local->port;

	// Get saved remote host definitions
	sprintf(filename, "%s/%s", command->config->path, p3ANON_FNAME);
	if ((afile = fopen(filename, "r")) == NULL) {
		if (errno != ENOENT) {
			sprintf(p3buf, "init_anonymous: Failed to open anonymous\
 definitions file %s: %s\n", filename, strerror(errno));
			p3errmsg (p3MSG_ERR, p3buf);
			stat = -1;
			goto out;
		}
	} else {
		// Allocate first shared block
		if (get_anon_share(NULL, 1) < 0) {
			stat = -1;
			goto out;
		}
		// Parse anonymous definitions file
		while (!feof(afile) && !ferror(afile)) {
			line++;
			if (fgets(anonbuf, p3BUFSIZE, afile) == NULL) {
				if (ferror(afile)) {
					sprintf(p3buf, "init_anonymous: Error reading anonymous\
 definitions file at line %d\n", line);
					p3errmsg (p3MSG_ERR, p3buf);
					stat = -1;
					goto out;
				}
				continue;
			}
			// Empty line or comment
			if (anonbuf[0] == '\n' || anonbuf[0] == '#') {
				continue;
			}
			// Initialize token fields to indicate unused
			memset(anon_def, -1, (p3MAX_TOKENS + (2 * sizeof(int))));
			errmsg[0] = '\0';
			if ((stat = parse_def(anonbuf, anon_def)) < 0) {
				goto out;
			} else if (stat > 0) {
				if (strlen(errmsg) > 0) {
					sprintf(p3buf, "init_anonymous: Error parsing anonymous\
 definitions file at line %d: %s\n", line, errmsg);
					p3errmsg (p3MSG_ERR, p3buf);
				}
				continue;
			}
			// Store remote host definitions in shared memory
			remote = command->aremotes;
			while (remote != NULL) {
				// Session already established
				if (remote->ID == anon_def->id1) {
					if (remote->ID < remotes)
						remotes = remote->ID - 1;
					break;
				}
				remote = remote->next;
			}
			// Allocate new remote structure and add to anonymous list
			if (remote == NULL) {
				if ((remote = (p3remote *)
						p3shalloc(sizeof(p3remote))) == NULL) {
					sprintf(p3buf, "init_anonymous: Failed to allocate\
 remote structure: %s\n", strerror(errno));
					p3errmsg (p3MSG_CRIT, p3buf);
					stat = -1;
					goto out;
				}
				remote->ID = remotes--;
			}
			if ((stat = handle_remote_defs(anon_def, remote)) > 0) {
				sprintf(p3buf, "init_anonymous: Error handling \
 anonymous configuration data: %s\n", errmsg);
				p3errlog(p3MSG_ERR, p3buf);
				break;
			} else if (stat < 0) {
				goto out;
			}
		}
		fclose(afile);
	}
#elif _p3_SECONDARY
	// Secondary only supports single primary
	if (command->config->remote == NULL ||
			command->config->remote->addr == NULL) {
		p3errmsg (p3MSG_ERR,
			"init_anonymous: No remote host configuration data\n");
		stat = -1;
		goto out;
	}
	localnet->addr = command->config->remote->addr;
	localnet->port = command->config->remote->port;
	// Connect to the primary
	stat = anon_connect();
#endif

out:
	return(stat);
}

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>init_anonymous: Failed to allocate anonymous structures:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 anonymous connection handler attempts to allocate a network structure
 * and a P3 FIFO structure.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>init_anonymous: No <i>type</i> configuration data</b>
 * \par Description (ERR):
 * The P3 anonymous connection handler needs the specified configuration
 * data to handle anonymous connections.
 * \par Response:
 * Correct the configuration file.
 *
 * <hr><b>init_anonymous: Failed to open anonymous definitions file:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 configuration handler was unable to open the anonymous definitions
 * file.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>init_anonymous: Error reading anonymous definitions file at
 * line <i>line</i></b>
 * \par Description (ERR):
 * The P3 configuration handler was unable to read the anonymous definitions
 * file following the given line number.
 * \par Response:
 * Correct the configuration file.
 *
 * <hr><b>init_anonymous: Error at line # in anonymous definitions file:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * A syntax error was found in the anonymous definitions file.
 * \par Response:
 * Correct the configuration error.
 *
 * <hr><b>init_anonymous: Failed to allocate remote structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 anonymous connection handler attempts to allocate a remote structure
 * for the each remote host defined in the anonymous definitions file.  If
 * this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>init_anonymous: Attempt to start anonymous connection monitoring
 * failed: <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 anonymous connection handler attempts to start a standalone process
 * to handle anonymous connections.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

#ifdef _p3_PRIMARY
/**
 * \par Function:
 * anon_handler
 *
 * \par Description:
 * Listen for and process anonymous connections and
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

int anon_handler()
{
	int iend, ibgn, stat = 0, numfds;
	fd_set fdset;
	struct timeval tv;
	p3remote *remote;
	p3subnet *snloc, *snshr, *snlist = NULL;
	p3comm adef, *anon_def = &adef;
	struct sigaction p3sigact;

	// Allocate first shared block
	if (anon_share == NULL && get_anon_share(NULL, 1) < 0) {
		stat = -1;
		goto out;
	}
	// Spawn a child process to listen for secondary connections
	if ((command->anonpid = fork()) < 0) {
		sprintf(p3buf, "init_anonymous: Attempt to start\
anonymous connection monitoring failed: %s\n", strerror(errno));
		p3errmsg(p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	// Network listener
	} else if (command->anonpid == 0) {
		// Set interrupt handler
		p3signal(SIGQUIT, anon_shutdown, p3sigact, stat);
		p3signal(SIGTERM, anon_shutdown, p3sigact, stat);
		p3signal(SIGPIPE, anon_shutdown, p3sigact, stat);
		if (stat < 0) {
			sprintf(p3buf, "anon_handler: Error setting interrupt\
 handler: %s\n", strerror(errno));
			p3errlog(p3MSG_ERR, p3buf);
			stat = -1;
			goto out;
		}

		// Start listening for secondary configurations
		command->config->flag |= p3CFG_REMOTE;
		if (p3listen(p3START, localnet) < 0) {
			NET_LOGMSG();
			anon_shutdown(-1);
		}

		// Loop forever listening for connections
		anon_def->datlen = 0;
		anon_def->data = NULL;
		while (1) {
			FD_ZERO(&fdset);
			numfds = localnet->sd + 1;
			FD_SET(localnet->sd, &fdset);
			remote = command->aremotes;
			while (remote != NULL) {
				if (remote->net != NULL && remote->net->flag & NET_ST_UP) {
					if (numfds <= remote->net->sd)
						numfds = remote->net->sd + 1;
					FD_SET(remote->net->sd, &fdset);
				}
				remote = remote->next;
			}
			// Wake up at least once every 10 seconds(?)
			tv.tv_sec = 0;
			tv.tv_usec = p3MILLISEC(10000);
			if ((stat = select(numfds, &fdset, 0, 0, &tv)) < 0) {
				if (errno == EINTR)
					continue;
				sprintf(p3buf, "anon_handler: Error in anonymous\
 connection monitor: %s\n", strerror(errno));
				p3errlog(p3MSG_ERR, p3buf);
				anon_shutdown(-1);
			}

			// Handle session request
			if (localnet->sd > 0 && FD_ISSET(localnet->sd, &fdset)) {
				p3listener(localnet->sd, localnet);
				NET_LOGMSG();
			}

			// Receive secondary configuration information
			remote = command->aremotes;
			while (remote != NULL) {
				if (remote->net != NULL && remote->net->sd > 0 &&
						FD_ISSET(remote->net->sd, &fdset)) {
					// Deactivate remote definition
sprintf(p3buf, "Inbound data\n");
p3errlog(p3MSG_DEBUG, p3buf);
					if (net_receive(remote->net) < 0) {
						NET_LOGMSG();
						FREE_p3NETWORK(remote->net);
						remote->net = NULL;
					// Parse remote definition
					} else {
						if (remote->net->len == 0)
							break;
sprintf(p3buf, "Parse remote data (%d): %s\n", remote->net->len, remote->net->data);
p3errlog(p3MSG_DEBUG, p3buf);
						iend = 0;
						while (iend < remote->net->len) {
							ibgn = iend;
							while (iend < remote->net->len &&
									remote->net->data[iend] != '\n')
								iend++;
							while (iend < remote->net->len &&
									(remote->net->data[iend] == '\n' ||
									remote->net->data[iend] == '\0')) {
								remote->net->data[iend] = '\0';
								iend++;
							}
							// Use configuration parser
							if ((stat = parse_def((char *)
									&remote->net->data[ibgn], anon_def)) > 0) {
								sprintf(p3buf, "anon_handler: Error parsing \
 anonymous configuration data: %s\n", errmsg);
								p3errlog(p3MSG_ERR, p3buf);
							} else if (stat < 0) {
								anon_shutdown(-1);
							} else if ((stat =
									handle_remote_defs(anon_def, remote)) > 0) {
								sprintf(p3buf, "anon_handler: Error handling \
 anonymous configuration data: %s\n", errmsg);
								p3errlog(p3MSG_ERR, p3buf);
								break;
							} else if (stat < 0) {
								anon_shutdown(-1);
							}
						}
						// Move subnets from local memory to shared
						snloc = remote->subnets;
						while(snloc != NULL) {
							// Subnet already in shared memory
							if ((void *) anon_share->next < (void *) snloc
									&& (void *) snloc < sharemax) {
								// Move subnet to temporary list
								remote->subnets = snloc->next;
								snloc->next = snlist;
								snlist = snloc;
								snloc = remote->subnets;
							} else {
								if ((snshr = (p3subnet *)
										p3shalloc(sizeof(p3subnet))) == NULL) {
									sprintf(p3buf, "anon_handler: Error allocating\
 subnet in shared memory: %s\n", strerror(errno));
									p3errlog(p3MSG_ERR, p3buf);
									anon_shutdown(-1);
								}
								// Put new subnet on temporary list
								memcpy(snshr, snloc, sizeof(p3subnet));
								snshr->next = snlist;
								snlist = snshr;
								remote->subnets = snloc->next;
								free(snloc);
								snloc = remote->subnets;
							}
						}
						// Set new remote subnet
						remote->subnets = snlist;
					}
				}
				remote = remote->next;
			}
		}
	} else {
		p3errmsg(p3MSG_NOTICE, "Starting anonymous connection monitoring\n");
	}

out:
	return(stat);
} /* end anon_handler */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>anon_handler:  Error setting interrupt handler:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 anonymous connection monitor attempts to set an interrupt handler
 * to shutdown cleanly.  If there is an error, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>anon_handler:  Error in anonymous connection monitor:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 anonymous connection monitor listens for secondary requesting
 * a connection.  If there is an error, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * p3validate
 *
 * \par Description:
 * Validate connection requests from P3 secondary hosts.  When this
 * function is called by the network listener, the source address is
 * checked for the following:
 * - Is there already a session with the secondary host?
 * - Is the secondary host address from an allowed subnet?
 * If the validation is successful, the remote list of anonymous remote
 * definitions is checked.  If there is already a remote structure, it
 * is used.  Otherwise, a new one is created and added to the list.
 *
 * \par Inputs:
 * - net: The connecting secondary network information
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = Session accepted
 *   - <0 = Session rejected
 *   - >0 = Session already established
 */

int p3validate(p3network *net)
{
	int i, stat = 0;
	unsigned char *address, *subnet;
	p3remote *remote;
	union {
		struct in_addr	v4;
		struct in6_addr	v6;
	} addr;

	// Allow only specified subnet access
	if (command->flag & p3CMD_ANONNET) {
		if (command->flag & p3HST_IPV4) {
			inet_pton(AF_INET, net->addr, &addr.v4);
			address = (unsigned char *) &addr.v4;
			subnet = (unsigned char *) &command->mask.v4;
			for (i=0; i < sizeof(struct in_addr); i++)
				address[i] &= subnet[i];
			subnet = (unsigned char *) &command->subnet.v4;
			for (i=0; i < sizeof(struct in_addr); i++) {
				if (address[i] != subnet[i]) {
					stat = -1;
					goto out;
				}
			}
		} else if (command->flag & p3HST_IPV6) {
			inet_pton(AF_INET6, net->addr, &addr.v6);
			address = (unsigned char *) &addr.v6;
			subnet = (unsigned char *) &command->mask.v6;
			for (i=0; i < sizeof(struct in6_addr); i++)
				address[i] &= subnet[i];
			subnet = (unsigned char *) &command->subnet.v6;
			for (i=0; i < sizeof(struct in6_addr); i++) {
				if (address[i] != subnet[i]) {
					stat = -1;
					goto out;
				}
			}
		}
	}

	remote = command->aremotes;
	while (remote != NULL) {
		// Session already established
		if (strcmp(net->addr, remote->addr) == 0) {
			if (remote->net->flag & NET_ST_UP) {
				// Existing session failed
				if (net_status(net) < 0) {
					sprintf(p3buf, "p3validate: Session replaced by\
 new session with %s\n", net->addr);
					p3errlog(p3MSG_WARN, p3buf);
				// Reject new session
				} else {
					stat = 1;
					goto out;
				}
			}
			// Replace old session with new
			free(remote->net);
			remote->net = net;
			net->addr = remote->addr;
			break;
		} else {
			remote = remote->next;
		}
	}

	// Allocate new remote structure and add to anonymous list
	if (remote == NULL) {
		if ((remote = (p3remote *) p3shalloc(sizeof(p3remote))) == NULL) {
			sprintf(p3buf, "p3validate: Failed to allocate remote structure:\
 %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
		remote->addr = net->addr;
		remote->next = command->aremotes;
		remote->net = net;
		command->aremotes = remote;
	}
sprintf(p3buf, "Remote validated %x(%x)\n", command, remote);
p3errlog(p3MSG_DEBUG, p3buf);

out:
	return(stat);
} /* end p3validate */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>p3validate:  Session replaced by new session with <i>host address</i></b>
 * \par Description (WARN):
 * The anonymous connection validation process has found an existing session
 * that has stopped but was not closed cleanly.
 * \par Response:
 * This warning notifies the administrator to verify that the session information
 * for the secondary is valid when accepting anonymous connections.
 *
 * <hr><b>p3validate: Failed to allocate remote structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The anonymous connection handler attempts to allocate a remote structure
 * for a new remote host.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * get_anon_share
 *
 * \par Description:
 * Get a new or attach an existing shared memory block.
 *
 * \par Inputs:
 * - share: The p3share structure to be processed.  If requesting a
 *   new shared block, this may be NULL.  Otherwise, it must point to
 *   an existing block that has not previously been attached.
 * - new: If 1, create a new block. If 0, just attach an existing block.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int get_anon_share(p3share *share, int new)
{
	int stat = 0;
	unsigned long l;
	void *sblock, *sloc;

printf("DEBUG: Get shared block: %x, %d\n", share, new);
	if (new) {
		// Register new shared block
		if (anon_share == NULL) {
			if ((share = (p3share *) p3calloc(sizeof(p3share))) == NULL) {
				sprintf(p3buf, "get_anon_share: Failed to allocate shared memory\
 anchor structure: %s\n", strerror(errno));
				p3errmsg (p3MSG_CRIT, p3buf);
				stat = -1;
				goto out;
			}
			sloc = (void *) p3SHARE_START;
		} else {
			share = anon_share->tail;
			l = (unsigned long) share;
			l += p3SHARE_SIZE;
			sloc = (p3share *) l;
		}

printf("DEBUG: Shmget\n");
		if ((share->shmid =
				shmget(0, p3SHARE_SIZE, IPC_CREAT+0660)) < 0) {
			sprintf(p3buf, "get_anon_share: Error getting new shared\
 memory block: %s\n", strerror(errno));
			p3errlog(p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
	} else
		sloc = share->next;

	// Attach shared block
printf("DEBUG: Shmat\n");
	if ((sblock = (void *)
			shmat(share->shmid, sloc, SHM_RND)) == (void *) -1) {
		sprintf(p3buf, "get_anon_share: Error attaching shared\
 memory block: %s\n", strerror(errno));
		p3errlog(p3MSG_CRIT, p3buf);
		stat = -1;
		goto out;
	}

	if (new) {
		// Free shared block when last process detaches it
printf("DEBUG: Shmctl\n");
		if (shmctl(share->shmid, IPC_RMID, NULL) < 0) {
			sprintf(p3buf, "get_anon_share: Failed to set shared block to\
 automatically release: %s\n", strerror(errno));
			p3errlog(p3MSG_CRIT, p3buf);
			stat = -1;
			if (shmdt(sblock) < 0) {
				sprintf(p3buf, "get_anon_share: Failed to detach shared block: %s\n",
					strerror(errno));
				p3errlog(p3MSG_ERR, p3buf);
			}
			goto out;
		}
		// Add new shared block to list
printf("DEBUG: Add block\n");
		memset(sblock, 0, sizeof(p3share));
		if (anon_share == NULL) {
			share->next = (p3share *) sblock;
			share->tail = (p3share *) sblock;
			share->next->used = sizeof(p3share);
			anon_share = share;
			sharemax = (void *) sblock + p3SHARE_SIZE;
		} else {
			share->tail = (p3share *) sblock;
			share->tail->used = sizeof(p3share);
			anon_share->tail = share;
			share->next = (p3share *) sblock;
			sharemax += p3SHARE_SIZE;
		}
	}

	// Indicate shared block has been processed
printf("DEBUG: Shm success\n");
	if (command->flag & p3CMD_ADMIN)
		share->libflag |= p3SHR_NEXTAT;
	else
		share->extflag |= p3SHR_NEXTAT;

out:
printf("DEBUG: Exit get share %d\n", stat);
	return(stat);
} /* end get_anon_share */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>get_anon_share:  Error getting new shared memory block:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 anonymous connection monitor uses shared memory to save
 * secondary host configuration information.  If there is an error
 * getting a new shared block, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>get_anon_share:  Error attaching new shared memory block:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 anonymous connection monitor uses shared memory to save
 * secondary host configuration information.  If there is an error
 * attaching a shared block, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>get_anon_share:  Failed to set shared block to automatically release:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 anonymous connection monitor uses shared memory to save
 * secondary host configuration information.  If there is an error
 * setting a shared block to automatically release, there is a system
 * wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>get_anon_share:  Failed to detach shared block:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 anonymous connection monitor uses shared memory to save
 * secondary host configuration information.  If there is an error
 * detaching a shared block, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 **/

/**
 * \par Function:
 * p3shalloc
 *
 * \par Description:
 * Get a buffer from shared memory and initialize it to all zeros.
 *
 * <i> Note that each buffer is taken from the end of the shared memory
 * list, and buffers are never freed.  Thus, it is assumed that as an
 * administration application, this will not run continuously.</i>
 *
 * \par Inputs:
 * - size: The buffer size to be allocated.
 *
 * \par Outputs:
 * - void *: The location of the buffer, or NULL if there is an error.
 */

void *p3shalloc(int size)
{
	unsigned long l;
	void *newbuf = NULL;
printf("DEBUG: shalloc %d\n", size);

	// Check for space in current shared block
	if (anon_share == NULL && get_anon_share(NULL, 1) < 0)
		goto out;
printf("DEBUG: \n");
	if (size > (p3SHARE_SIZE - anon_share->tail->used)
			&& get_anon_share(NULL, 1) < 0)
		goto out;

printf("DEBUG: Set buffer\n");
	// Get buffer and update shared block information
	l = (unsigned long) anon_share->tail;
	l += anon_share->tail->used;
	newbuf = (void *) l;
printf("DEBUG: Clear newbuf %x\n", newbuf);
	memset(newbuf, 0, size);
	anon_share->tail->used += size;

out:
printf("DEBUG: Exit shalloc: %x\n", newbuf);
	return(newbuf);
}

/**
 * \par Function:
 * attach_shares
 *
 * \par Description:
 * Attach all shared blocks not already attached.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int attach_shares()
{
	int stat = 0;
	p3share *share;

	if (anon_share == NULL)
		goto out;

	share = anon_share;
	while (share->next != NULL) {
		// Skip already attached shared blocks
		if ((command->flag & p3CMD_ADMIN) && (share->libflag & p3SHR_NEXTAT)) {
			share = share->next;
			continue;
		} else if (!(command->flag & p3CMD_ADMIN) && (share->extflag & p3SHR_NEXTAT)) {
			share = share->next;
			continue;
		}
		// Attach shared block
printf("DEBUG: Attach share\n");
		if (shmat(share->shmid, share->next, SHM_RND) == (void *) -1) {
			sprintf(p3buf, "attach_shares: Error attaching shared\
 memory block: %s\n", strerror(errno));
			p3errlog(p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
		share = share->next;
	}

out:
	return(stat);
} /* end attach_shares */

/**
 * \par Function:
 * save_anonymous
 *
 * \par Description:
 * Save anonymous definitions that have not been accepted
 * in a temporary file.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int save_anonymous()
{
	int stat = 0;

	return(stat);
} /* end save_anonymous */

/**
 * \par Function:
 * anon_shutdown
 *
 * \par Description:
 * Shutdown the anonymous connection handler cleanly.
 *
 * \par Inputs:
 * - status:  The status code for shutting down
 *
 * \par Outputs:
 * - None
 */

void anon_shutdown(int status)
{

	while (command->aremotes != NULL) {
		p3session(p3STOP, command->aremotes->net);
		command->aremotes = command->aremotes->next;
	}

	p3listen(p3STOP, localnet);
	free(localnet);
	localnet = NULL;

	command->anonpid = -1;
	command->anonsd = -1;

	exit(status);
} /* end anon_shutdown */
#endif

#ifdef _p3_SECONDARY
/**
 * \par Function:
 * anon_connect
 *
 * \par Description:
 * Attempt to connect to a P3 primary host and send the
 * secondary's configuration information.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int anon_connect()
{
	int i, stat = 0, len, ipver;
	p3subnet *subnet;
	char sbuf[p3CFG_MAXLEN << 1];

	// Connect to primary
	if (p3session(p3START, localnet) < 0) {
		NET_PRTMSG();
		stat = -1;
		goto out;
	}

	// Send configuration information
	// remote/#/hostname string
	sprintf(&sbuf[2], "%s/0/%s=%s\n", p3L1REMOTE_TOK, p3L2HOSTNAME_TOK,
			command->config->local->hostname);
	len = strlen(&sbuf[2]);
	sbuf[0] = (unsigned char) (len >> 8) & 0xff;
	sbuf[1] = (unsigned char) len & 0xff;
	NET_SETDATA(localnet, sbuf, len + 2);
sprintf(p3buf, "Send data(%d): %s\n", len, &localnet->data[2]);
p3errmsg (p3MSG_DEBUG, p3buf);
	if (net_transmit(localnet) < 0) {
		NET_PRTMSG();
		stat = -1;
		goto out;
	}
	subnet = command->config->local->subnets;
	i = 0;
	while (subnet != NULL) {
		if (subnet->flag & p3HST_IPV6)
			ipver = 6;
		else
			ipver = 4;
		// remote/#/subnet/#/ipver number
		// remote/#/subnet/#/address ip_addr
sprintf(p3buf, "Subnet(%d): %x, Adr %x\n", subnet->ID, subnet, subnet->addr);
p3errmsg (p3MSG_DEBUG, p3buf);
		sprintf(&sbuf[2], "%s/0/%s/%d/%s=%d\n%s/0/%s/%d/%s=%s\n",
			p3L1REMOTE_TOK, p3L2SUBNET_TOK, subnet->ID, p3L3IPVER_TOK, ipver,
			p3L1REMOTE_TOK, p3L2SUBNET_TOK, subnet->ID, p3L3ADDRESS_TOK, subnet->addr);
		i++;
		len = strlen(&sbuf[2]) + 1;
		sbuf[0] = (len >> 8) & 0xff;
		sbuf[1] = len & 0xff;
		NET_SETDATA(localnet, sbuf, len + 2);
sprintf(p3buf, "Send data(%d): %s\n", len, &localnet->data[2]);
p3errmsg (p3MSG_DEBUG, p3buf);
		if (net_transmit(localnet) < 0) {
			NET_PRTMSG();
			stat = -1;
			goto out;
		}
		subnet = subnet->next;
	}

	// Disconnect from primary
	if (p3session(p3STOP, localnet) < 0) {
		NET_PRTMSG();
		stat = -1;
		goto out;
	}

out:
	return(stat);
} /* end anon_connect */
#endif
