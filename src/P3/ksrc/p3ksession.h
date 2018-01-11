/**
 * \file p3ksession.h
 * <h3>Protected Point to Point primary user interface header file</h3>
 * 
 * Copyright (C) Velocite 2010
 */

#ifndef _p3kSESSION_H
#define _p3kSESSION_H

/*****  INCLUDE FILES *****/

#include "p3kbase.h"
#include "p3kconnect.h"
#include "p3kcrypto.h"

/*****  CONSTANTS  *****/

#define p3STYPE_DATA			1
#define p3STYPE_CTL				2

#define p3MAX_MSG_SZ			1460 - 96 - 16	/* MSS - 2 IPv6 P3 headers - encrypt alignment */

#define p3CMSG_KTYPE			0x0f	/* Key type field */
#define p3CMSG_SET_KEY_ARRAY	1
#define p3CMSG_ACK_KEY_ARRAY	2
#define p3CMSG_AKKERR			0x01	/* ACK_KEY_ARRAY Key error */
#define p3CMSG_AKDERR			0x02	/* ACK_KEY_ARRAY Data error */
#define p3CMSG_AKERR \
	(p3CMSG_AKKERR | p3CMSG_AKDERR)
#define p3CMSG_REPLACE_KEY		3
#define p3CMSG_KRCIDX			0x10	/* REPLACE_KEY Control index */
#define p3CMSG_KRDIDX			0x20	/* REPLACE_KEY Data index */
#define p3CMSG_KRXMIT			0x40	/* REPLACE_KEY Retransmit */
#define p3CMSG_REKEY			4
#define p3CMSG_RKCKERR			0x01	/* REKEY Control key error */
#define p3CMSG_RKCIERR			0x02	/* REKEY Control key index error */
#define p3CMSG_RKDKERR			0x04	/* REKEY Data key error */
#define p3CMSG_RKDIERR			0x08	/* REKEY Data key index error */
#define p3CMSG_RKERR \
	(p3CMSG_RKCKERR | p3CMSG_RKCIERR | p3CMSG_RKDKERR | p3CMSG_RKDIERR)
#define p3CMSG_REKEY_TEST		5
#define p3CMSG_HRTBEAT_QUERY	6
#define p3CMSG_HRTBEAT_ANSWER	7
#define p3CMSG_STATUS_REQ		8
#define p3CMSG_STATUS_RESP		9
#define p3CMSG_UPDATE_INFO		10
#define p3CMSG_UICODE			0x01	/* UPDATE_INFO Code update */
#define p3CMSG_UICFG			0x02	/* UPDATE_INFO Configuration update */
#define p3CMSG_ACK_UPDATE		11
#define p3CMSG_AUDERR			0x01	/* ACK_UPDATE Update data error */
#define p3CMSG_AUUERR			0x02	/* ACK_UPDATE Update error */
#define p3CMSG_AUERR \
	(p3CMSG_AUDERR | p3CMSG_AUUERR)
#define p3CMSG_SHUTDOWN			12
#define p3CMSG_SHRIMM			0x01	/* SHUTDOWN Restart immediately */
#define p3CMSG_SHRTIME			0x02	/* SHUTDOWN Restart after timeout */
#define p3CMSG_ACK_SHUTDOWN		13
#define p3CMSG_ASACK			0x01	/* ACK_SHUTDOWN Shutdown ACK */

/*****  DATA DEFINITIONS  *****/

/*****  MACROS  *****/

/*****  PROTOYPES  *****/

extern void init_session(p3host *host, void *srcaddr);
extern p3ctlmsg *build_newkey_message(unsigned int flag, p3session *p3sess, p3key_mgr *key_mgr);
extern p3ctlmsg *build_flag_message(int type, unsigned int flag);
extern p3ctlmsg *build_vlen_message(int type, unsigned int flag, unsigned char *message, int len);
extern p3ctlmsg *build_heartbeat_message(int type, unsigned int sequence);
extern p3ctlmsg *build_ctl_message(int type, unsigned char *message, int len);
extern int parse_ctl_message(p3ctlmsg *ctlmsg, p3session *p3sess);

extern int pri_session_manager(void);
extern void key_array_error(int flag, p3session *p3sess);
void rekey_session(int flag, unsigned int key_num, p3session *p3sess);
extern int rekey_test_pri(unsigned char *message, int size, p3session *p3sess);
extern void heartbeat_answer(unsigned int time, unsigned int seqnum, p3session *p3sess);

extern int sec_session_manager(void);
extern int set_key_array(unsigned char *message, int ksize, int dsize, p3session *p3sess);
extern int replace_key(unsigned char *dkey, int dindex, unsigned char *ckey, int cindex, p3session *p3sess);
extern void rekey_session(int flag, unsigned int key_num, p3session *p3sess);
extern int rekey_test_sec(unsigned char *message, int size, p3session *p3sess);
extern int heartbeat_query(unsigned int time, unsigned int seqnum, p3session *p3sess);

/*****  EXTERNAL DEFINITIONS  *****/

#ifdef _p3_SECONDARY
extern p3session *p3sess;
#endif

#endif /* _p3kSESSION_H */

