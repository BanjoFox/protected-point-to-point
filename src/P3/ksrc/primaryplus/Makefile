# Copyright 2010 Velocite Systems
# 
# Protected Point to Point System kernel module Makefile
#
# NOTE: This Makefile is customized for the 2.6.31 kernel.
#

MOCFLAGS = -D__MOC_IPV4_STACK__ \
	-D__RTOS_LINUX__

### NOTE: Create a link to the Mocana source code directory
###       called moc_src in the build directory. Note that
###       make must have write permission in the directory.
MSRCDIR = moc_src

MOBJS = $(MSRCDIR)/aes.o \
	$(MSRCDIR)/aesalgo.o \
	$(MSRCDIR)/merrors.o

EXTRA_CFLAGS += -D_p3_PRIMARYPLUS=1 $(MOCFLAGS) -I$(MSRCDIR)

obj-m +=  p3primaryplus.o

p3primaryplus-objs := p3kprimaryplus.o \
	p3knet.o \
	p3kpri_session.o \
	p3ksec_session.o \
	p3ksession.o \
	p3kcrypto.o \
	$(MOBJS) \
	p3linux.o

OSVER=`uname -r`
export KROOT=/lib/modules/$(OSVER)/build
export SROOT=/usr/src/linux-source-2.6.31
export CDIR=`pwd`

allofit:   modules
modules:
	@$(MAKE) $(EXTRA_CFLAGS) -C $(KROOT) M=$(CDIR) modules
install:
	@$(MAKE) -C $(SROOT) M=$(CDIR) modules_install
kernel_clean:
	@$(MAKE) -C $(KROOT) M=$(CDIR) clean

clean: kernel_clean
	rm -rf   Module.symvers modules.order

