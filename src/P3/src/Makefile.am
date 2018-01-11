## Process this file with automake to produce Makefile.in

INCLUDES =\
	 -I./ \
	 -I/home/jsansing/data/mocana_nanocrypto/src

lib_LIBRARIES = \
	libp3prisystem.a \
	libp3secsystem.a

libp3prisystem_a_CFLAGS =\
	-D_p3_PRIMARY=1\
	-Wall\
	-O2\
	-ggdb3
#	-Wno-format
#	-Wno-pointer-sign

libp3prisystem_a_SOURCES =\
	p3system.c \
	p3net.c \
	p3pri_key_server.c \
	p3crypto.c \
	p3admin.c \
	p3utils.c

libp3secsystem_a_CFLAGS =\
	-D_p3_SECONDARY=1\
	-Wall\
	-O2\
	-ggdb3
#	-Wno-format
#	-Wno-pointer-sign

libp3secsystem_a_SOURCES =\
	p3system.c \
	p3net.c \
	p3sec_key_handler.c \
	p3admin.c \
	p3utils.c

bin_PROGRAMS = p3sysprimary \
	p3syssecondary
##	p3sysprimaryplus

p3sysprimary_CFLAGS =\
	 -D_p3_PRIMARY=1\
	 -Wall\
	 -ggdb3

p3sysprimary_SOURCES =\
	p3primary.c

p3sysprimary_LDFLAGS =\
	-L. \
	-L/home/jsansing/data/mocana_nanocrypto/bin

p3sysprimary_LDADD =\
	-lp3prisystem\
	-lcrypto\
	-lpthread

p3syssecondary_CFLAGS =\
	 -D_p3_SECONDARY=1\
	 -Wall\
	 -ggdb3

p3syssecondary_SOURCES =\
	p3secondary.c

p3syssecondary_LDFLAGS =\
	-L.\
	-L/home/jsansing/data/mocana_nanocrypto/bin

p3syssecondary_LDADD =\
	-lp3secsystem\
	-lcrypto\
	-lpthread

## p3sysprimaryplus_CFLAGS =\
##	 -D_p3_PRIMARYPLUS=1\
##	 -Wall\
##	 -ggdb3

## p3sysprimaryplus_SOURCES = \
##	p3primaryplus.c

## p3sysprimaryplus_LDFLAGS = 

## p3sysprimaryplus_LDADD = 

