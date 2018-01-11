/**
 * \file p3primary.c
 * <h3>Protected Point to Point primary administrator program file</h3>
 *
 * Copyright (C) Velocite 2010
 *
 * The P3 primary administrator interface is used to manage configuration data
 * and P3 sessions.
 */

#define _p3_PRIMARY 1
#include "p3admin.h"
#include "p3utils.h"

/**
 * \par Function:
 * main
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

int main(int argc, char *argv[])
{
	int i, rsp, stat = 0;
	char ibuf[128];
	p3comm pcomm, *plist;

	if ((stat = init_admin(argc, argv)) != 0) {
printf(util->message);
		goto out;
	}

	// Loop to handle input from either interface
	while (1) {
		printf("P3 URI:  ");
		fgets(ibuf, 127, stdin);
		if (strncmp(ibuf, "quit", 4) == 0)
			break;
		else if ((stat = parse_comm(ibuf, &pcomm)) < 0) {
			if (pcomm.data != NULL)
				free (pcomm.data);
			pcomm.data = get_errmsg();
			pcomm.datlen = strlen(pcomm.data) + 1;
printf(pcomm.data);
			pcomm.data = NULL;
		} else if (stat > 0) {
			sprintf(p3buf, "Error parsing command: %s", errmsg);
			p3errmsg(p3MSG_ERR, p3buf);
			if (pcomm.data != NULL)
				free (pcomm.data);
			pcomm.data = get_errmsg();
			pcomm.datlen = strlen(pcomm.data) + 1;
printf(pcomm.data);
			pcomm.data = NULL;
		} else if (admin_handler(&pcomm) == NULL) {
			if (pcomm.data != NULL)
				free (pcomm.data);
			pcomm.data = get_errmsg();
			pcomm.datlen = strlen(pcomm.data) + 1;
printf(pcomm.data);
			pcomm.data = NULL;
		} else {
			if (pcomm.tokens[1] == p3L2GET) {
				rsp = strtol(pcomm.data, NULL, 10);
				init_comm(&pcomm);
				pcomm.tokens[0] = p3L1GETNEXT;
				for (i=0; i < rsp; i++) {
					if ((plist = admin_handler(&pcomm)) == NULL) {
printf("Less than %d items in list\n", rsp);
						break;
					} else {
sprintf(p3buf, "List(%d) Tokens: %d %d %d %d, IDs: %d %d, Data(%d)\n  |%s|\n", i,
	plist->tokens[0], plist->tokens[1], plist->tokens[2], plist->tokens[3],
	plist->id1, plist->id2, plist->datlen, plist->data);
p3errlog(p3MSG_DEBUG, p3buf);
						// Free communication structure list element
						plist->tokens[0] = p3L1FREE;
						admin_handler(plist);
					}
				}
			} else {
				if (pcomm.data != NULL)
					free (pcomm.data);
				pcomm.data = get_errmsg();
				pcomm.datlen = strlen(pcomm.data) + 1;
printf(pcomm.data);
				pcomm.data = NULL;
			}
		}
	}
	admin_shutdown();

out:
	return(stat);
}
