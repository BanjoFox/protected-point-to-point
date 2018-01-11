/**
 * \file s3agent_net.h
 * <h3>Secure Storage Solution agent network header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _s3AGENT_NET_H
#define _s3AGENT_NET_H

/*****  INCLUDE FILES *****/

#include "s3base.h"

/*****  CONSTANTS  *****/

/*****  DATA DEFINITIONS  *****/

typedef struct _s3agent_net s3agent_net;

/**
 * Structure:
 * s3agent_net
 * 
 * \par Description:
 * Description.
 */

struct _s3agent_net {
	unsigned int	flag;
#define s3PNET_FLG1	0x00000001
#define s3PNET_FLG2	0x00000002
#define s3PNET_FLG3	0x00000004
};

/*****  MACROS  *****/

/*****  PROTOTYPES  *****/

int init_net();
int req_connect(int host_id);
int send_msg(int host_id, unsigned char *message);
unsigned char *recv_msg(int host_id);

/*****  EXTERNAL DEFINITIONS  *****/

#ifndef _s3AGENT_NET_C
extern s3agent_net *net;
#endif

#endif /* _s3AGENT_NET_H */
