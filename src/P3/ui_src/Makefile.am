#
# The automake file for the P3 user interface library
#
#

JAVADIR=/usr/lib/jvm/java-1.5.0-sun-1.5.0.22

INCLUDES =\
	-I./\
	-I$(JAVADIR)/include\
	-I$(JAVADIR)/include/linux

lib_LTLIBRARIES = \
	libp3priadmin.la \
	libp3secadmin.la

libp3priadmin_la_SOURCES = \
	p3admin.c \
	p3config.c \
	p3command.c \
	p3anonymous.c \
	p3fifo.c \
	p3net.c \
	p3utils.c

libp3priadmin_la_CFLAGS =\
	-D_p3_PRIMARY=1\
	-Wall\
	-O2\
	-ggdb3

#	-Wno-format
#	-Wno-pointer-sign

# Version info rules
# 1. Start with version information of ‘0:0:0’ for each libtool library.
# 2. Update the version information only immediately before a public release
#   of the software. More frequent updates are unnecessary, and only guarantee
#   that the current interface number gets larger faster.
# 3. If the library source code has changed at all since the last update, then
#   increment revision (‘c:r:a’ becomes ‘c:r+1:a’).
# 4. If any interfaces have been added, removed, or changed since the last
#   update, increment current, and set revision to 0.
# 5. If any interfaces have been added since the last public release, then
#   increment age.
# 6. If any interfaces have been removed or changed since the last public
#   release, then set age to 0. 
libp3priadmin_la_LDFLAGS =\
	-version-info 0:0:0

libp3secadmin_la_SOURCES = \
	p3admin.c \
	p3config.c \
	p3command.c \
	p3anonymous.c \
	p3fifo.c \
	p3net.c \
	p3utils.c

libp3secadmin_la_CFLAGS =\
	-D_p3_SECONDARY=1\
	-Wall\
	-O2\
	-ggdb3

#	-Wno-format
#	-Wno-pointer-sign

libp3secadmin_la_LDFLAGS =\
	-version-info 0:0:0

sbin_PROGRAMS = p3primary \
	p3secondary

p3primary_CFLAGS =\
	-D_p3_PRIMARY=1\
	-Wall\
	-ggdb3\
	-O2

#	-Wno-format
#	-Wno-pointer-sign

p3primary_SOURCES = \
	p3primary.c


p3primary_LDFLAGS = \
	-L.

p3primary_LDADD = \
	-lp3priadmin

p3secondary_CFLAGS =\
	-D_p3_PRIMARY=1\
	-Wall\
	-ggdb3\
	-O2

#	-Wno-format
#	-Wno-pointer-sign

p3secondary_SOURCES = \
	p3secondary.c


p3secondary_LDFLAGS = \
	-L.

p3secondary_LDADD = \
	-lp3secadmin

