/**
 * \file p3admin.h
 * <h3>Protected Point to Point product administration header file</h3>
 *
 * Copyright (C) Velocite 2010
 */

#ifndef _p3_ADMIN_H
#define _p3_ADMIN_H

#include "p3utils.h"
#include "p3config.h"
#include "p3command.h"

/*****  CONSTANTS  *****/

#define p3GROUPTYPE		1	/* A get remote spec uses a group ID list */
#define p3REMOTETYPE	2	/* A get remote spec uses a remote ID list */

/*****  MACROS  *****/

/*****  DATA DEFINITIONS  *****/

typedef struct _p3admin p3admin;
typedef struct _p3commlist p3commlist;
typedef struct _p3user p3user;
typedef struct _p3remotelist p3remotelist;

/**
 * Structure:
 * p3admin
 *
 * \par Description:
 * The main administration structure is used to locate all other
 * data structures.
 */

struct _p3admin {
	char 		*home;					/**< Home directory path */
#define p3ADM_HOME	"/usr/share/velsys/p3/"
	p3config	*config;				/**< Configuration information */
	p3command	*command;				/**< UI/Applcation communication */
	p3group		*groups[p3MAX_GROUPS];	/*<< Group list arrays */
	p3user		*users;					/*<< User list */
	p3user		*admuser;				/*<< Current administration user */
	int			ID;						/*<< Communication ID counter */
};

/**
 * Structure:
 * p3commlist
 *
 * \par Description:
 * The structure provides a way of creating lists of p3comm structures,
 * which do not themselves include next pointers.
 */

struct _p3commlist {
	p3commlist	*next;		/**< List pointer */
	p3comm		*comm;		/*<< Communication structure */
};

/**
 * Structure:
 * p3user
 *
 * \par Description:
 * The user structure is used to maintain basic information about
 * valid users.
 */

struct _p3user {
	p3user		*next;		/**< List pointer */
	char		*name;		/**< Human readable name */
	char		*uid;		/**< P3 System user id */
	int			ID;			/**< ID number */
	int			level;		/**< Authorization level */
};

/**
 * Structure:
 * p3remotelist
 *
 * \par Description:
 * The structure maintains a list of ID numbers of either group or remote
 * definitions for the user interface to request groups of remotes.
 */

struct _p3remotelist {
	int			len;	/**< Number of IDs in list */
	int			*list;	/**< ID list array */
};

/*****  PROTOTYPES  *****/

int init_admin(int argc, char *argv[]);
int adm_parse_cmdline(int pc_argc, char *pc_argv[]);
int setup_admin();
p3comm *admin_handler(p3comm *uidata);
void init_comm(p3comm *comm_def);
int parse_comm(char *comm_input, p3comm *comm_def);
int get_list(p3comm *uicomm, int update);
int add_comm(char *levels, int id1, int id2, char *data);
p3remotelist *get_remote_spec(char *spec, int type);
int admin_shutdown();

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _p3_ADMIN_C
extern p3admin *admin;
extern p3commlist *commlist;
#endif

#endif /* _p3_ADMIN_H */
