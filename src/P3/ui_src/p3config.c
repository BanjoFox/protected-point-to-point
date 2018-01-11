/**
 * \file p3config.c
 * <h3>Protected Point to Point config file</h3>
 * 
 * Copyright (C) Velocite 2010
 *
 * Overview of the purpose of the functions in the source file.
 */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <p><hr><hr>
 * \section P3_CONFIG P3 Configuration Handler Messages
 */

#define _p3_CONFIG_C
#include "p3config.h"
#include "p3admin.h"

/** Local copy of configuration data */
p3config *config;

/** Configuration parser buffer */
char cfgbuf[p3CFG_MAXLEN];

#ifdef _p3_PRIMARY
/** Group array list */
p3group **grouplist = NULL;
#endif

/**
 * \par Function:
 * init_config
 *
 * \par Description:
 * The configuration initialization opens the configuration file and
 * allocates the configuration structures.
 * 
 * \par Inputs:
 * - admin_cfg:  Pointer to the adminstration configuration structure.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int init_config(p3config *admin_cfg)
{
	int stat = 0;

	config = admin_cfg;

	// Get the configuration directory
	if (config->path == NULL) {
		// If not set on the command line, use the default
		if ((config->path = (char *) p3malloc(sizeof(p3CFG_PATH))) == NULL) {
			sprintf(p3buf, "init_config: Failed to allocate configuration directory\
 name: %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
		strcpy(config->path, p3CFG_PATH);
	}

	// Get the primary configuration filename
	if ((config->flag & p3CFG_PRI) && config->pricfg == NULL) {
		// If not set on the command line, use the default
		if ((config->pricfg = (char *) p3malloc(sizeof(p3CFG_PRIFILE))) == NULL) {
			sprintf(p3buf, "init_config: Failed to allocate primary configuration\
 filename: %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
		strcpy(config->pricfg, p3CFG_PRIFILE);
	}

	// Get the secondary configuration filename
	if ((config->flag & p3CFG_SEC) && config->seccfg == NULL) {
		// If not set on the command line, use the default
		if ((config->seccfg = (char *) p3malloc(sizeof(p3CFG_SECFILE))) == NULL) {
			sprintf(p3buf, "init_config: Failed to allocate secondary configuration\
 filename: %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
		strcpy(config->seccfg, p3CFG_SECFILE);
	}

out:
	return(stat);
} /* end init_config */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>init_config: Failed to allocate main <i>location</i> name:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 configuration handler attempts to allocate path and file names
 * for configuration data.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * get_config
 *
 * \par Description:
 * Open the configuration file defined in the p3config structure
 * and call the parser to read the definitions defined by the flags
 * set in the p3config structure.  <i>Note that either or both or the
 * primary and secondary configuration files are read, depending
 * on the compile time constants for building primary, secondary,
 * or primaryplus.</i>
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int get_config()
{
	int line = 0, stat = 0;
	char filename[p3BUFSIZE];
	FILE *cfile = NULL;
	p3comm cdef, *cfg_def = &cdef;
#ifdef _p3_PRIMARY
	p3group *group;
#endif
	p3remote *remote;
printf("DEBUG: Read configuration definitions\n");

	// Data is managed by definition parser
	cfg_def->datlen = 0;
	cfg_def->data = NULL;

#ifdef _p3_PRIMARY
	// Read and parse primary configuration file
	sprintf(filename, "%s%s", config->path, config->pricfg);
	if ((cfile = fopen(filename, "r")) == NULL) {
		sprintf(p3buf, "get_config: Failed to open primary configuration\
 file %s: %s\n", filename, strerror(errno));
		p3errmsg (p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	}
	// Parse configuration file
	while (!feof(cfile) && !ferror(cfile)) {
		line++;
		if (fgets(cfgbuf, p3BUFSIZE, cfile) == NULL) {
			if (ferror(cfile)) {
				sprintf(p3buf, "get_config: Error reading primary configuration\
 file at line %d\n", line);
				p3errmsg (p3MSG_ERR, p3buf);
				stat = -1;
				goto out;
			}
			continue;
		}
		// Empty line or comment
		if (cfgbuf[0] == '\n' || cfgbuf[0] == '#') {
			continue;
		}
		// Initialize token fields to indicate unused
		memset(cfg_def, -1, (p3MAX_TOKENS + (2 * sizeof(int))));
		errmsg[0] = '\0';
		if ((stat = parse_def(cfgbuf, cfg_def)) < 0) {
			goto out;
		} else if (stat > 0) {
			if (strlen(errmsg) > 0) {
				sprintf(p3buf, "get_config: Error parsing primary configuration\
 file at line %d: %s\n", line, errmsg);
				p3errmsg (p3MSG_ERR, p3buf);
			}
			continue;
		}
		if ((stat = handle_def(cfg_def)) > 0) {
			sprintf(p3buf, "get_config: Error at line %d in primary\
 configuration file: %s\n", line, errmsg);
			p3errmsg (p3MSG_ERR, p3buf);
			stat = 0;
		} else if (stat < 0) {
			goto out;
		}
	}
	fclose(cfile);
	cfile = NULL;
#endif

#ifdef _p3_SECONDARY
	// Read and parse secondary configuration file
	sprintf(filename, "%s%s", config->path, config->seccfg);
	if ((cfile = fopen(filename, "r")) == NULL) {
		sprintf(p3buf, "get_config: Failed to open secondary configuration\
 file %s: %s\n", filename, strerror(errno));
		p3errmsg (p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	}
	// Parse configuration file
	while (!feof(cfile) && !ferror(cfile)) {
		line++;
		if (fgets(cfgbuf, p3BUFSIZE, cfile) == NULL) {
			if (ferror(cfile)) {
				sprintf(p3buf, "get_config: Error reading secondary configuration\
 file at line %d\n", line);
				p3errmsg (p3MSG_ERR, p3buf);
				stat = -1;
				goto out;
			}
			continue;
		}
		// Empty line or comment
		if (cfgbuf[0] == '\n' || cfgbuf[0] == '#') {
			continue;
		}
		// Initialize token fields to indicate unused
		memset(cfg_def, -1, (p3MAX_TOKENS + (2 * sizeof(int))));
		errmsg[0] = '\0';
		if ((stat = parse_def(cfgbuf, cfg_def)) < 0) {
			goto out;
		} else if (stat > 0) {
			if (strlen(errmsg) > 0) {
				sprintf(p3buf, "get_config: Error at line %d in secondary configuration\
 file: %s\n", line, errmsg);
				p3errmsg (p3MSG_ERR, p3buf);
			}
			continue;
		}
		if ((stat = handle_def(cfg_def)) > 0) {
			sprintf(p3buf, "get_config: Error at line %d in secondary\
 configuration file: %s\n", line, errmsg);
			p3errmsg (p3MSG_ERR, p3buf);
			stat = 0;
		} else if (stat < 0) {
			goto out;
		}
	}
#endif

	// Validate all required parameters entered
printf("DEBUG: Validate configuration definitions\n");
	if (validate_local(config->local) < 0) {
		sprintf(p3buf, "get_config: Local definition errors:%s\n", errmsg);
		p3errmsg (p3MSG_ERR, p3buf);
	}
#ifdef _p3_PRIMARY
	group = config->group;
	while (group != NULL) {
		if (validate_group(group) < 0) {
			sprintf(p3buf, "get_config: Group %d definition errors:%s\n",
				group->ID, errmsg);
			p3errmsg (p3MSG_ERR, p3buf);
		}
		group = group->next;
	}
#endif
	remote = config->remote;
	while (remote != NULL) {
		if (validate_remote(remote) < 0) {
			sprintf(p3buf, "get_config: Remote %d definition errors:%s\n",
				remote->ID, errmsg);
			p3errmsg (p3MSG_ERR, p3buf);
		}
		remote = remote->next;
	}

out:
	if (cfile != NULL)
		fclose(cfile);
	if (cfg_def->data != NULL)
		free(cfg_def->data);

	return(stat);
} /* end get_config */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>get_config: Failed to open <i>p3_system</i> configuration file:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 configuration handler was unable to open the specified configuration
 * file.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>get_config: Error reading <i>p3_system</i> configuration file at
 * line <i>line</i></b>
 * \par Description (ERR):
 * The P3 configuration handler was unable to read the specified configuration
 * file following the given line number.
 * \par Response:
 * Correct the configuration file.
 *
 * <hr><b>get_config: Error at line # in <i>p3_system</i> configuration file:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * A syntax error was found in the specified configuration file.
 * \par Response:
 * Correct the configuration error.
 *
 */

/**
 * \par Function:
 * parse_def
 *
 * \par Description:
 * Get the configuration definitions defined by the flags
 * set in the p3config structure.  The configuration input
 * is parsed to break out individual tokens and construct
 * a p3comm structure.  This is then used to handle the data.
 *
 * \par Inputs:
 * - cfg_input: The configuration line to be parsed.
 * - cfg_def: The structure for storing the results of the
 *   definition parsing.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - >0 = Top level token type not selected in flags
 *   - <0 = Error
 */

int parse_def(char *cfg_input, p3comm *cfg_def)
{
	int i, idx, size, stat = 0;
	char *value = NULL, *token = cfg_input;
#ifdef _p3_PRIMARY
	p3group *group;
#endif

	for (i=0; i < p3MAX_TOKENS; i++)
		cfg_def->tokens[i] = -1;
	cfg_def->id1 = -1;
	cfg_def->id2 = -1;

	if (cfg_input[0] == '/')
		token++;
	// Break out tokens and find parameter value
	size = strlen(cfg_input);
	value = NULL;
	i = 0;
	for (idx=0; idx < size; idx++) {
		if (cfg_input[idx] == '/' || cfg_input[idx] == '\n' || cfg_input[idx] == ' ') {
			if (!(cfg_input[idx] == '/' && i))
				cfg_input[idx] = '\0';
		} else if (cfg_input[idx] == '=') {
			i = 1;
			cfg_input[idx] = '\0';
			// Set definition value to empty
			if (cfg_def->data != NULL)
				cfg_def->data[0] = '\0';
		} else if (i > 0) {
			i = -1;
			// Reuse the value buffer if possible
			value = &cfg_input[idx];
			if ((stat = get_comm_data(cfg_def, strlen(value) + 1)) < 0)
				goto out;
		}
	}
	if (value != NULL)
		strcpy(cfg_def->data, value);

	// Get level 1 index
	cfg_def->tokens[0] = get_level_idx(level1, p3L1MAX, token);
	if (cfg_def->tokens[0] < 0) {
		sprintf(errmsg, "No token found at top level");
		stat = 1;
		goto out;
	}
	i = 0;
	if ((config->flag & p3CFG_LOCAL) && cfg_def->tokens[0] == p3L1LOCAL) {
		i = 1;
#ifdef _p3_PRIMARY
	} else if (cfg_def->tokens[0] == p3L1GROUP) {
		next_token(token, &cfg_input[size]);
		idx = strtol(token, NULL, 10);
		if (idx > 254) {
			sprintf(errmsg, "Maximum number of groups exceeded");
			stat = 1;
			goto out;
		}
		else if (idx > config->groups)
			config->groups = idx + 1;
		if (config->flag & p3CFG_GROUP) {
			i = 1;
			cfg_def->id1 = idx;
		}
#endif
	} else if (cfg_def->tokens[0] == p3L1REMOTE) {
#ifdef _p3_PRIMARY
		// Build group array list
		if (grouplist == NULL) {
			if ((grouplist = (p3group **)
					p3calloc(sizeof(p3group *) * config->groups)) == NULL) {
				sprintf(p3buf, "parse_def: Failed to allocate group list\
 array: %s\n", strerror(errno));
				p3errmsg (p3MSG_CRIT, p3buf);
				stat = -1;
				goto out;
			}
			group = config->group;
			while (group != NULL) {
				grouplist[group->ID] = group;
				group = group->next;
			}
		}
		next_token(token, &cfg_input[size]);
		// Get an estimate of defined remotes
		idx = strtol(token, NULL, 10);
		if ((idx + 1) > config->remotes)
			config->remotes = idx + 1;
#else
		config->remotes = 1;
#endif
		if (config->flag & p3CFG_REMOTE) {
			i = 1;
#ifdef _p3_PRIMARY
			cfg_def->id1 = idx;
#endif
		}
	}
	// Only handle the requested configuration types
	if (!i) {
		stat = 1;
		goto out;
	}
	next_token(token, &cfg_input[size]);

	// Get level 2 index
	cfg_def->tokens[1] = get_level_idx(level2, p3L2MAX, token);
	if (cfg_def->tokens[1] < 0) {
		sprintf(errmsg, "No token found at second level");
		stat = 1;
		goto out;
	}
	// Set subnet ID
	if (cfg_def->tokens[1] == p3L2SUBNET) {
		next_token(token, &cfg_input[size]);
		idx = strtol(token, NULL, 10);
		if (cfg_def->id1 < 0)
			cfg_def->id1 = idx;
		else
			cfg_def->id2 = idx;
	}

	// Get level 3 index
	next_token(token, &cfg_input[size]);
	cfg_def->tokens[2] = get_level_idx(level3, p3L3MAX, token);
	if (cfg_def->tokens[2] < 0) {
		goto out;
	}

	// Get level 4 index
	next_token(token, &cfg_input[size]);
	cfg_def->tokens[3] = get_level_idx(level4, p3L4MAX, token);
	if (cfg_def->tokens[3] < 0) {
		goto out;
	}


out:
	return(stat);
} /* end parse_def */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>parse_def: Failed to allocate value buffer: <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 configuration handler attempts to allocate a buffer to hold the
 * value of the definition.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>parse_def: Failed to allocate group list array: <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 configuration handler attempts to allocate an array to manage
 * group data.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * handle_def
 *
 * \par Description:
 * If needed, allocate a new configuration structure for the
 * definition.  Then determine the appropriate definition handler
 * and pass the parsed values to it.
 *
 * \par Inputs:
 * - cfg_def: The parsed definition values.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - >0 = Parser error
 *   - <0 = System error
 */

int handle_def(p3comm *cfg_def)
{
	int stat = 0;
#ifdef _p3_PRIMARY
	int i;
	p3group *group;
#endif
	p3remote *remote;

	if (cfg_def->tokens[0] == p3L1LOCAL) {
		if (config->local == NULL) {
			p3errmsg (p3MSG_ERR,
				"handle_def: Local structure not initialized\n");
			stat = -1;
			goto out;
		}
		stat = handle_local_defs(cfg_def);
#ifdef _p3_PRIMARY
	} else if (cfg_def->tokens[0] == p3L1GROUP) {
		group = config->group;
		while (group != NULL) {
			if (group->ID == cfg_def->id1)
				break;
			group = group->next;
		}
		if (group == NULL) {
			if ((group = (p3group *) p3calloc(sizeof(p3group))) == NULL) {
				sprintf(p3buf, "handle_def: Failed to allocate group structure: %s\n",
						strerror(errno));
				p3errmsg (p3MSG_CRIT, p3buf);
				stat = -1;
				goto out;
			}
			// Initialize and add group to end of list
			group->ID = cfg_def->id1;
			if (config->group == NULL) {
				config->group = group;
				config->grptail = group;
			} else {
				config->grptail->next = group;
				config->grptail = group;
			}
		}
		stat = handle_group_defs(cfg_def, group);
#endif
	} else if (cfg_def->tokens[0] == p3L1REMOTE) {
		remote = config->remote;
#ifdef _p3_PRIMARY
		while (remote != NULL) {
			if (remote->ID == cfg_def->id1)
				break;
			remote = remote->next;
		}
#endif
		if (remote == NULL) {
			if ((remote = (p3remote *) p3calloc(sizeof(p3remote))) == NULL) {
				sprintf(p3buf, "handle_def: Failed to allocate remote structure: %s\n",
						strerror(errno));
				p3errmsg (p3MSG_CRIT, p3buf);
				stat = -1;
				goto out;
			}
#ifdef _p3_PRIMARY
			remote->ID = cfg_def->id1;
			for (i=0; i < p3RMT_GROUPS; i++)
				remote->groups[i] = 255;
#endif
#ifdef _p3_SECONDARY
			remote->port = p3ADM_PORT;
#endif
			// Add remote to end of list
			if (config->remote == NULL) {
				config->remote = remote;
				config->rmttail = remote;
			} else {
				config->rmttail->next = remote;
				config->rmttail = remote;
			}
		}
		stat = handle_remote_defs(cfg_def, remote);
	}

out:
	return(stat);
} /* end handle_def */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>handle_def: Local structure: <i>error reason</i></b>
 * \par Description (ERR):
 * The local host structure to manage local host data should be allocated
 * and initialized when the administration application is started.  If
 * this is not the case, then there is an application error.
 * \par Response:
 * Notify the Velocite Systems P3 support team.
 *
 * <hr><b>handle_def: Failed to allocate <i>configuration type</i> structure:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 configuration handler attempts to allocate the structure to manage
 * configuration data.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */


/**
 * \par Function:
 * handle_local_defs
 *
 * \par Description:
 * Handle the local configuration definitions by setting the
 * appropriate local structure values, based on the parsed data.
 *
 * \par Inputs:
 * - cfg_def: The parsed definition values.
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - >0 = Parser error
 *   - <0 = System error
 */

int handle_local_defs(p3comm *cfg_def)
{
	int stat = 0;
#ifdef _p3_PRIMARY
	int i;
	unsigned int ui;
#endif
#ifdef _p3_SECONDARY
    p3subnet *subnet;
#endif

	switch (cfg_def->tokens[1]) {

	case p3L2HOSTNAME:
		if (config->local->hostname != NULL
				&& strlen(config->local->hostname) < cfg_def->datlen) {
			free(config->local->hostname);
			config->local->hostname = NULL;
		}
		if (config->local->hostname == NULL && (config->local->hostname =
				(char *) p3malloc(cfg_def->datlen)) == NULL) {
			sprintf(p3buf, "handle_local_defs: Failed to allocate hostname\
 buffer: %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
		strcpy(config->local->hostname, cfg_def->data);
		config->local->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2FIFO:
		if (cfg_def->tokens[2] == p3L3U2SNAME) {
			if (config->local->u2sname != NULL
					&& strlen(config->local->u2sname) < cfg_def->datlen) {
				free(config->local->u2sname);
				config->local->u2sname = NULL;
			}
			if (config->local->u2sname == NULL && (config->local->u2sname =
					(char *) p3malloc(cfg_def->datlen)) == NULL) {
				sprintf(p3buf, "handle_local_defs: Failed to allocate FIFO\
 directory buffer: %s\n", strerror(errno));
				p3errmsg (p3MSG_CRIT, p3buf);
				stat = -1;
				goto out;
			}
			strcpy(config->local->u2sname, cfg_def->data);
		} else if (cfg_def->tokens[2] == p3L3S2UNAME) {
			if (config->local->s2uname != NULL
					&& strlen(config->local->s2uname) < cfg_def->datlen) {
				free(config->local->s2uname);
				config->local->s2uname = NULL;
			}
			if (config->local->s2uname == NULL && (config->local->s2uname =
					(char *) p3malloc(cfg_def->datlen)) == NULL) {
				sprintf(p3buf, "handle_local_defs: Failed to allocate FIFO\
 directory buffer: %s\n", strerror(errno));
				p3errmsg (p3MSG_CRIT, p3buf);
				stat = -1;
				goto out;
			}
			strcpy(config->local->s2uname, cfg_def->data);
		} else {
			sprintf(errmsg, "Invalid FIFO definition field: %s",
					level3[(unsigned int) cfg_def->tokens[2]]);
			stat = 1;
			goto out;
		}
		config->local->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2IPVER:
		if (cfg_def->data[0] == '4' &&
				!(config->local->flag & p3HST_IPV6)) {
			config->local->flag |= p3HST_IPV4;
		} else if (cfg_def->data[0] == '6' &&
				!(config->local->flag & p3HST_IPV4)) {
			config->local->flag |= p3HST_IPV6;
		} else {
			sprintf(errmsg, "Invalid subnet IP version: %s",
					cfg_def->data);
			stat = 1;
			goto out;
		}
		config->local->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2ADDRESS:
		// If not IPv6, test address for IPv4
		if (!(config->local->flag & p3HST_IPV6)) {
			if (parse_ipaddr(cfg_def->data, NULL, NULL,
					(p3HST_IPV4 | p3ADR_STR), errmsg) < 0) {
				stat = 1;
				goto out;
			}
			config->local->flag |= p3HST_IPV4;
		// If not IPv4, test address for IPv6
		} else if (!(config->local->flag & p3HST_IPV4)) {
			if (parse_ipaddr(cfg_def->data, NULL, NULL,
					(p3HST_IPV6 | p3ADR_STR), errmsg) < 0) {
				stat = 1;
				goto out;
			}
			config->local->flag |= p3HST_IPV6;
		// Both IPv4 and IPv6 set
		} else {
			config->local->flag &= ~p3LHCFG_IPVER;
			sprintf(errmsg, "Error in IP version");
			stat = 1;
			goto out;
		}
		if ((config->local->addr = (char *) p3malloc(cfg_def->datlen))
				== NULL) {
			sprintf(p3buf, "handle_local_defs: Failed to allocate host\
address buffer: %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
		strcpy(config->local->addr, cfg_def->data);
		config->local->flag |= p3LHCFG_UPDATE;
		break;

#ifdef _p3_PRIMARY
	case p3L2PORT:
		i = strtol(cfg_def->data, NULL, 10);
		if (NOT_PORT(i)) {
			sprintf(errmsg, "Invalid port: %d", i);
			stat = 1;
			goto out;
		}
		config->local->port = i;
		config->local->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2GENKEY:
		for (i=0; i < cfg_def->datlen; i++)
			cfg_def->data[i] = toupper(cfg_def->data[i]);
		if (strcmp(cfg_def->data, "HARDWARE") == 0)
			config->local->flag |= p3LOC_HWGEN;
		else if (strcmp(cfg_def->data, "SOFTWARE") != 0) {
			sprintf(errmsg, "Invalid key generation option: %s", cfg_def->data);
			stat = 1;
			goto out;
		}
		config->local->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2ENCRYPTION:
		for (i=0; i < cfg_def->datlen; i++)
			cfg_def->data[i] = toupper(cfg_def->data[i]);
		if (strcmp(cfg_def->data, "AES128") == 0) {
			config->local->flag &= ~p3LHCFG_KEYALG;
			config->local->flag |= p3LHCFG_AES128;
		} else if (strcmp(cfg_def->data, "AES256") == 0) {
			config->local->flag &= ~p3LHCFG_KEYALG;
			config->local->flag |= p3LHCFG_AES256;
		} else {
			sprintf(errmsg, "Invalid key generation option: %s", cfg_def->data);
			stat = 1;
			goto out;
		}
		config->local->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2REKEY:
		ui = strtol(cfg_def->data, NULL, 10);
		if (cfg_def->tokens[2] == p3L3TIME) {
			if (NOT_TIME(ui)) {
				sprintf(errmsg, "Invalid rekeying time: %d", ui);
				stat = 1;
				goto out;
			}
			config->local->rekey = ui;
			config->local->flag &= ~p3LHCFG_REKEY;
			config->local->flag |= p3LHCFG_REKEYTM;
		} else if (cfg_def->tokens[2] == p3L3PACKETS) {
			if (NOT_PACKETS(ui)) {
				sprintf(errmsg, "Invalid rekeying packet count: %d", ui);
				stat = 1;
				goto out;
			}
			config->local->rekey = ui;
			config->local->flag &= ~p3LHCFG_REKEY;
			config->local->flag |= p3LHCFG_REKEYPKT;
		} else {
			sprintf(errmsg, "Invalid rekeying definition field: %s",
					level3[(unsigned int) cfg_def->tokens[2]]);
			stat = 1;
			goto out;
		}
		config->local->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2ARRAY:
		if (cfg_def->tokens[2] == p3L3USE) {
			for (i=0; i < cfg_def->datlen; i++)
				cfg_def->data[i] = toupper(cfg_def->data[i]);
			if (strcmp(cfg_def->data, "YES") == 0) {
				config->local->flag &= ~p3LHCFG_NUSEARRAY;
			} else if (strcmp(cfg_def->data, "NO") == 0) {
				config->local->flag |= p3LHCFG_NUSEARRAY;
			}
		} else if (cfg_def->tokens[2] == p3L3SIZE) {
			i = strtol(cfg_def->data, NULL, 10);
			if (!(64 <= i && i <= 1024)) {
				sprintf(errmsg, "Invalid key array size: %d", i);
				stat = 1;
				goto out;
			}
			config->local->arraysz = i;
		} else if (cfg_def->tokens[2] == p3L3DATA) {
			ui = strtol(cfg_def->data, NULL, 10);
			if (cfg_def->tokens[3] == p3L4TIME) {
				if (NOT_TIME(ui)) {
					sprintf(errmsg, "Invalid data array time: %d", ui);
					stat = 1;
					goto out;
				}
				config->local->datarray = ui;
				config->local->flag &= ~p3LHCFG_DATA;
				config->local->flag |= p3LHCFG_DATTM;
			} else if (cfg_def->tokens[3] == p3L4PACKETS) {
				if (NOT_PACKETS(ui)) {
					sprintf(errmsg, "Invalid data array packet count: %d", ui);
					stat = 1;
					goto out;
				}
				config->local->datarray = ui;
				config->local->flag &= ~p3LHCFG_DATA;
				config->local->flag |= p3LHCFG_DATPKT;
			} else {
				sprintf(errmsg, "Invalid array data definition field: %s",
						level4[(unsigned int) cfg_def->tokens[3]]);
				stat = 1;
				goto out;
			}
		} else if (cfg_def->tokens[2] == p3L3CONTROL) {
			ui = strtol(cfg_def->data, NULL, 10);
			if (cfg_def->tokens[3] == p3L4TIME) {
				if (NOT_TIME(ui)) {
					sprintf(errmsg, "Invalid control array time: %d", ui);
					stat = 1;
					goto out;
				}
				config->local->ctlarray = ui;
				config->local->flag &= ~p3LHCFG_CONTROL;
				config->local->flag |= p3LHCFG_CTLTM;
			} else if (cfg_def->tokens[3] == p3L4PACKETS) {
				if (NOT_PACKETS(ui)) {
					sprintf(errmsg, "Invalid control array packet count: %d", ui);
					stat = 1;
					goto out;
				}
				config->local->ctlarray = ui;
				config->local->flag &= ~p3LHCFG_CONTROL;
				config->local->flag |= p3LHCFG_CTLPKT;
			} else {
				sprintf(errmsg, "Invalid array data definition field: %s",
						level4[(unsigned int) cfg_def->tokens[3]]);
				stat = 1;
				goto out;
			}
		} else {
			sprintf(errmsg, "Invalid array definition field: %s",
					level3[(unsigned int) cfg_def->tokens[2]]);
			stat = 1;
			goto out;
		}
		config->local->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2HEARTBEAT:
		if (cfg_def->tokens[2] == p3L3TIME) {
			ui = strtol(cfg_def->data, NULL, 10);
			if (!(15 <= ui && ui <= 3600)) {
				sprintf(errmsg, "Invalid heartbeat period: %d", ui);
				stat = 1;
				goto out;
			}
			config->local->heartbeat = ui;
		} else if (cfg_def->tokens[2] == p3L3FAIL) {
			ui = strtol(cfg_def->data, NULL, 10);
			if (!(5 <= ui && ui <= 600)) {
				sprintf(errmsg, "Invalid heartbeat failure time: %d", ui);
				stat = 1;
				goto out;
			}
			config->local->heartfail = ui;
		} else {
			sprintf(errmsg, "Invalid heartbeat definition field: %s",
					level3[(unsigned int) cfg_def->tokens[2]]);
			stat = 1;
			goto out;
		}
		config->local->flag |= p3LHCFG_UPDATE;
		break;
#endif

#ifdef _p3_SECONDARY
	case p3L2SUBNET:
		subnet = config->local->subnets;
		while (subnet != NULL) {
			if (subnet->ID == cfg_def->id1)
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
			subnet->ID = cfg_def->id1;
			// Add subnet to head of list
			if (config->local->subnets != NULL)
				subnet->next = config->local->subnets;
			config->local->subnets = subnet;
		}
		if (cfg_def->tokens[2] == p3L3IPVER) {
			if (cfg_def->data[0] == '4' && !(subnet->flag & p3HST_IPV6)) {
				subnet->flag |= p3HST_IPV4;
			} else if (cfg_def->data[0] == '6' && !(subnet->flag & p3HST_IPV4)) {
				subnet->flag |= p3HST_IPV6;
			} else {
				sprintf(errmsg, "Invalid subnet IP version: %s",
						cfg_def->data);
				stat = 1;
				goto out;
			}
		} else if (cfg_def->tokens[2] == p3L3ADDRESS) {
			// If not IPv6, test address for IPv4
			if (!(subnet->flag & p3HST_IPV6)) {
				if (parse_ipaddr(cfg_def->data, NULL, NULL,
						(p3HST_IPV4 | p3ADR_MASK | p3ADR_STR), errmsg) < 0) {
					stat = 1;
					goto out;
				}
				subnet->flag |= p3HST_IPV4;
			// If not IPv4, test address for IPv6
			} else if (!(subnet->flag & p3HST_IPV4)) {
				if (parse_ipaddr(cfg_def->data, NULL, NULL,
						(p3HST_IPV6 | p3ADR_MASK | p3ADR_STR), errmsg) < 0) {
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
			if ((subnet->addr = (char *) p3malloc(cfg_def->datlen))
					== NULL) {
				sprintf(p3buf, "handle_local_defs: Failed to allocate local\
 subnet buffer: %s\n", strerror(errno));
				p3errmsg (p3MSG_CRIT, p3buf);
				stat = -1;
				goto out;
			}
			strcpy(subnet->addr, cfg_def->data);
		} else {
			sprintf(errmsg, "Invalid subnet definition field: %s",
					level3[(unsigned int) cfg_def->tokens[2]]);
			stat = 1;
			goto out;
		}
		config->local->flag |= p3LHCFG_UPDATE;
		break;
#endif

	default:
		sprintf(errmsg, "Invalid local definition field: %s",
				level2[(unsigned int) cfg_def->tokens[1]]);
		stat = 1;
		break;

	}

out:
	return(stat);
} /* end handle_local_defs */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>handle_local_defs: Failed to allocate <i>data structure</i>:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 configuration handler attempts to allocate the required structures
 * to manage local host data.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

#ifdef _p3_PRIMARY
/**
 * \par Function:
 * handle_group_defs
 *
 * \par Description:
 * Handle the group configuration definitions by setting the
 * appropriate group structure values, based on the parsed data.
 *
 * \par Inputs:
 * - cfg_def: The parsed definition values.
 * - group: The group structure to be updated
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - >0 = Parser error
 *   - <0 = System error
 */

int handle_group_defs(p3comm *cfg_def, p3group *group)
{
	int i, stat = 0;
	unsigned int ui;

	switch (cfg_def->tokens[1]) {
	case p3L2NAME:
		if (group->name != NULL
				&& strlen(group->name) < cfg_def->datlen) {
			free(group->name);
			group->name = NULL;
		}
		if (group->name == NULL && (group->name =
				(char *) p3malloc(cfg_def->datlen)) == NULL) {
			sprintf(p3buf, "handle_group_defs: Failed to allocate name\
 buffer: %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
		strcpy(group->name, cfg_def->data);
		config->group->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2CATEGORY:
		for (i=0; i < cfg_def->datlen; i++)
			cfg_def->data[i] = toupper(cfg_def->data[i]);
		if (strcmp(cfg_def->data, "ORG") == 0)
			group->level = p3GRP_ORG;
		else if (strcmp(cfg_def->data, "LOC") == 0)
			group->level = p3GRP_LOC;
		else if (strcmp(cfg_def->data, "SEC") == 0)
			group->level = p3GRP_SEC;
		else if (strcmp(cfg_def->data, "NET") == 0)
			group->level = p3GRP_NET;
		else {
			sprintf(errmsg, "Invalid group category: %s", cfg_def->data);
			stat = 1;
			goto out;
		}
		config->group->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2ENCRYPTION:
		for (i=0; i < cfg_def->datlen; i++)
			cfg_def->data[i] = toupper(cfg_def->data[i]);
		if (strcmp(cfg_def->data, "AES128") == 0) {
			group->flag &= ~p3LHCFG_KEYALG;
			group->flag |= p3LHCFG_AES128;
		} else if (strcmp(cfg_def->data, "AES256") == 0) {
			group->flag &= ~p3LHCFG_KEYALG;
			group->flag |= p3LHCFG_AES256;
		} else {
			sprintf(errmsg, "Invalid group key generation option: %s", cfg_def->data);
			stat = 1;
			goto out;
		}
		config->group->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2REKEY:
		ui = strtol(cfg_def->data, NULL, 10);
		if (cfg_def->tokens[2] == p3L3TIME) {
			if (NOT_TIME(ui)) {
				sprintf(errmsg, "Invalid group rekeying time: %d", ui);
				stat = 1;
				goto out;
			}
			group->rekey = ui;
			group->flag &= ~p3LHCFG_REKEY;
			group->flag |= p3LHCFG_REKEYTM;
		} else if (cfg_def->tokens[2] == p3L3PACKETS) {
			if (NOT_PACKETS(ui)) {
				sprintf(errmsg, "Invalid group rekeying packet count: %d", ui);
				stat = 1;
				goto out;
			}
			group->rekey = ui;
			group->flag &= ~p3LHCFG_REKEY;
			group->flag |= p3LHCFG_REKEYPKT;
		} else {
			sprintf(errmsg, "Invalid group rekeying definition field: %s",
					level3[(unsigned int) cfg_def->tokens[2]]);
			stat = 1;
			goto out;
		}
		config->group->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2ARRAY:
		if (cfg_def->tokens[2] == p3L3USE) {
			for (i=0; i < cfg_def->datlen; i++)
				cfg_def->data[i] = toupper(cfg_def->data[i]);
			if (strcmp(cfg_def->data, "YES") == 0) {
				group->flag &= ~p3LHCFG_NUSEARRAY;
			} else if (strcmp(cfg_def->data, "NO") == 0) {
				group->flag |= p3LHCFG_NUSEARRAY;
			}
		} else if (cfg_def->tokens[2] == p3L3SIZE) {
			i = strtol(cfg_def->data, NULL, 10);
			if (!(64 <= i && i <= 1024)) {
				sprintf(errmsg, "Invalid group key array size: %d", i);
				stat = 1;
				goto out;
			}
			group->arraysz = i;
		} else if (cfg_def->tokens[2] == p3L3DATA) {
			ui = strtol(cfg_def->data, NULL, 10);
			if (cfg_def->tokens[3] == p3L4TIME) {
				if (NOT_TIME(ui)) {
					sprintf(errmsg, "Invalid group data array time: %d", ui);
					stat = 1;
					goto out;
				}
				group->datarray = ui;
				group->flag &= ~p3LHCFG_DATA;
				group->flag |= p3LHCFG_DATTM;
			} else if (cfg_def->tokens[3] == p3L4PACKETS) {
				if (NOT_PACKETS(ui)) {
					sprintf(errmsg, "Invalid group data array packet count: %d", ui);
					stat = 1;
					goto out;
				}
				group->datarray = ui;
				group->flag &= ~p3LHCFG_DATA;
				group->flag |= p3LHCFG_DATPKT;
			} else {
				sprintf(errmsg, "Invalid group array data definition field: %s",
						level4[(unsigned int) cfg_def->tokens[3]]);
				stat = 1;
				goto out;
			}
		} else if (cfg_def->tokens[2] == p3L3CONTROL) {
			ui = strtol(cfg_def->data, NULL, 10);
			if (cfg_def->tokens[3] == p3L4TIME) {
				if (NOT_TIME(ui)) {
					sprintf(errmsg, "Invalid group control array time: %d", ui);
					stat = 1;
					goto out;
				}
				group->ctlarray = ui;
				group->flag &= ~p3LHCFG_CONTROL;
				group->flag |= p3LHCFG_CTLTM;
			} else if (cfg_def->tokens[3] == p3L4PACKETS) {
				if (NOT_PACKETS(ui)) {
					sprintf(errmsg, "Invalid group control array packet count: %d", ui);
					stat = 1;
					goto out;
				}
				group->ctlarray = ui;
				group->flag &= ~p3LHCFG_CONTROL;
				group->flag |= p3LHCFG_CTLPKT;
			} else {
				sprintf(errmsg, "Invalid group array data definition field: %s",
						level4[(unsigned int) cfg_def->tokens[3]]);
				stat = 1;
				goto out;
			}
		} else {
			sprintf(errmsg, "Invalid group array definition field: %s",
					level3[(unsigned int) cfg_def->tokens[2]]);
			stat = 1;
			goto out;
		}
		config->group->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2HEARTBEAT:
		if (cfg_def->tokens[2] == p3L3TIME) {
			ui = strtol(cfg_def->data, NULL, 10);
			if (!(15 <= ui && ui <= 3600)) {
				sprintf(errmsg, "Invalid group heartbeat period: %d", ui);
				stat = 1;
				goto out;
			}
			group->heartbeat = ui;
		} else if (cfg_def->tokens[2] == p3L3FAIL) {
			ui = strtol(cfg_def->data, NULL, 10);
			if (!(5 <= ui && ui <= 600)) {
				sprintf(errmsg, "Invalid group heartbeat failure time: %d", ui);
				stat = 1;
				goto out;
			}
			group->heartfail = ui;
		} else {
			sprintf(errmsg, "Invalid group heartbeat definition field: %s",
					level3[(unsigned int) cfg_def->tokens[2]]);
			stat = 1;
			goto out;
		}
		config->group->flag |= p3LHCFG_UPDATE;
		break;

	default:
		sprintf(errmsg, "Invalid group definition field: %s",
				level2[(unsigned int) cfg_def->tokens[1]]);
		stat = 1;
		break;

	}

out:
	return(stat);
} /* end handle_group_defs */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>handle_group_defs: Failed to allocate <i>data structure</i>:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 configuration handler attempts to allocate the required structures
 * to manage group data.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

#endif

/**
 * \par Function:
 * handle_remote_defs
 *
 * \par Description:
 * Handle the remote configuration definitions by setting the
 * appropriate remote structure values, based on the parsed data.
 *
 * \par Inputs:
 * - cfg_def: The parsed definition values.
 * - remote: The remote structure to be updated
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - >0 = Parser error
 *   - <0 = System error
 */

int handle_remote_defs(p3comm *cfg_def, p3remote *remote)
{
	int i, stat = 0;
#ifdef _p3_PRIMARY
	unsigned int ui;
	int gcat;
	char *group;
#endif
	p3subnet *subnet;

	switch (cfg_def->tokens[1]) {

	case p3L2HOSTNAME:
		if (remote->hostname != NULL
				&& strlen(remote->hostname) < cfg_def->datlen) {
			free(remote->hostname);
			remote->hostname = NULL;
		}
		if (remote->hostname == NULL && (remote->hostname =
				(char *) p3malloc(cfg_def->datlen)) == NULL) {
			sprintf(p3buf, "handle_remote_defs: Failed to allocate hostname\
 buffer: %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
		strcpy(remote->hostname, cfg_def->data);
		config->remote->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2IPVER:
		if (cfg_def->data[0] == '4' && !(remote->flag & p3HST_IPV6)) {
			remote->flag |= p3HST_IPV4;
		} else if (cfg_def->data[0] == '6' && !(remote->flag & p3HST_IPV4)) {
			remote->flag |= p3HST_IPV6;
		} else {
			sprintf(errmsg, "Invalid subnet IP version: %s",
					cfg_def->data);
			stat = 1;
			goto out;
		}
		config->remote->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2ADDRESS:
		// If not IPv6, test address for IPv4
		if (!(remote->flag & p3HST_IPV6)) {
			if (parse_ipaddr(cfg_def->data, NULL, NULL,
					(p3HST_IPV4 | p3ADR_STR), errmsg) < 0) {
				stat = 1;
				goto out;
			}
			remote->flag |= p3HST_IPV4;
		// If not IPv4, test address for IPv6
		} else if (!(remote->flag & p3HST_IPV4)) {
			if (parse_ipaddr(cfg_def->data, NULL, NULL,
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
		if ((remote->addr = (char *) p3malloc(cfg_def->datlen))
				== NULL) {
			sprintf(p3buf, "handle_remote_defs: Failed to allocate remote\
 address buffer: %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		}
		strcpy(remote->addr, cfg_def->data);
		config->remote->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2SUBNET:
		subnet = remote->subnets;
		while (subnet != NULL) {
			if (subnet->ID == cfg_def->id2)
				break;
			subnet = subnet->next;
		}
		if (subnet == NULL) {
			if ((subnet = (p3subnet *) p3calloc(sizeof(p3subnet))) == NULL) {
				sprintf(p3buf, "handle_remote_defs: Failed to allocate subnet\
 structure: %s\n", strerror(errno));
				p3errmsg (p3MSG_CRIT, p3buf);
				stat = -1;
				goto out;
			}
			subnet->ID = cfg_def->id2;
			// Add subnet to head of list
			if (remote->subnets != NULL)
				subnet->next = remote->subnets;
			remote->subnets = subnet;
		}
		if (cfg_def->tokens[2] == p3L3IPVER) {
			if (cfg_def->data[0] == '4' && !(subnet->flag & p3HST_IPV6)) {
				subnet->flag |= p3HST_IPV4;
			} else if (cfg_def->data[0] == '6' && !(subnet->flag & p3HST_IPV4)) {
				subnet->flag |= p3HST_IPV6;
			} else {
				sprintf(errmsg, "Invalid subnet IP version: %s",
						cfg_def->data);
				stat = 1;
				goto out;
			}
		} else if (cfg_def->tokens[2] == p3L3ADDRESS) {
			// If not IPv6, test address for IPv4
			if (!(subnet->flag & p3HST_IPV6)) {
				if (parse_ipaddr(cfg_def->data, NULL, NULL,
						(p3HST_IPV4 | p3ADR_STR), errmsg) < 0) {
					stat = 1;
					goto out;
				}
				subnet->flag |= p3HST_IPV4;
			// If not IPv4, test address for IPv6
			} else if (!(subnet->flag & p3HST_IPV4)) {
				if (parse_ipaddr(cfg_def->data, NULL, NULL,
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
			if ((subnet->addr = (char *) p3malloc(cfg_def->datlen))
					== NULL) {
				sprintf(p3buf, "handle_remote_defs: Failed to allocate remote\
 subnet address buffer: %s\n", strerror(errno));
				p3errmsg (p3MSG_CRIT, p3buf);
				stat = -1;
				goto out;
			}
			strcpy(subnet->addr, cfg_def->data);
		} else {
			sprintf(errmsg, "Invalid subnet definition field: %s",
					level3[(unsigned int) cfg_def->tokens[2]]);
			stat = 1;
			goto out;
		}
		config->remote->flag |= p3LHCFG_UPDATE;
		break;

#ifdef _p3_PRIMARY
	case p3L2GROUP:
		if (grouplist != NULL) {
			// Separate category and group name
			group = NULL;
			for (i=0; i < strlen(cfg_def->data); i++) {
				if (cfg_def->data[i] == ',') {
					cfg_def->data[i] = '\0';
					i++;
					while (cfg_def->data[i] == ' ')
						i++;
					group = &cfg_def->data[i];
					break;
				}
			}
			if (group == NULL) {
				sprintf(errmsg, "Invalid remote group definition: %s",
					cfg_def->data);
				stat = 1;
				goto out;
			}
			if (strncmp(cfg_def->data, "ORG", 3) == 0)
				gcat = p3GRP_ORG;
			else if (strncmp(cfg_def->data, "LOC", 3) == 0)
				gcat = p3GRP_LOC;
			else if (strncmp(cfg_def->data, "SEC", 3) == 0)
				gcat = p3GRP_SEC;
			else if (strncmp(cfg_def->data, "NET", 3) == 0)
				gcat = p3GRP_NET;
			else {
				sprintf(errmsg, "Invalid group category: %s", cfg_def->data);
				stat = 1;
				goto out;
			}
			for (i=0; i < config->groups; i++) {
				if (grouplist[i] == NULL)
					continue;
				if (grouplist[i]->level == gcat &&
						strcmp(grouplist[i]->name, group) == 0) {
					set_rgroup(remote, (gcat - 1), i);
					goto out;
				}
			}
			sprintf(errmsg, "Group not found: %s,%s", cfg_def->data, group);
			stat = 1;
			goto out;
		}
		config->remote->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2ENCRYPTION:
		for (i=0; i < cfg_def->datlen; i++)
			cfg_def->data[i] = toupper(cfg_def->data[i]);
		if (strcmp(cfg_def->data, "AES128") == 0) {
			remote->flag &= ~p3LHCFG_KEYALG;
			remote->flag |= p3LHCFG_AES128;
		} else if (strcmp(cfg_def->data, "AES256") == 0) {
			remote->flag &= ~p3LHCFG_KEYALG;
			remote->flag |= p3LHCFG_AES256;
		} else {
			sprintf(errmsg, "Invalid key generation option: %s", cfg_def->data);
			stat = 1;
			goto out;
		}
		config->remote->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2REKEY:
		ui = strtol(cfg_def->data, NULL, 10);
		if (cfg_def->tokens[2] == p3L3TIME) {
			if (NOT_TIME(ui)) {
				sprintf(errmsg, "Invalid rekeying time: %d", ui);
				stat = 1;
				goto out;
			}
			remote->rekey = ui;
			remote->flag &= ~p3LHCFG_REKEY;
			remote->flag |= p3LHCFG_REKEYTM;
		} else if (cfg_def->tokens[2] == p3L3PACKETS) {
			if (NOT_PACKETS(ui)) {
				sprintf(errmsg, "Invalid rekeying packet count: %d", ui);
				stat = 1;
				goto out;
			}
			remote->rekey = ui;
			remote->flag &= ~p3LHCFG_REKEY;
			remote->flag |= p3LHCFG_REKEYPKT;
		} else {
			sprintf(errmsg, "Invalid rekeying definition field: %s",
					level3[(unsigned int) cfg_def->tokens[2]]);
			stat = 1;
			goto out;
		}
		config->remote->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2ARRAY:
		if (cfg_def->tokens[2] == p3L3USE) {
			for (i=0; i < cfg_def->datlen; i++)
				cfg_def->data[i] = toupper(cfg_def->data[i]);
			if (strcmp(cfg_def->data, "YES") == 0) {
				remote->flag &= ~p3LHCFG_NUSEARRAY;
			} else if (strcmp(cfg_def->data, "NO") == 0) {
				remote->flag |= p3LHCFG_NUSEARRAY;
			}
		} else if (cfg_def->tokens[2] == p3L3SIZE) {
			i = strtol(cfg_def->data, NULL, 10);
			if (!(64 <= i && i <= 1024)) {
				sprintf(errmsg, "Invalid key array size: %d", i);
				stat = 1;
				goto out;
			}
			remote->arraysz = i;
		} else if (cfg_def->tokens[2] == p3L3DATA) {
			ui = strtol(cfg_def->data, NULL, 10);
			if (cfg_def->tokens[3] == p3L4TIME) {
				if (NOT_TIME(ui)) {
					sprintf(errmsg, "Invalid data array time: %d", ui);
					stat = 1;
					goto out;
				}
				remote->datarray = ui;
				remote->flag &= ~p3LHCFG_DATA;
				remote->flag |= p3LHCFG_DATTM;
			} else if (cfg_def->tokens[3] == p3L4PACKETS) {
				if (NOT_PACKETS(ui)) {
					sprintf(errmsg, "Invalid data array packet count: %d", ui);
					stat = 1;
					goto out;
				}
				remote->datarray = ui;
				remote->flag &= ~p3LHCFG_DATA;
				remote->flag |= p3LHCFG_DATPKT;
			} else {
				sprintf(errmsg, "Invalid array data definition field: %s",
						level4[(unsigned int) cfg_def->tokens[3]]);
				stat = 1;
				goto out;
			}
		} else if (cfg_def->tokens[2] == p3L3CONTROL) {
			ui = strtol(cfg_def->data, NULL, 10);
			if (cfg_def->tokens[3] == p3L4TIME) {
				if (NOT_TIME(ui)) {
					sprintf(errmsg, "Invalid control array time: %d", ui);
					stat = 1;
					goto out;
				}
				remote->ctlarray = ui;
				remote->flag &= ~p3LHCFG_CONTROL;
				remote->flag |= p3LHCFG_CTLTM;
			} else if (cfg_def->tokens[3] == p3L4PACKETS) {
				if (NOT_PACKETS(ui)) {
					sprintf(errmsg, "Invalid control array packet count: %d", ui);
					stat = 1;
					goto out;
				}
				remote->ctlarray = ui;
				remote->flag &= ~p3LHCFG_CONTROL;
				remote->flag |= p3LHCFG_CTLPKT;
			} else {
				sprintf(errmsg, "Invalid array data definition field: %s",
						level4[(unsigned int) cfg_def->tokens[3]]);
				stat = 1;
				goto out;
			}
		} else {
			sprintf(errmsg, "Invalid array definition field: %s",
					level3[(unsigned int) cfg_def->tokens[2]]);
			stat = 1;
			goto out;
		}
		config->remote->flag |= p3LHCFG_UPDATE;
		break;

	case p3L2HEARTBEAT:
		if (cfg_def->tokens[2] == p3L3TIME) {
			ui = strtol(cfg_def->data, NULL, 10);
			if (!(15 <= ui && ui <= 3600)) {
				sprintf(errmsg, "Invalid heartbeat period: %d", ui);
				stat = 1;
				goto out;
			}
			remote->heartbeat = ui;
		} else if (cfg_def->tokens[2] == p3L3FAIL) {
			ui = strtol(cfg_def->data, NULL, 10);
			if (!(5 <= ui && ui <= 600)) {
				sprintf(errmsg, "Invalid heartbeat failure time: %d", ui);
				stat = 1;
				goto out;
			}
			remote->heartfail = ui;
		} else {
			sprintf(errmsg, "Invalid heartbeat definition field: %s",
					level3[(unsigned int) cfg_def->tokens[2]]);
			stat = 1;
			goto out;
		}
		config->remote->flag |= p3LHCFG_UPDATE;
		break;
#endif

#ifdef _p3_SECONDARY
	case p3L2PORT:
		i = strtol(cfg_def->data, NULL, 10);
		if (NOT_PORT(i)) {
			sprintf(errmsg, "Invalid port: %d", i);
			stat = 1;
			goto out;
		}
		remote->port = i;
		config->remote->flag |= p3LHCFG_UPDATE;
		break;
#endif

	default:
		sprintf(errmsg, "Invalid remote definition field: %s",
				level2[(unsigned int) cfg_def->tokens[1]]);
		stat = 1;
		break;

	}

out:
	return(stat);
} /* end handle_remote_defs */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>handle_remote_defs: Failed to allocate <i>data structure</i>:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 configuration handler attempts to allocate the required structures
 * to manage remote host data.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

/**
 * \par Function:
 * save_config
 *
 * \par Description:
 * Save all P3 definitions to the configuration file.
 *
 * \par Inputs:
 * - None
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int save_config()
{
	int i, sz, stat = 0;
	char new_filename[p3BUFSIZE], old_filename[p3BUFSIZE];
	FILE *cfile = NULL;
#ifdef _p3_PRIMARY
	int j;
	p3group *group;
#endif
	p3remote *remote;
	p3subnet *subnet;

	// Read and parse primary configuration file
#ifdef _p3_PRIMARY
	sprintf(new_filename, "%s%s.new", config->path, config->pricfg);
#else
	sprintf(new_filename, "%s%s.new", config->path, config->seccfg);
#endif
	if ((cfile = fopen(new_filename, "w")) == NULL) {
		sprintf(p3buf, "get_config: Failed to open configuration\
 file %s: %s\n", new_filename, strerror(errno));
		p3errmsg (p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	}

	// Build local configuration
	if (validate_local(config->local) < 0) {
		sprintf(p3buf, "get_config: Local definition errors:%s\n", errmsg);
		p3errmsg (p3MSG_ERR, p3buf);
		stat = -1;
		goto out;
	}
#ifdef _p3_PRIMARY
	strcpy(p3buf, "#\n# Primary local host definitions\n#\n");
#else
	strcpy(p3buf, "#\n# Secondary local host definitions\n#\n");
#endif
	sz = strlen(p3buf);
	sprintf(cfgbuf, "%s/%s = %s\n", p3L1LOCAL_TOK, p3L2HOSTNAME_TOK,
		config->local->hostname);
	sprintf(p3buf, "%s%s", p3buf, cfgbuf);
	sprintf(cfgbuf, "%s/%s = %s\n", p3L1LOCAL_TOK, p3L2ADDRESS_TOK,
		config->local->addr);
	sprintf(p3buf, "%s%s", p3buf, cfgbuf);
#ifdef _p3_PRIMARY
	// Set key management defaults
	if (config->local->flag & p3LHCFG_REKEYTM)
		sprintf(cfgbuf, "%s/%s/%s = %d\n", p3L1LOCAL_TOK, p3L2REKEY_TOK,
			p3L3TIME_TOK, config->local->rekey);
	else
		sprintf(cfgbuf, "%s/%s/%s = %d\n", p3L1LOCAL_TOK, p3L2REKEY_TOK,
			p3L3PACKETS_TOK, config->local->rekey);
	sprintf(p3buf, "%s%s", p3buf, cfgbuf);
	sprintf(cfgbuf, "%s/%s/%s = %d\n", p3L1LOCAL_TOK, p3L2ARRAY_TOK,
		p3L3SIZE_TOK, config->local->arraysz);
	sprintf(p3buf, "%s%s", p3buf, cfgbuf);
	if (config->local->flag & p3LHCFG_DATTM)
		sprintf(cfgbuf, "%s/%s/%s/%s = %d\n", p3L1LOCAL_TOK, p3L2ARRAY_TOK,
			p3L3DATA_TOK, p3L4TIME_TOK, config->local->datarray);
	else
		sprintf(cfgbuf, "%s/%s/%s/%s = %d\n", p3L1LOCAL_TOK, p3L2ARRAY_TOK,
			p3L3DATA_TOK, p3L4PACKETS_TOK, config->local->datarray);
	sprintf(p3buf, "%s%s", p3buf, cfgbuf);
	if (config->local->flag & p3LHCFG_CTLTM)
		sprintf(cfgbuf, "%s/%s/%s/%s = %d\n", p3L1LOCAL_TOK, p3L2ARRAY_TOK,
			p3L3CONTROL_TOK, p3L4TIME_TOK, config->local->ctlarray);
	else
		sprintf(cfgbuf, "%s/%s/%s/%s = %d\n", p3L1LOCAL_TOK, p3L2ARRAY_TOK,
			p3L3CONTROL_TOK, p3L4PACKETS_TOK, config->local->ctlarray);
	sprintf(p3buf, "%s%s", p3buf, cfgbuf);
	sprintf(cfgbuf, "%s/%s/%s = %d\n", p3L1LOCAL_TOK, p3L2HEARTBEAT_TOK,
		p3L3TIME_TOK, config->local->heartbeat);
	sprintf(p3buf, "%s%s", p3buf, cfgbuf);
	sprintf(cfgbuf, "%s/%s/%s = %d\n", p3L1LOCAL_TOK, p3L2HEARTBEAT_TOK,
		p3L3FAIL_TOK, config->local->heartfail);
	sprintf(p3buf, "%s%s", p3buf, cfgbuf);
#endif

#ifdef _p3_SECONDARY
	// Set local subnets for anonymous connections
	subnet = config->local->subnets;
	while (subnet != NULL) {
		if (subnet->flag & p3HST_IPV6)
			i = 6;
		else
			i = 4;
		sprintf(cfgbuf, "%s/%s/%d/%s = %d\n", p3L1LOCAL_TOK, p3L2SUBNET_TOK,
			subnet->ID, p3L3IPVER_TOK, i);
		sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		sprintf(cfgbuf, "%s/%s/%d/%s = %s\n", p3L1LOCAL_TOK, p3L2SUBNET_TOK,
			subnet->ID, p3L3ADDRESS_TOK, subnet->addr);
		sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		subnet = subnet->next;
	}
#endif
	// Save local configuration
	if (sz < strlen(p3buf)) {
		sz = strlen(p3buf);
printf("DEBUG: Write local config: Size %d\n", sz);
		if (fwrite(p3buf, 1, sz, cfile) < sz) {
			sprintf(p3buf, "get_config: Error writing to configuration\
 file %s: %s\n", new_filename, strerror(errno));
			p3errmsg (p3MSG_ERR, p3buf);
			stat = -1;
			goto out;
		}
	}

#ifdef _p3_PRIMARY
	// Build group configuration
	strcpy(p3buf, "#\n# Primary group definitions\n#\n");
	sz = strlen(cfgbuf);
	group = config->group;
	while (group != NULL) {
		if (validate_group(group) < 0) {
			sprintf(p3buf, "get_config: Group %d definition errors:%s\n",
				group->ID, errmsg);
			p3errmsg (p3MSG_ERR, p3buf);
			stat = -1;
			goto out;
		}
		sprintf(cfgbuf, "%s/%d/%s = %s\n", p3L1GROUP_TOK, group->ID,
			p3L2NAME_TOK, group->name);
		sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		if (group->level == p3GRP_ORG)
			sprintf(cfgbuf, "%s/%d/%s = ORG\n", p3L1GROUP_TOK, group->ID,
				p3L2CATEGORY_TOK);
		else if (group->level == p3GRP_LOC)
			sprintf(cfgbuf, "%s/%d/%s = LOC\n", p3L1GROUP_TOK, group->ID,
				p3L2CATEGORY_TOK);
		else if (group->level == p3GRP_SEC)
			sprintf(cfgbuf, "%s/%d/%s = SEC\n", p3L1GROUP_TOK, group->ID,
				p3L2CATEGORY_TOK);
		else if (group->level == p3GRP_NET)
			sprintf(cfgbuf, "%s/%d/%s = NET\n", p3L1GROUP_TOK, group->ID,
				p3L2CATEGORY_TOK);
		sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		// Override encryption algorithm
		if (group->flag & p3LHCFG_AES128) {
			sprintf(cfgbuf, "%s/%d/%s = AES128\n", p3L1GROUP_TOK, group->ID,
				p3L2ENCRYPTION_TOK);
			sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		} else if (group->flag & p3LHCFG_AES256) {
			sprintf(cfgbuf, "%s/%d/%s = AES256\n", p3L1GROUP_TOK, group->ID,
				p3L2ENCRYPTION_TOK);
			sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		}
		// Override rekeying period
		if (group->rekey > 0) {
			if (group->flag & p3LHCFG_REKEYTM)
				sprintf(cfgbuf, "%s/%d/%s/%s = %d\n", p3L1GROUP_TOK, group->ID,
					p3L2REKEY_TOK, p3L3TIME_TOK, group->rekey);
			else
				sprintf(cfgbuf, "%s/%d/%s/%s = %d\n", p3L1GROUP_TOK, group->ID,
					p3L2REKEY_TOK, p3L3PACKETS_TOK, group->rekey);
			sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		}
		// Override array use
		if (group->flag & p3LHCFG_NUSEARRAY) {
			sprintf(cfgbuf, "%s/%d/%s/%s = NO\n", p3L1GROUP_TOK, group->ID,
				p3L2ARRAY_TOK, p3L3USE_TOK);
			sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		} else {
			if (group->arraysz > 0) {
				sprintf(cfgbuf, "%s/%d/%s/%s = %d\n", p3L1GROUP_TOK,
					group->ID, p3L2ARRAY_TOK, p3L3SIZE_TOK, group->arraysz);
				sprintf(p3buf, "%s%s", p3buf, cfgbuf);
			}
			if (group->datarray > 0) {
				if (group->flag & p3LHCFG_DATTM)
					sprintf(cfgbuf, "%s/%d/%s/%s/%s = %d\n", p3L1GROUP_TOK,
						group->ID, p3L2ARRAY_TOK, p3L3DATA_TOK,
						p3L4TIME_TOK, group->datarray);
				else
					sprintf(cfgbuf, "%s/%d/%s/%s/%s = %d\n", p3L1GROUP_TOK,
						group->ID, p3L2ARRAY_TOK, p3L3DATA_TOK,
						p3L4PACKETS_TOK, group->datarray);
				sprintf(p3buf, "%s%s", p3buf, cfgbuf);
			}
			if (group->ctlarray > 0) {
				if (group->flag & p3LHCFG_CTLTM)
					sprintf(cfgbuf, "%s/%d/%s/%s/%s = %d\n", p3L1GROUP_TOK,
						group->ID, p3L2ARRAY_TOK, p3L3CONTROL_TOK,
						p3L4TIME_TOK, group->ctlarray);
				else
					sprintf(cfgbuf, "%s/%d/%s/%s/%s = %d\n", p3L1GROUP_TOK,
						group->ID, p3L2ARRAY_TOK, p3L3CONTROL_TOK,
						p3L4PACKETS_TOK, group->ctlarray);
				sprintf(p3buf, "%s%s", p3buf, cfgbuf);
			}
		}
		// Override heartbeat values
		if (group->heartbeat > 0) {
			sprintf(cfgbuf, "%s/%d/%s/%s = %d\n", p3L1GROUP_TOK, group->ID,
				p3L2HEARTBEAT_TOK, p3L3TIME_TOK, group->heartbeat);
			sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		}
		if (group->heartfail > 0) {
			sprintf(cfgbuf, "%s/%d/%s/%s = %d\n", p3L1GROUP_TOK, group->ID,
				p3L2HEARTBEAT_TOK, p3L3FAIL_TOK, group->heartfail);
			sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		}
		sprintf(p3buf, "%s#\n", p3buf);
		// Save group configuration
		if (sz < strlen(p3buf)) {
			sz = strlen(p3buf);
	printf("DEBUG: Write group (%d) config: Size %d\n", group->ID, sz);
			if (fwrite(p3buf, 1, sz, cfile) < sz) {
				sprintf(p3buf, "get_config: Error writing to configuration\
	 file %s: %s\n", new_filename, strerror(errno));
				p3errmsg (p3MSG_ERR, p3buf);
				stat = -1;
				goto out;
			}
		}
		group = group->next;
		p3buf[0] = '\0';
		sz = 0;
	}
#endif

	// Build remote configuration
#ifdef _p3_PRIMARY
	strcpy(p3buf, "#\n# Primary remote host definitions\n#\n");
#else
	strcpy(p3buf, "#\n# Secondary remote host definitions\n#\n");
#endif
	sz = strlen(p3buf);
	remote = config->remote;
	while (remote != NULL) {
		if (validate_remote(remote) < 0) {
			sprintf(p3buf, "get_config: Remote %d definition errors:%s\n",
				remote->ID, errmsg);
			p3errmsg (p3MSG_ERR, p3buf);
			stat = -1;
			goto out;
		}
#ifdef _p3_PRIMARY
		sprintf(cfgbuf, "%s/%d/%s = %s\n", p3L1REMOTE_TOK, remote->ID,
			p3L2HOSTNAME_TOK, remote->hostname);
		sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		if (remote->flag & p3HST_IPV6)
			i = 6;
		else
			i = 4;
		sprintf(cfgbuf, "%s/%d/%s = %d\n", p3L1REMOTE_TOK, remote->ID,
			p3L2IPVER_TOK, i);
		sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		sprintf(cfgbuf, "%s/%d/%s = %s\n", p3L1REMOTE_TOK, remote->ID,
			p3L2ADDRESS_TOK, remote->addr);
		sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		if (grouplist != NULL) {
			for (i=0; i < p3MAX_GROUPS; i += 2) {
				j = (remote->groups[i] << 8) | remote->groups[i+1];
				if (j != 0xffff && grouplist[j] != NULL) {
					if (grouplist[j]->level == p3GRP_ORG)
						sprintf(cfgbuf, "%s/%d/%s = ORG,%s\n", p3L1REMOTE_TOK,
							remote->ID, p3L2GROUP_TOK,
							grouplist[j]->name);
					else if (grouplist[j]->level == p3GRP_LOC)
						sprintf(cfgbuf, "%s/%d/%s = LOC,%s\n", p3L1REMOTE_TOK,
							remote->ID, p3L2GROUP_TOK,
							grouplist[j]->name);
					else if (grouplist[j]->level == p3GRP_SEC)
						sprintf(cfgbuf, "%s/%d/%s = SEC,%s\n", p3L1REMOTE_TOK,
							remote->ID, p3L2GROUP_TOK,
							grouplist[j]->name);
					else if (grouplist[j]->level == p3GRP_NET)
						sprintf(cfgbuf, "%s/%d/%s = NET,%s\n", p3L1REMOTE_TOK,
							remote->ID, p3L2GROUP_TOK,
							grouplist[j]->name);
					else
						continue;
					sprintf(p3buf, "%s%s", p3buf, cfgbuf);
				}
			}
		}
		subnet = remote->subnets;
		while (subnet != NULL) {
			if (subnet->flag & p3HST_IPV6)
				i = 6;
			else
				i = 4;
			sprintf(cfgbuf, "%s/%d/%s/%d/%s = %d\n", p3L1REMOTE_TOK,
				remote->ID, p3L2SUBNET_TOK, subnet->ID, p3L3IPVER_TOK, i);
			sprintf(p3buf, "%s%s", p3buf, cfgbuf);
			sprintf(cfgbuf, "%s/%d/%s/%d/%s = %s\n", p3L1REMOTE_TOK,
				remote->ID, p3L2SUBNET_TOK, subnet->ID, p3L3ADDRESS_TOK,
				subnet->addr);
			sprintf(p3buf, "%s%s", p3buf, cfgbuf);
			subnet = subnet->next;
		}
		// Override encryption algorithm
		if (remote->flag & p3LHCFG_AES128) {
			sprintf(cfgbuf, "%s/%d/%s = AES128\n", p3L1REMOTE_TOK, remote->ID,
				p3L2ENCRYPTION_TOK);
			sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		} else if (remote->flag & p3LHCFG_AES256) {
			sprintf(cfgbuf, "%s/%d/%s = AES256\n", p3L1REMOTE_TOK, remote->ID,
				p3L2ENCRYPTION_TOK);
			sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		}
		// Override rekeying period
		if (remote->rekey > 0) {
			if (remote->flag & p3LHCFG_REKEYTM)
				sprintf(cfgbuf, "%s/%d/%s/%s = %d\n", p3L1REMOTE_TOK, remote->ID,
					p3L2REKEY_TOK, p3L3TIME_TOK, remote->rekey);
			else
				sprintf(cfgbuf, "%s/%d/%s/%s = %d\n", p3L1REMOTE_TOK, remote->ID,
					p3L2REKEY_TOK, p3L3PACKETS_TOK, remote->rekey);
			sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		}
		// Override array use
		if (remote->flag & p3LHCFG_NUSEARRAY) {
			sprintf(cfgbuf, "%s/%d/%s/%s = NO\n", p3L1REMOTE_TOK, remote->ID,
				p3L2ARRAY_TOK, p3L3USE_TOK);
			sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		} else {
			if (remote->arraysz > 0) {
				sprintf(cfgbuf, "%s/%d/%s/%s = %d\n", p3L1REMOTE_TOK,
					remote->ID, p3L2ARRAY_TOK, p3L3SIZE_TOK, remote->arraysz);
				sprintf(p3buf, "%s%s", p3buf, cfgbuf);
			}
			if (remote->datarray > 0) {
				if (remote->flag & p3LHCFG_DATTM)
					sprintf(cfgbuf, "%s/%d/%s/%s/%s = %d\n", p3L1REMOTE_TOK,
						remote->ID, p3L2ARRAY_TOK, p3L3DATA_TOK,
						p3L4TIME_TOK, remote->datarray);
				else
					sprintf(cfgbuf, "%s/%d/%s/%s/%s = %d\n", p3L1REMOTE_TOK,
						remote->ID, p3L2ARRAY_TOK, p3L3DATA_TOK,
						p3L4PACKETS_TOK, remote->datarray);
				sprintf(p3buf, "%s%s", p3buf, cfgbuf);
			}
			if (remote->ctlarray > 0) {
				if (remote->flag & p3LHCFG_CTLTM)
					sprintf(cfgbuf, "%s/%d/%s/%s/%s = %d\n", p3L1REMOTE_TOK,
						remote->ID, p3L2ARRAY_TOK, p3L3CONTROL_TOK,
						p3L4TIME_TOK, remote->ctlarray);
				else
					sprintf(cfgbuf, "%s/%d/%s/%s/%s = %d\n", p3L1REMOTE_TOK,
						remote->ID, p3L2ARRAY_TOK, p3L3CONTROL_TOK,
						p3L4PACKETS_TOK, remote->ctlarray);
				sprintf(p3buf, "%s%s", p3buf, cfgbuf);
			}
		}
		// Override heartbeat values
		if (remote->heartbeat > 0) {
			sprintf(cfgbuf, "%s/%d/%s/%s = %d\n", p3L1REMOTE_TOK, remote->ID,
				p3L2HEARTBEAT_TOK, p3L3TIME_TOK, remote->heartbeat);
			sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		}
		if (remote->heartfail > 0) {
			sprintf(cfgbuf, "%s/%d/%s/%s = %d\n", p3L1REMOTE_TOK, remote->ID,
				p3L2HEARTBEAT_TOK, p3L3FAIL_TOK, remote->heartfail);
			sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		}
		sprintf(p3buf, "%s#\n", p3buf);
#endif

#ifdef _p3_SECONDARY
		sprintf(cfgbuf, "%s/%s = %s\n", p3L1REMOTE_TOK,
			p3L2HOSTNAME_TOK, remote->hostname);
		sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		if (remote->flag & p3HST_IPV6)
			i = 6;
		else
			i = 4;
		sprintf(cfgbuf, "%s/%s = %d\n", p3L1REMOTE_TOK,
			p3L2IPVER_TOK, i);
		sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		sprintf(cfgbuf, "%s/%s = %s\n", p3L1REMOTE_TOK,
			p3L2ADDRESS_TOK, remote->addr);
		sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		sprintf(cfgbuf, "%s/%s = %d\n", p3L1REMOTE_TOK,
			p3L2PORT_TOK, remote->port);
		sprintf(p3buf, "%s%s", p3buf, cfgbuf);
		subnet = remote->subnets;
		while (subnet != NULL) {
			if (subnet->flag & p3HST_IPV6)
				i = 6;
			else
				i = 4;
			sprintf(cfgbuf, "%s/%s/%d/%s = %d\n", p3L1REMOTE_TOK,
				p3L2SUBNET_TOK, subnet->ID, p3L3IPVER_TOK, i);
			sprintf(p3buf, "%s%s", p3buf, cfgbuf);
			sprintf(cfgbuf, "%s/%s/%d/%s = %s\n", p3L1REMOTE_TOK,
				p3L2SUBNET_TOK, subnet->ID, p3L3ADDRESS_TOK,
				subnet->addr);
			sprintf(p3buf, "%s%s", p3buf, cfgbuf);
			subnet = subnet->next;
		}
#endif
		// Save remote configuration
		if (sz < strlen(p3buf)) {
			sz = strlen(p3buf);
printf("DEBUG: Write remote (%d) config: Size %d\n", remote->ID, sz);
			if (fwrite(p3buf, 1, sz, cfile) < sz) {
				sprintf(p3buf, "get_config: Error writing to configuration\
 file %s: %s\n", new_filename, strerror(errno));
				p3errmsg (p3MSG_ERR, p3buf);
				stat = -1;
				goto out;
			}
		}
		remote = remote->next;
		p3buf[0] = '\0';
		sz = 0;
	}

/*** TODO: Rename configuration file
	fclose(cfile);
	cfile = NULL;
	sprintf(old_filename, "%s%s", config->path, config->pricfg);
	remove(old_filename);
	rename(new_filename, old_filename);
 ***/

out:
	if (cfile != NULL)
		fclose(cfile);

	return(stat);
} /* end save_config */


/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>get_config: Failed to open configuration file <i>file name</i>:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 configuration handler was unable to open the specified
 * configuration file.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 * <hr><b>get_config: Error writing to configuration file <i>file name</i>:
 * <i>error reason</i></b>
 * \par Description (ERR):
 * The P3 configuration handler was unable to write to the specified
 * configuration file.  The file is temporary, so the original data is
 * not lost.  However, no updates are saved.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */


/**
 * \par Function:
 * validate_local
 *
 * \par Description:
 * Verify that required fields are set in the local host definition.
 * If there is an error, it is reported in the global, errmsg.
 *
 * \par Inputs:
 * - local: Local host defintion structure
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int validate_local(p3local *local)
{
	int stat = 0;

	errmsg[0] = '\0';
	if (local->hostname == NULL) {
		sprintf(errmsg, "\n  Hostname is undefined");
		stat = -1;
	}
	if (local->addr == NULL) {
		sprintf(errmsg, "%s\n  Address is undefined", errmsg);
		stat = -1;
	}
#ifdef _p3_SECONDARY
	if (local->subnets == NULL) {
		sprintf(errmsg, "%s\n  No local subnets defined", errmsg);
		stat = -1;
	}
#endif
	if (local->s2uname == NULL) {
		if ((local->s2uname = (char *)
			p3malloc(sizeof(p3FIFOIN))) == NULL) {
			sprintf(p3buf, "validate_local: Failed to allocate fifo\
 name buffer: %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		} else {
			strcpy(local->s2uname, p3FIFOIN);
		}
	}
	if (local->u2sname == NULL) {
		if ((local->u2sname = (char *)
			p3malloc(sizeof(p3FIFOOUT))) == NULL) {
			sprintf(p3buf, "validate_local: Failed to allocate fifo\
 name buffer: %s\n", strerror(errno));
			p3errmsg (p3MSG_CRIT, p3buf);
			stat = -1;
			goto out;
		} else {
			strcpy(local->u2sname, p3FIFOOUT);
		}
	}

out:
	return(stat);
} /* end validate_local */

/**
 * \page P3ADMIN_MSGS Protected Point to Point Administration Messages
 * <hr><b>validate_local: Failed to allocate fifo name buffer:
 * <i>error reason</i></b>
 * \par Description (CRIT):
 * The P3 configuration handler attempts to allocate buffers for FIFO
 * names.  If this fails, there is a system wide problem.
 * \par Response:
 * Troubleshoot the operating system problem based on the error reason.
 *
 */

#ifdef _p3_PRIMARY
/**
 * \par Function:
 * validate_group
 *
 * \par Description:
 * Verify that required fields are set in the group host definition.
 * If there is an error, it is reported in the global, errmsg.
 *
 * \par Inputs:
 * - group: group host defintion structure
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int validate_group(p3group *group)
{
	int stat = 0;

	errmsg[0] = '\0';
	if (group->name == NULL) {
		sprintf(errmsg, "\n  Group name is undefined");
		stat = -1;
	}

	return(stat);
} /* end validate_group */
#endif

/**
 * \par Function:
 * validate_remote
 *
 * \par Description:
 * Verify that required fields are set in the remote host definition.
 * If there is an error, it is reported in the global, errmsg.
 *
 * \par Inputs:
 * - remote: remote host defintion structure
 *
 * \par Outputs:
 * - int: Status
 *   - 0 = OK
 *   - <0 = Error
 */

int validate_remote(p3remote *remote)
{
	int stat = 0;
	p3subnet *subnet;

	errmsg[0] = '\0';
	if (remote->hostname == NULL) {
		sprintf(errmsg, "\n  Hostname is undefined");
		stat = -1;
	}
	if (remote->addr == NULL) {
		sprintf(errmsg, "%s\n  Address is undefined", errmsg);
		stat = -1;
	}
	if (remote->subnets == NULL) {
		sprintf(errmsg, "%s\n  Subnets are undefined", errmsg);
		stat = -1;
	} else {
		subnet = remote->subnets;
		while (subnet != NULL) {
			if (subnet->addr == NULL) {
				sprintf(errmsg, "%s\n  Subnet %d address is undefined",
					errmsg, subnet->ID);
				stat = -1;
			}
			subnet = subnet->next;
		}
	}

	return(stat);
} /* end validate_remote */
