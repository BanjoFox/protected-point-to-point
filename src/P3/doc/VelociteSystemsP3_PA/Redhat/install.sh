#!/bin/sh
#
# This script must be run as root
#

myID=`id -u`
if [ "$myID" != "0" ]; then
  echo "This script must be run by root"
  exit 1
fi

if [ ! -d /etc/p3 ]; then
  mkdir /etc/p3
  if [ "$?" != 0 ]; then
	exit 1
  fi
fi

if [ ! -d /lib/modules ]; then
  echo "No /lib/modules"
  exit 1
fi
if [ ! -d /lib/modules/2.6.18-274.7.1.el5 ]; then
  echo "No /lib/modules/2.6.18-274.7.1.el5"
  exit 1
fi
if [ ! -d /lib/modules/2.6.18-274.7.1.el5/extra ]; then
  mkdir /lib/modules/2.6.18-274.7.1.el5/extra
  if [ "$?" != 0 ]; then
	exit 1
  fi
fi

echo -n "Enter P3 use P(rimary) or S(econdary):"
read p3PARM
	if [ "$p3PARM" = "p" ] || [ "$p3PARM" = "P" ]; then
		cp p3primary.ko /lib/modules/2.6.18-274.7.1.el5/extra/.
		cp p3sysprimary /usr/sbin/.
		cp p3primary.conf /etc/p3/.
	elif [ "$p3PARM" = "s" ] || [ "$p3PARM" = "S" ]; then
		cp p3secondary.ko /lib/modules/2.6.18-274.7.1.el5/extra
		cp p3syssecondary /usr/sbin/.
		cp p3secondary.conf /etc/p3/.
	else
		echo "Invalid selection $p3PARM"
		exit 1
	fi

mkfifo /etc/p3/s2ufifo1
mkfifo /etc/p3/s2ufifo2
mkfifo /etc/p3/s2ufifo3
mkfifo /etc/p3/u2sfifo1
mkfifo /etc/p3/u2sfifo2
mkfifo /etc/p3/u2sfifo3
depmod -a

echo "The Velocite Systems P3 files have been installed"
echo "To test the installation, run the following commands:"
echo ''
	if [ "$p3PARM" = "p" ] || [ "$p3PARM" = "P" ]; then
		echo "  modprobe p3primary"
		echo "  p3sysprimary &"
	else
		echo "  modprobe p3secondary"
		echo "  p3syssecondary &"
	fi
echo ''
echo "To shut down the P3 system, get the process ID of the"
echo "P3 system application and shut it down.  Then deinstall"
echo "the P3 kernel module."
echo ''
	if [ "$p3PARM" = "p" ] || [ "$p3PARM" = "P" ]; then
		echo "  ps -C p3sysprimary"
		echo "  kill <pid>"
		echo "  rmmod p3primary"
	else
		echo "  ps -C p3syssecondary"
		echo "  kill <pid>"
		echo "  rmmod p3secondary"
	fi
echo ''

