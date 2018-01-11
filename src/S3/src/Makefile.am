## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" 

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = s3key_server \
	s3encrypt_engine \
	s3agent

s3key_server_SOURCES = \
	s3key_server.c \
	s3net.c \
	s3session.c \
	s3ui.c \
	s3utils.c

s3key_server_LDFLAGS = 

s3key_server_LDADD = 

s3encrypt_engine_SOURCES = \
	s3encrypt_engine.c \
	s3net.c \
	s3session.c \
	s3utils.c

s3encrypt_engine_LDFLAGS = 

s3encrypt_engine_LDADD = 

s3agent_SOURCES = \
	s3agent.c \
	s3agent_net.c \
	s3session.c \
	s3agent_ui.c \
	s3utils.c

s3agent_LDFLAGS = 

s3agent_LDADD = 

