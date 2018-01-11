# Copyright 2010 Velocite Systems
# 
# Protected Point to Point System kernel module Makefile
#
# This Makefile will only work on the following Linux distributions
# with the source code installed:
#    Ubuntu GNU/Linux 2.6.31
#

###
### UNTIL FURTHER NOTICE:
###   There must be a link from each of the module directories:
###   primary, secondary, and primaryplus to the moc_src directory.
###
###

### IMPORTANT NOTE
### IMPORTANT NOTE
### 
###     The p3LINUXVER value must be set in p3linux.h to
###     build to the proper kernel definitions.
### 
### IMPORTANT NOTE
### IMPORTANT NOTE

OSVER=`uname -r`
export KROOT=/lib/modules/$(OSVER)/build
# export KROOT=/lib/modules/2.6.31-22-generic/build
export SROOT=/usr/src/linux-source-2.6.31
export CDIR=`pwd`

allofit:   modules
modules:
	cp p3kbase.h p3kshare.h p3kcrypto.c p3kcrypto.h \
		p3kprimary.c p3kprimary.h p3knet.c \
		p3knet.h p3kpri_session.c p3ksession.c \
		p3ksession.h p3kconnect.h p3linux.c p3linux.h primary/.
	cp p3kbase.h p3kshare.h p3kcrypto.c p3kcrypto.h \
		p3kprimaryplus.c p3kprimaryplus.h \
		p3knet.c p3knet.h p3kpri_session.c p3ksec_session.c \
		p3ksession.c p3ksession.h p3kconnect.h p3linux.c p3linux.h primaryplus/.
	cp p3kbase.h p3kshare.h p3kcrypto.c p3kcrypto.h \
		p3knet.c p3knet.h p3ksecondary.c \
		p3ksecondary.h p3ksec_session.c p3ksession.c \
		p3ksession.h p3kconnect.h p3linux.c p3linux.h secondary/.
	@$(MAKE) -C $(KROOT) M=$(CDIR)/primary modules
	@$(MAKE) -C $(KROOT) M=$(CDIR)/secondary modules
#	@$(MAKE) -C $(KROOT) M=$(CDIR)/primaryplus modules

install:
	@$(MAKE) -C $(SROOT) M=$(CDIR)/primary modules_install
	@$(MAKE) -C $(SROOT) M=$(CDIR)/secondary modules_install
#	@$(MAKE) -C $(SROOT) M=$(CDIR)/primaryplus modules_install

kernel_clean:
	@$(MAKE) -C $(KROOT) M=$(CDIR)/primary clean
	@$(MAKE) -C $(KROOT) M=$(CDIR)/secondary clean
#	@$(MAKE) -C $(KROOT) M=$(CDIR)/primaryplus clean
	rm primary/p3* secondary/p3* primaryplus/p3*

clean: kernel_clean
	rm -rf   Module.symvers modules.order moc_src/*.o moc_src/.*.o.cmd

